#include "eftstructs.h"
#include "xorstr.hpp"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <iostream>
#include "driver.h"
#include <d3dx9math.h>
#include <d2d1.h>
#include "overlay.h"

using namespace std;
std::list<uint64_t> bodypart = { BodyParts::Head, BodyParts::Thorax, BodyParts::Stomach, BodyParts::LeftArm, BodyParts::RightArm, BodyParts::LeftLeg, BodyParts::RightLeg };

EFTData* EFTData::Instance()
{
	static EFTData instance;
	return &instance;
}

D3DXMATRIX getoptic_matrix(uint64_t instance)
{
	D3DXMATRIX temp_matrix;
	D3DXMATRIX outmatrix;

	static std::vector<uint64_t> tempchain{ EFTData::Instance()->offsets.Player.proceduralWeaponAnimation,0x88, 0x20, 0x28, 0x30 };

	uint64_t temp = driver::readEFTChain(instance, tempchain);

	//printf(_xor_("temp : 0x%X\n"), temp);
	driver::read_memory(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
	D3DXMatrixTranspose(&outmatrix, &temp_matrix);

	return outmatrix;
}


D3DXMATRIX viewMatrix;

bool WorldToScreenv2(const FVector& point3D, D2D1_POINT_2F& point2D)
{
	D3DXVECTOR3 _point3D = D3DXVECTOR3(point3D.x, point3D.z, point3D.y);

	auto& matrix = viewMatrix;

	if (EFTData::Instance()->IsAiming(EFTData::Instance()->localPlayer.instance) && EFTData::Instance()->get_mpcamera(EFTData::Instance()->localPlayer.instance))
	{
		matrix = getoptic_matrix(EFTData::Instance()->localPlayer.instance);
	}

	D3DXVECTOR3 translationVector = D3DXVECTOR3(matrix._41, matrix._42, matrix._43);
	D3DXVECTOR3 up = D3DXVECTOR3(matrix._21, matrix._22, matrix._23);
	D3DXVECTOR3 right = D3DXVECTOR3(matrix._11, matrix._12, matrix._13);

	float w = D3DXVec3Dot(&translationVector, &_point3D) + matrix._44;

	if (w < 0.098f)
		return false;

	float y = D3DXVec3Dot(&up, &_point3D) + matrix._24;
	float x = D3DXVec3Dot(&right, &_point3D) + matrix._14;


	if (EFTData::Instance()->IsAiming(EFTData::Instance()->localPlayer.instance) && EFTData::Instance()->get_mpcamera(EFTData::Instance()->localPlayer.instance))
	{
		uint64_t chain = driver::readEFTChain(EFTData::Instance()->offsets.fpsCamera, { 0x30, 0x18 });

		x /= driver::read<float>(chain + 0x12C);

		if (x < 2.f)
			x /= driver::read<float>(chain + 0xAC);

		y /= driver::read<float>(chain + 0x118);

		if (y < 2.f)
			y /= driver::read<float>(chain + 0x98);
	}

	point2D.x = (2560 / 2) * (1.f + x / w);
	point2D.y = (1440 / 2) * (1.f - y / w);

	return true;
}



/* All one time initialization in here*/
bool EFTData::InitOffsets()
{
	this->offsets.gameObjectManager = driver::read<uint64_t>(driver::get_process_peb() + this->offsets.offs_gameObjectManager);

	//cout << "gameObjectManager: " << hex << this->offsets.gameObjectManager;
	//printf("GOM: 0x%X\n", this->offsets.gameObjectManager);
	
	//

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = driver::read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	//printf("ActiveObjects: 0x%X\n", active_objects);
	
	if (!(this->offsets.gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
		return false;

	//printf("this->offsets.gameWorld: 0x%X\n", this->offsets.gameWorld);
	
	// Find fps camera.
	this->offsets.localGameWorld = driver::readEFTChain(this->offsets.gameWorld, { 0x30, 0x18, 0x28 });

	//printf("localgameWorld: 0x%X\n", this->offsets.localGameWorld);
	
	// Get tagged objects and find fps camera.
	auto tagged_objects = driver::read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(this->offsets.fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;
	//printf("fpsCamera: 0x%X\n", this->offsets.fpsCamera);
	/*
	*/
	return true;
}

bool EFTData::Read()
{
	this->players.clear();
	// Accumulate players.
	{

		uint64_t onlineusers = driver::read<uint64_t>(this->offsets.localGameWorld + this->offsets.localGameWorld_offsets.registeredPlayers);

		if (!onlineusers)
		{
			cout << "onlineusers read faild! \n";
			return false;
		}
			
		uint64_t list_base = driver::read<uint64_t>(onlineusers + offsetof(EFTStructs::List, listBase));
		int player_count = driver::read<int>(onlineusers + offsetof(EFTStructs::List, itemCount));
		//printf("player_count: %d\n", player_count);
		
		if (player_count <= 0 || !list_base)
			return false;
		//string palyinfo = "{\"player_count\":\"" + to_string(player_count) + "\"},";
		//sendStr += palyinfo;
		constexpr auto BUFFER_SIZE = 128;

		uint64_t player_buffer[BUFFER_SIZE];
		driver::read_memory(list_base + offsetof(EFTStructs::ListInternal, firstEntry), player_buffer, sizeof(uint64_t) * player_count);

		EFTPlayer player;

		// 获取相机矩阵
		{
			uint64_t temp = this->offsets.fpsCamera;
			if (!(temp = driver::read<uint64_t>(temp + 0x30)) || !(temp = driver::read<uint64_t>(temp + 0x18)))
				return false;

			D3DXMATRIX temp_matrix;
			driver::read_memory(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
			viewMatrix = temp_matrix;
			D3DXMatrixTranspose(&viewMatrix, &temp_matrix);

		}

		static char cTitle[256];
		sprintf_s(cTitle, "Players:%d", player_count);
		
		String(42, 92, D3DCOLOR_RGBA(255, 255, 255, 255), true, cTitle);

		float distance;
		float MaxDrawDistance = 700.f;
		string player_name = "";
		for (int i = 0; i < player_count; i++)
		{
			player.instance = player_buffer[i];
			this->playercount = player_count;


			uint64_t bone_matrix = this->getbone_matrix(player.instance);

			if (bone_matrix)
			{

				uint64_t bone = driver::readEFTChain(bone_matrix, { 0x20, 0x10, 0x38 });
				player.location = driver::read<FVector>(bone + 0xB0);

				player_name = EFTData::getPlayerName(player.instance);

				//cout << "get players!  x:" << player.location.x << "y: "<< player.location.y << "z: " << player.location.z << "\n";
				//player.headPos = GetPosition(driver::read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanHead * 8))));

			}

			// Leave this at the end to have all the data.
			if (driver::read<int>(player.instance + 0x18))
			{

				this->localPlayer = player;
				this->localPlayer.location = player.location;
			}

			this->players.emplace_back(player);
		
			// 2d pos get
			if (!player.instance) continue;
			if (player.instance == this->localPlayer.instance)continue;
			distance = this->localPlayer.location.Distance(player.location);
			
			if (distance > MaxDrawDistance)
				continue;
			D2D1_POINT_2F screen_pos;
			WorldToScreenv2(player.location, screen_pos);

			// draw
			String((int)screen_pos.x, (int)screen_pos.y, D3DCOLOR_RGBA(255, 255, 255, 255), true, _xor_("%0.2fm").operator const char* (), distance);
			Circle((int)screen_pos.x, (int)screen_pos.y, 4, 0, 1, true, 32, D3DCOLOR_ARGB(255, 255, 255, 255));

			float ki = (1 / distance) * 1.5 * 1440;
			if (player_name == "isPlayer")
				draw_box((int)screen_pos.x - (ki / 1440) * 400, (int)screen_pos.y - 70 * (ki / 1440) * 20, ki * 0.5, ki, D3DCOLOR_ARGB(200, 215, 0, 255));
			else 
				draw_box((int)screen_pos.x - (ki / 1440) * 400, (int)screen_pos.y - 70 * (ki / 1440) * 20, ki * 0.5, ki, D3DCOLOR_ARGB(200, 255, 215, 0));
			player_name = "";
			screen_pos.x = 0;
			screen_pos.y = 0;
		}
	}
	/*
	*/
	
	return true;
}


uint64_t EFTData::GetObjectFromList(uint64_t listPtr, uint64_t lastObjectPtr, const char* objectName)
{
	using EFTStructs::BaseObject;
	char name[256];
	uint64_t classNamePtr = 0x0;

	BaseObject activeObject = driver::read<BaseObject>(listPtr);
	BaseObject lastObject = driver::read<BaseObject>(lastObjectPtr);

	if (activeObject.object != 0x0)
	{
		while (activeObject.object != 0 && activeObject.object != lastObject.object)
		{
			classNamePtr = driver::read<uint64_t>(activeObject.object + 0x60);
			driver::read_memory(classNamePtr + 0x0, &name, sizeof(name));
			if (strcmp(name, objectName) == 0)
			{
				return activeObject.object;
			}

			activeObject = driver::read<BaseObject>(activeObject.nextObjectLink);
		}
	}
	if (lastObject.object != 0x0)
	{
		classNamePtr = driver::read<uint64_t>(lastObject.object + 0x60);
		driver::read_memory(classNamePtr + 0x0, &name, 256);
		if (strcmp(name, objectName) == 0)
		{
			return lastObject.object;
		}
	}

	return uint64_t();
}

uint64_t EFTData::getbone_matrix(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.Player.playerBody, 0x28, 0x28, 0x10 };

	return driver::readEFTChain(instance, temp);
}

string EFTData::getPlayerName(uint64_t instance)
{
	static std::vector<uint64_t> tempchain{ this->offsets.Player.profile, this->offsets.profile.information };

	uint64_t information = driver::readEFTChain(instance, tempchain);

	if (driver::read<int32_t>(information + 0x54) != 0)
	{
		// 获取用户名
		//uint64_t player_name = driver::read<int32_t>(information + 0x0010);
		//printf("player_name: 0x%X\n", player_name);
		//if (player_name)
		//{
		//	//int32_t nameLength = driver::read<int32_t>(player_name + this->offsets.unicodeString.length);
		//	//printf("nameLength: 0x%X\n", nameLength);
		//	return driver::GetUnicodeString(player_name , 5);
		//}
		return "isPlayer";
	}

	return "";
}

bool EFTData::IsAiming(uint64_t	 instance)
{
	uint64_t m_pbreath = driver::readEFTChain(instance, { this->offsets.Player.proceduralWeaponAnimation, 0x28 });
	return driver::read<bool>(m_pbreath + 0x88);
}

uint64_t EFTData::get_mpcamera(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.Player.proceduralWeaponAnimation, 0x88, 0x20 };

	return driver::readEFTChain(instance, temp);
}


