#include "../FileLogger.h"
#include "../Patcher/patch.h"
#include "../SafeWrite.h"
#include "../GameConfig.h"
#include "GLua.h"
#include "..\LuaHandler.h"

#include "../Render/Render3D.h"

#include <safetyhook.hpp>
#include "..\General\General.h"
#include "InGameConfig.h"
#include "../Player/Input.h"
typedef void(__stdcall* NeverDieT)(int character,uint8_t status);
NeverDieT NeverDie = (NeverDieT)0x00966720;

namespace GLua
{
    int __cdecl lua_func_never_die(lua_State* L)
    {
        if (lua_gettop(L) >= 1)
        {
            int player = *(int*)0x21703D4;
            if (!player) {
                lua_error(L);
                return 0;
            }
            bool status = lua_toboolean(L, 1);
            NeverDie(player, status ? 1 : 0);
            return 0;
        }
        lua_pushstring(L, "Error: Expected a boolean argument.");
        lua_error(L); 

        return 0; 
    }
    void PatchSleepHack(int value) {
        if (value == 0) {
            if(Render3D::IsSleepHooked)
            Render3D::UnHookSleep();
            Render3D::CMPatches_PatchLowSleepHack.Restore();
            Render3D::CPatches_MediumSleepHack.Restore();
        }
        else if (value == 1) {
            if (Render3D::IsSleepHooked)
            Render3D::UnHookSleep();
            Render3D::CMPatches_PatchLowSleepHack.Apply();
            Render3D::CPatches_MediumSleepHack.Restore();
        }
        else if (value == 2) {
            if (Render3D::IsSleepHooked)
            Render3D::UnHookSleep();
            Render3D::CMPatches_PatchLowSleepHack.Apply();
            Render3D::CPatches_MediumSleepHack.Apply();
        }
        else if (value == 3) {
            Render3D::HookSleep();
            Render3D::CMPatches_PatchLowSleepHack.Apply();
            Render3D::CPatches_MediumSleepHack.Apply();
        }

    }
    int __cdecl lua_func_vint_get_avg_processing_time(lua_State* L) {
        using namespace InGameConfig;
        if (L == NULL) {
            return 0;
        }
        const char* cmd = lua_tostring(L, 1);
        if (cmd == NULL) {
            lua_pushnil(L);
            return 0;
        }
        if (strcmp(cmd, "INPUT") == 0) {
            if (Input::LastInputUI() == Input::GAME_LAST_INPUT::CONTROLLER) {
                if(!Input::usePS3Prompts)
                lua_pushstring(L, "XBOX360");
                else lua_pushstring(L, "PS3");
                return 1;
            }
                lua_pushstring(L, "PC");
            return 1;
        }
        if (strcmp(cmd, "JuicedCall") == 0) {
            int value = (int)lua_tonumber(L, 2);
            GLuaWrapper(cmd, &value, false);
            lua_pushboolean(L, 1);
            return 1;
        }
        if (strcmp(cmd, "ReadInt") == 0) {
            unsigned int address = (unsigned int)lua_tonumber(L, 2);
            int value = *(int*)address;
            lua_pushnumber(L, value);
            Logger::TypedLog(CHN_LUA, "game called %s it read 0x%X and got %d", "ReadInt", address, value);
            return 1;
        }
        else if (strcmp(cmd, "WriteInt") == 0) {
            unsigned int address = (unsigned int)lua_tonumber(L, 2);
            int value = (int)lua_tonumber(L, 3);
            *(int*)address = value;
            lua_pushboolean(L, 1); // Success
            return 1;
        }
        else if (strcmp(cmd, "ReadJuiced") == 0) {
            const char* varName = lua_tostring(L, 2);
            if (varName == NULL) {
                lua_pushnil(L);
                return 1;
            }
            int value = 0;

            if (strcmp(varName, "DisableAimAssist") == 0) {
                value = Input::disable_aim_assist_noMatterInput;
                lua_pushnumber(L, value);
                return 1;
            }

            if (strcmp(varName, "SleepHack") == 0) {
                if (Render3D::IsSleepHooked)
                    value = 3;
                else if (Render3D::CPatches_MediumSleepHack.IsApplied() && Render3D::CMPatches_PatchLowSleepHack.IsApplied())
                    value = 2;
                else if (Render3D::CMPatches_PatchLowSleepHack.IsApplied() && !Render3D::CPatches_MediumSleepHack.IsApplied())
                    value = 1;
                lua_pushnumber(L, value);
                return 1;
            }

            PatchEntry* entry = FindPatchEntry(varName);
            if (entry) {
                if (entry->singlePatch)
                    value = entry->singlePatch->IsApplied();
                else if (entry->multiPatch)
                    value = entry->multiPatch->IsApplied();

                lua_pushnumber(L, value);
                return 1;
            }
            if(&value)
            InGameConfig::GLuaWrapper(varName, &value, false);

            lua_pushnumber(L, value);
            return 1;
        }
        else if (strcmp(cmd, "WriteJuiced") == 0) {
            const char* varName = lua_tostring(L, 2);
            int value = (int)lua_tonumber(L, 3);

            if (varName == NULL) {
                lua_pushboolean(L, 0); // Failure
                return 1;
            }

            if (strcmp(varName, "DisableAimAssist") == 0) {
                    Input::disable_aim_assist_noMatterInput = value;
                    GameConfig::SetValue("Gameplay", "DisableAimAssist", std::clamp(
                        (int)Input::disable_aim_assist_noMatterInput, 0, 2));
            }

            if (strcmp(varName, "SleepHack") == 0) {
                PatchSleepHack(value);
                GameConfig::SetValue("Debug", "SleepHack", value);
                lua_pushboolean(L, 1); // Success
                return 1;
            }
            PatchEntry* entry = FindPatchEntry(varName);
            if (entry) {
                if (entry->singlePatch)
                    value ? entry->singlePatch->Apply() : entry->singlePatch->Restore();
                else if (entry->multiPatch) {
                    value ? entry->multiPatch->Apply() : entry->multiPatch->Restore();
                    // Hard coded fix for X360 Gamma toggle, would implement a callback system into CMultiPatch but feels too much.
                    if (strcmp(entry->name, "VFXPlus") == 0)
                        Render3D::VFXBrightnesstoggle();
                }
                GameConfig::SetValue(entry->configApp, entry->configKey, value);
            lua_pushboolean(L, 1); // Success
            return 1;
            }
            if (&value)
                InGameConfig::GLuaWrapper(varName, &value, true);
            lua_pushboolean(L, 1);
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    typedef char(__thiscall* cellphone_dial_numberT)(const char* number);
    cellphone_dial_numberT cellphone_dial_number = (cellphone_dial_numberT)0x788840;

    char __fastcall cellphone_dial_number_c_function(const char* number, void* arg) {
        const char* JuicedUnlockAllPhoneNumber1 = "#2008";
        const char* JuicedUnlockAllPhoneNumber2 = "588444222333";
        if (!strncmp(number, JuicedUnlockAllPhoneNumber1, sizeof(JuicedUnlockAllPhoneNumber1) - 1) || !strncmp(number, JuicedUnlockAllPhoneNumber2, sizeof(JuicedUnlockAllPhoneNumber2) - 1)) {
            ((void(*)())0x6849F0)();
            // -- Clippy, there is a proper way to have the game show up a proper cellphone message but for now we'll have to do AddMessage.
            General::AddMessage(L"Juiced", L"All cheats have been unlocked!");
        }
        return cellphone_dial_number(number);
    }
    void Init() {
//#if !RELOADED
        SafeWrite32(0x00A4EC84 + 4, (UInt32)&lua_func_never_die);
        Logger::TypedLog("CHN_DBG", "address of lua func 0x%X \n", &lua_func_vint_get_avg_processing_time);
        //static SafetyHookInline memoryutils = safetyhook::create_inline(0x00B907F0, &lua_func_vint_get_avg_processing_time);
        SafeWrite32(0x00B91212 + 7, (UInt32)&lua_func_vint_get_avg_processing_time);
        WriteRelCall(0x00789018, (uint32_t)&cellphone_dial_number_c_function);
//#endif
    }
}