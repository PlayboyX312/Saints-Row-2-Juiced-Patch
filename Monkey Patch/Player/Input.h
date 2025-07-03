#pragma once
#include <safetyhook.hpp>
namespace Input {
	extern bool useTextPrompts;
	extern void Init();
	enum GAME_LAST_INPUT : BYTE {
		UNKNOWN,
		MOUSE,
		CONTROLLER,
	};
	inline bool betterTags;
	extern void UnloadXInputDLL();
	extern BYTE disable_aim_assist_noMatterInput;
	extern SafetyHookMid player_autoaim_do_assisted_aiming_midhook;
	extern GAME_LAST_INPUT LastInput();
	extern GAME_LAST_INPUT LastInputUI();
	extern int usePS3Prompts;
	extern int ForceInput;
	extern BYTE EnableDynamicPrompts;
	extern int HoldFineAim;
}