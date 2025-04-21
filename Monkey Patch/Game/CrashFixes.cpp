// CrashFixes.cpp (clippy95)
// --------------------
// Created: 20/4/2025
#include <safetyhook.hpp>
#include <wtypes.h>
#include "../GameConfig.h"
#include "../FileLogger.h"
#include <unordered_set>

namespace AssertHandler {
	static std::unordered_set<std::string> ignored_asserts;

	inline void AssertOnce(const char* id, const char* message) {
		if (ignored_asserts.count(id))
			return;

		std::string full_message =
			"[Assert ID: " + std::string(id) + "]\n" +
			message +
			"\n\nPress 'Yes' to ignore this warning for the session.";

		Logger::TypedLog(CHN_DEBUG, "[Assert ID: %s] %s", id, message);

		int result = MessageBoxA(nullptr, full_message.c_str(), "Error Occurred", MB_ICONWARNING | MB_YESNO);
		if (result == IDYES) {
			ignored_asserts.insert(id);
		}
	}
}

namespace CrashFixes {
	// sidokuthps had a crash while playing The Ronin - Rest In Peace, this section of code seems to only execute in missions? need to confirm, either ways they jump to an else if v0 is null but not if *v0 is null
// thus it crashes, here I check for that as well - Clippy95
	SAFETYHOOK_NOINLINE void FixAudioLoop_null_crash1(SafetyHookContext& ctx) {
		if (ctx.ebx) {
			if (*(int*)ctx.ebx == NULL) {
				// So Safetyhook converts the jnz short loc_46EE5B to a jne, so we'll jump by 6 bytes to keep on going.
				ctx.eip += 0x6;
			}
		}
	}
	SAFETYHOOK_NOINLINE void Fix_0x0055B681_crash(SafetyHookContext& ctx) {
		if (!*(DWORD*)ctx.eax)
			ctx.eip = 0x0055B68D;
	}
	SAFETYHOOK_NOINLINE void Fix_0x007B1D7E_crash_weaponstore(SafetyHookContext& ctx) {
		if (!ctx.edx)
			ctx.eip = 0x007B1E0D;
	}
	SAFETYHOOK_NOINLINE void Fix_0x007B510D_crash(SafetyHookContext& ctx) {
		if (!ctx.edx)
			ctx.eip = 0x007B5135;
	}
	SAFETYHOOK_NOINLINE void Fix_009AEDAD_crash_cs_start_characters_for_shot(SafetyHookContext& ctx) {
		if (ctx.ebx) {
			if (!(*(int*)(ctx.ebx + 0x10A4) + 0x18)) {
				ctx.eip = 0x9AEE79;
				goto crash_cs_start_characters_for_shot_009AEDAD;
			}
			else if(!*(int*)(*(int*)(ctx.ebx + 0x10A4) + 0x18)) {
				ctx.eip = 0x9AEE79;
				goto crash_cs_start_characters_for_shot_009AEDAD;
			}
			else if (!(*(int*)(*(int*)(ctx.ebx + 0x10A4) + 0x18) + 0x44)) {
				ctx.eip = 0x9AEE79;
				goto crash_cs_start_characters_for_shot_009AEDAD;
			}
			else if (!*(int*)(*(int*)(*(int*)(ctx.ebx + 0x10A4) + 0x18) + 0x44)) {
				ctx.eip = 0x9AEE79;
				goto crash_cs_start_characters_for_shot_009AEDAD;
			}
		}
		return;
	crash_cs_start_characters_for_shot_009AEDAD:
		AssertHandler::AssertOnce("cs_start_characters_for_shot_009AEDAD", "Crash prevented due to bad preload breaking cutscene objects, please fix your preload, game might still crash. \n");
	}
	SAFETYHOOK_NOINLINE void Fix_009AEE86_crash_cs_start_characters_for_shot(SafetyHookContext& ctx) {
		if (ctx.ebx) {
			if (!(*(int*)(ctx.ebx + 0x10AC) + 0x18)) {
				ctx.eip = 0x9AEF8C;
				goto crash_cs_start_characters_for_shot_009AEE86;
			}
			else if (!*(int*)(*(int*)(ctx.ebx + 0x10AC) + 0x18)) {
				ctx.eip = 0x9AEF8C;
				goto crash_cs_start_characters_for_shot_009AEE86;
			}
			else if (!(*(int*)(*(int*)(ctx.ebx + 0x10AC) + 0x18) + 0x44)) {
				ctx.eip = 0x9AEF8C;
				goto crash_cs_start_characters_for_shot_009AEE86;
			}
			else if (!*(int*)(*(int*)(*(int*)(ctx.ebx + 0x10AC) + 0x18) + 0x44)) {
				ctx.eip = 0x9AEF8C;
				goto crash_cs_start_characters_for_shot_009AEE86;
			}
		}
		return;
	crash_cs_start_characters_for_shot_009AEE86:
		AssertHandler::AssertOnce("cs_start_characters_for_shot_009AEE86", "Crash prevented due to bad preload breaking cutscene objects, please fix your preload, game might still crash \n");
	}
	void Init() {
		static auto FixAudioLoop_null_crash1_hook = safetyhook::create_mid(0x0046EE64, &FixAudioLoop_null_crash1);
		// The one above should work always..
		if (GameConfig::GetValue("Debug", "FixCrashes", 2)) {	
			static auto Fix_0x0055B681_hook = safetyhook::create_mid(0x0055B681, &Fix_0x0055B681_crash);
			static auto Fix_0x007B1D7E_hook = safetyhook::create_mid(0x007B1D7E, &Fix_0x007B1D7E_crash_weaponstore);
			static auto Fix_0x007B510D_hook = safetyhook::create_mid(0x007B510D, &Fix_0x007B510D_crash);
		}
		// I kind of want these seperated, idk why but let's say 2+ is confirmed by mods or modded setups?
		if (GameConfig::GetValue("Debug", "FixCrashes", 1) >= 2) {
			// Yes this is a modded crash, don't mess with preloads please. - this was caused by SidokuTHPS's modded preload.
			static auto Fix_0x009AEDAD_hook = safetyhook::create_mid(0x009AED73, &Fix_009AEDAD_crash_cs_start_characters_for_shot);
			// This one hasn't really crashed but it seems to be sameish?
			static auto Fix_0x009AEE86_hook = safetyhook::create_mid(0x009AEE86, &Fix_009AEE86_crash_cs_start_characters_for_shot);
		}
	}
}