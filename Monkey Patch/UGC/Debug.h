#pragma once
#include "../Patcher/CMultiPatch.h"
namespace Debug {
	extern void Init();
	extern void PatchDatafiles();
	extern bool addBindToggles;
	extern bool LoadLastSave;
	extern bool fixFrametime;
	extern int UseDynamicRenderDistance;
	extern float MAX_RENDER;
	extern float TRANSITION_SPEED;
	extern int SIZE_MIN;
	extern int MAX;
	extern CMultiPatch CMPatches_DisableCheatFlag;
	void DynamicRenderDistance();
	void PrintMemoryUsage(int y);
}