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
	//this->offsets.gameObjectManager = driver::read<uint64_t>(driver::get_process_peb() + this->offsets.offs_gameObjectManager);

	//cout << "gameObjectManager: " << hex << this->offsets.gameObjectManager;
	//printf("GOM: 0x%X\n", this->offsets.gameObjectManager);
	/*
	//

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = memio->read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	//printf("ActiveObjects: 0x%X\n", active_objects);

	if (!(this->offsets.gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
		return false;

	//printf("this->offsets.gameWorld: 0x%X\n", this->offsets.gameWorld);

	// Find fps camera.
	this->offsets.localGameWorld = memio->ReadChain(this->offsets.gameWorld, { 0x30, 0x18, 0x28 });

	//printf("localgameWorld: 0x%X\n", this->offsets.localGameWorld);

	// Get tagged objects and find fps camera.
	auto tagged_objects = memio->read<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(this->offsets.fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;


	*/
	return true;
}

