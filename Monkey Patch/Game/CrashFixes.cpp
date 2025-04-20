#include <safetyhook.hpp>
#include <wtypes.h>
#include "../GameConfig.h"
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
	void Init() {
		static auto FixAudioLoop_null_crash1_hook = safetyhook::create_mid(0x0046EE64, &FixAudioLoop_null_crash1);
		// The one above should work always..
		if (GameConfig::GetValue("Debug", "FixCrashes", 1)) {
			static auto Fix_0x0055B681_hook = safetyhook::create_mid(0x0055B681, &Fix_0x0055B681_crash);
			static auto Fix_0x007B1D7E_hook = safetyhook::create_mid(0x007B1D7E, &Fix_0x007B1D7E_crash_weaponstore);
			static auto Fix_0x007B510D_hook = safetyhook::create_mid(0x007B510D, &Fix_0x007B510D_crash);
		}
	}
}