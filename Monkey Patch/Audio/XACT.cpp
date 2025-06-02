// XACT.cpp (uzis, Scanti)
// --------------------
// Created: 22/02/2025

#include "../FileLogger.h"
#include "../Patcher/patch.h"
#include "../GameConfig.h"
#include "../SafeWrite.h"
#include "../RPCHandler.h"
#include "../Game/Game.h"
#include "../General/General.h"
#include <safetyhook.hpp>
namespace XACT
{

	void Cutscene3DAudio(SafetyHookContext& ctx) {
		if (*General::InCutscene) {
			char* Flag = (char*)(ctx.esi + 0x196);
			*Flag &= ~0x0001; // disabling this flag makes directional audio work at the cost of the volume sliders not doing anything (hence the cutscene check)
		}
	}

	void FixAudioHack()
	{
#if !JLITE
		if (GameConfig::GetValue("Debug", "FixAudio", 0)) // solid workaround for making cutscenes play with directional/3D audio (breaks the volume sliders)
														  // keep the name unchanged as we could use this in the future for fixing music or other audio issues?
		{
			static SafetyHookMid XACT3DAudioHack = safetyhook::create_mid(0x0047EA5E, &Cutscene3DAudio);
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
	void ChangeSpeakerCount()
	{
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