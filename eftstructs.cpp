#include "eftstructs.h"
#include "xorstr.hpp"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <iostream>
#include "driver.h"

using namespace std;
std::list<uint64_t> bodypart = { BodyParts::Head, BodyParts::Thorax, BodyParts::Stomach, BodyParts::LeftArm, BodyParts::RightArm, BodyParts::LeftLeg, BodyParts::RightLeg };

EFTData* EFTData::Instance()
{
	static EFTData instance;
	return &instance;
}

/* All one time initialization in here*/
bool EFTData::InitOffsets()
{
	this->offsets.gameObjectManager = driver::read<uint64_t>(driver::get_process_peb() + this->offsets.offs_gameObjectManager);

	cout << "gameObjectManager: " << hex << this->offsets.gameObjectManager;
	printf("GOM: 0x%X\n", this->offsets.gameObjectManager);
	
	//

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = driver::read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	printf("ActiveObjects: 0x%X\n", active_objects);
	
	if (!(this->offsets.gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
		return false;

	printf("this->offsets.gameWorld: 0x%X\n", this->offsets.gameWorld);
	
	// Find fps camera.
	this->offsets.localGameWorld = driver::readEFTChain(this->offsets.gameWorld, { 0x30, 0x18, 0x28 });

	printf("localgameWorld: 0x%X\n", this->offsets.localGameWorld);
	/*
	// Get tagged objects and find fps camera.
	auto tagged_objects = memio->read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(this->offsets.fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;


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
			cout << "onlineusers read faild! " << hex << onlineusers << "\n";
			return false;
		}
			

		uint64_t list_base = driver::read<uint64_t>(onlineusers + offsetof(EFTStructs::List, listBase));
		int player_count = driver::read<int>(onlineusers + offsetof(EFTStructs::List, itemCount));
		printf("player_count: %d\n", player_count);
		
		if (player_count <= 0 || !list_base)
			return false;

		constexpr auto BUFFER_SIZE = 128;

		uint64_t player_buffer[BUFFER_SIZE];
		driver::read_memory(list_base + offsetof(EFTStructs::ListInternal, firstEntry), player_buffer, sizeof(uint64_t) * player_count);

		EFTPlayer player;

		for (int i = 0; i < player_count; i++)
		{
			player.instance = player_buffer[i];
			this->playercount = player_count;


			uint64_t bone_matrix = this->getbone_matrix(player.instance);

			if (bone_matrix)
			{

				uint64_t bone = driver::readEFTChain(bone_matrix, { 0x20, 0x10, 0x38 });
				player.location = driver::read<FVector>(bone + 0xB0);
				cout << "get players!  x:" << player.location.x << "y: "<< player.location.y << "z: " << player.location.z << "\n";
				//player.headPos = GetPosition(driver::read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanHead * 8))));

			}

			// Leave this at the end to have all the data.
			if (driver::read<int>(player.instance + 0x18))
			{
				this->localPlayer = player;
				this->localPlayer.location = player.location;
			}

			this->players.emplace_back(player);
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