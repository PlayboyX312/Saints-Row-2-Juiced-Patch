// XACT.cpp (uzis, Scanti)
// --------------------
// Created: 22/02/2025

#include "../FileLogger.h"
#include "../Patcher/patch.h"
#include "../GameConfig.h"
#include "../SafeWrite.h"
#include "../RPCHandler.h"
#include "../Game/Game.h"
#include <safetyhook.hpp>
namespace XACT
{
	void FixAudioHack()
	{
#if !JLITE
		if (GameConfig::GetValue("Debug", "FixAudio", 0)) // Hacky half fix for 3D Audio in cutscenes.
		{
			Logger::TypedLog(CHN_DEBUG, "Patching Stereo Cutscenes (EXPERIMENTAL)...\n");
			Game::InLoop::ShouldFixStereo = true; 
			/* Hack is located in the rich presence code, kind of lazy yesand might be moved in the future
			 but it was only a temporary patch anyway until we find something nicer. */
		}
#endif
	}

	void UpdateToNewerXACT()
	{
		if (GameConfig::GetValue("Audio", "UseFixedXACT", 1)) // Scanti the Goat
		{
			// Forces the game to use a newer version of XACT which in turn fixes all of the audio issues
			// in SR2 aside from 3D Panning.
			GUID xaudio = { 0x4c5e637a, 0x16c7, 0x4de3, 0x9c, 0x46, 0x5e, 0xd2, 0x21, 0x81, 0x96, 0x2d };        // version 2.3
			GUID ixaudio = { 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb };
			SafeWriteBuf((0x00DD8A08), &xaudio, sizeof(xaudio));
			SafeWriteBuf((0x00DD8A18), &ixaudio, sizeof(ixaudio));
			Logger::TypedLog(CHN_MOD, "Forcing the use of a fixed XACT version.\n");
		}
	}
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
	void ChangeSpeakerCount()
	{
		static auto FixAudioLoop_null_crash1_hook = safetyhook::create_mid(0x0046EE64, &FixAudioLoop_null_crash1);
		if (GameConfig::GetValue("Audio", "51Surround", 0) == 1)
		{
			Logger::TypedLog(CHN_AUDIO, "Using 5.1 Surround Sound...\n");
		}
		else {
			Logger::TypedLog(CHN_AUDIO, "Fixing Stereo Audio...\n");
			UINT32 number_of_speakers = 2;
			UINT32 frequency = 48000;

			//SafeWrite8(0x004818E3, number_of_speakers);         // Causes major audio glitches
			SafeWrite8(0x00482B08, number_of_speakers);
			SafeWrite8(0x00482B41, number_of_speakers);
			SafeWrite8(0x00482B96, number_of_speakers);

			SafeWrite32(0x00482B03, frequency);
			SafeWrite32(0x00482B3C, frequency);
			SafeWrite32(0x00482B91, frequency);
		}
	}

	void Init()
	{
		FixAudioHack();
		UpdateToNewerXACT();
		ChangeSpeakerCount();

	}
}