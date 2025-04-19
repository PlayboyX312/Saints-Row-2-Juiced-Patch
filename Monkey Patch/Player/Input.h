#pragma once
#include <safetyhook.hpp>
namespace Input {
	extern void Init();
	enum GAME_LAST_INPUT : BYTE {
		UNKNOWN,
		MOUSE,
		CONTROLLER,
	};
	extern void Process_Inventory_Hack_KBM();
	extern bool allow_hacked_inventory_KBM;
	inline bool betterTags;
	extern void UnloadXInputDLL();
	extern BYTE disable_aim_assist_noMatterInput;
	extern SafetyHookMid player_autoaim_do_assisted_aiming_midhook;
}