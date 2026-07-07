#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"

#include "synergy/core.h"
#include "synergy/hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t offset = 0;

    //CBaseEntity* corruption fix
    uint32_t antlion_guard_fix_one = server_srv->start + 0x00A2B5F4;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - antlion_guard_fix_one - 5;
    *(uint32_t*)(antlion_guard_fix_one+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t antlion_guard_fix_two = server_srv->start + 0x00A2B610;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - antlion_guard_fix_two - 5;
    *(uint32_t*)(antlion_guard_fix_two+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t tripmine_fix_one = server_srv->start + 0x00D11E75;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - tripmine_fix_one - 5;
    *(uint32_t*)(tripmine_fix_one+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t combineball_fix = server_srv->start + 0x00BA6675;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - combineball_fix - 5;
    *(uint32_t*)(combineball_fix+1) = offset;
}

void HookFunctionsSpecific()
{
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint, (void*)NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehavior_GetEntryTarget, (void*)NativeHooks::CAI_PassengerBehavior_GetEntryTarget_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehavior_GetEntryPoint, (void*)NativeHooks::CAI_PassengerBehavior_GetEntryPoint_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_SelectFailSchedule, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_SelectFailSchedule_Hook);

    HookFunction(server_srv, (void*)synergy_functions.CSoundControllerImp_SoundChangeVolume, (void*)NativeHooks::CSoundControllerImp_SoundChangeVolume_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CNPC_RollerMine_InputJoltVehicle, (void*)NativeHooks::CNPC_RollerMine_InputJoltVehicle_Hook);
    HookFunction(server_srv, (void*)synergy_functions.UTIL_GetPlayerMP, (void*)NativeHooks::UTIL_GetPlayerMP_Hook);
    HookFunction(server_srv, (void*)synergy_functions.ReleaseManhack, (void*)NativeHooks::ReleaseManhackHook);
    HookFunction(server_srv, (void*)synergy_functions.CombineBallGunDrop, (void*)NativeHooks::CombineBallGunDropHook);
    HookFunction(server_srv, (void*)synergy_functions.CombineAnimEvent, (void*)NativeHooks::CombineAnimEventHook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_FollowBehavior_UpdateFollowPosition, (void*)NativeHooks::CAI_FollowBehavior_UpdateFollowPosition_Hook);
    HookFunction(server_srv, (void*)synergy_functions.KillSpritesManhack, (void*)NativeHooks::KillSpritesManhackHook);
    HookFunction(server_srv, (void*)synergy_functions.TestCollision, (void*)NativeHooks::TestCollisionHook);
}

uint32_t NativeHooks::TestCollisionHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t entity = *(uint32_t*)(arg0+4);

    if(IsEntityValid(entity))
    {
        uint32_t physics_object = *(uint32_t*)(entity+offsets.vphysics_object_offset);
        float modelscale = *(float*)(entity+offsets.modelscale_offset);

        if(physics_object || modelscale == 1.0)
        {
            return synergy_functions.TestCollision(arg0, arg1, arg2, arg3);
        }
    }

    //ConsolePrint("TestCollision failed!");
    return 0;
}

uint32_t NativeHooks::KillSpritesManhackHook(uint32_t arg0)
{
    uint32_t sprite_one = *(uint32_t*)(arg0+synergy_offsets.manhack_m_pEyeGlow);
    uint32_t sprite_two = *(uint32_t*)(arg0+synergy_offsets.manhack_m_pLightGlow);

    if(!IsEntityValid(sprite_one))
    {
        ConsolePrint("Found corrupted sprite in manhack!");
        *(uint32_t*)(arg0+synergy_offsets.manhack_m_pEyeGlow) = 0;
    }

    if(!IsEntityValid(sprite_two))
    {
        ConsolePrint("Found corrupted sprite in manhack!");
        *(uint32_t*)(arg0+synergy_offsets.manhack_m_pLightGlow) = 0;
    }

    return synergy_functions.KillSpritesManhack(arg0);
}

uint32_t NativeHooks::CAI_FollowBehavior_UpdateFollowPosition_Hook(uint32_t arg0)
{
    uint32_t an_object = *(uint32_t*)(arg0+0x0D8);

    if(an_object)
    {
        uint32_t refhandle_chk = *(uint32_t*)(an_object+4);
        uint32_t object = GetCBaseEntity(refhandle_chk);

        if(!IsEntityValid(object))
        {
            ConsolePrint("Follow failed!");
            return 0;
        }
    }

    return synergy_functions.CAI_FollowBehavior_UpdateFollowPosition(arg0);
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_SelectFailSchedule_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehaviorCompanion_SelectFailSchedule(arg0, arg1, arg2);
    }

    ConsolePrint("Bad Entity - SelectFailSchedule");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_GetEntryPoint_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehavior_GetEntryPoint(arg0, arg1, arg2, arg3);
    }

    ConsolePrint("Bad Entity - GetEntryPoint");
    return 0;
}

uint32_t NativeHooks::CombineAnimEventHook(uint32_t arg0, uint32_t arg1)
{
    uint32_t activeweapon_handle = *(uint32_t*)(arg0+offsets.activeweapon_offset);
    uint32_t activeweapon = GetCBaseEntity(activeweapon_handle);

    if(!IsEntityValid(activeweapon))
    {
        ConsolePrint("Combine Anim failed!");
        return 0;
    }
    
    return synergy_functions.CombineAnimEvent(arg0, arg1);
}

uint32_t NativeHooks::CombineBallGunDropHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    uint32_t vphysics_object = *(uint32_t*)(arg0+offsets.vphysics_object_offset);

    if(!IsEntityValid(arg0) || !vphysics_object)
    {
        ConsolePrint("Entity failed - combine ball!");
        return 0;
    }

    return synergy_functions.CombineBallGunDrop(arg0, arg1, arg2);
}

uint32_t NativeHooks::ReleaseManhackHook(uint32_t arg0)
{
    uint32_t returnVal = synergy_functions.ReleaseManhack(arg0);

    uint32_t refhandle = *(uint32_t*)(arg0+synergy_offsets.metropolice_manhack_offset);
    uint32_t object = GetCBaseEntity(refhandle);

    if(!IsEntityValid(object))
    {
        ConsolePrint("Manhack failed!");

        uint32_t new_object = functions.CreateEntityByName((uint32_t)"npc_manhack", -1);
        functions.DispatchSpawn(new_object);

        uint32_t refhandle_new = *(uint32_t*)(new_object+offsets.refhandle_offset);
        *(uint32_t*)(arg0+synergy_offsets.metropolice_manhack_offset) = refhandle_new; 
    }

    return returnVal;
}

uint32_t NativeHooks::CSoundControllerImp_SoundChangeVolume_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    if(arg1)
    {
        return synergy_functions.CSoundControllerImp_SoundChangeVolume(arg0, arg1, arg2, arg3);
    }

    ConsolePrint("Prevented crash in helicopter sound system");
    return 0;
}

uint32_t NativeHooks::CNPC_RollerMine_InputJoltVehicle_Hook(uint32_t arg0)
{
    uint32_t ent_check = GetCBaseEntity(*(uint32_t*)(arg0+0x0F64));

    if(IsEntityValid(ent_check))
    {
        return synergy_functions.CNPC_RollerMine_InputJoltVehicle(arg0);
    }

    ConsolePrint("Failed to service jolt on vehicle");
    return 0;
}

uint32_t NativeHooks::UTIL_GetPlayerMP_Hook(uint32_t arg0, uint32_t arg1)
{
    uint32_t returnVal = synergy_functions.UTIL_GetPlayerMP(arg0, arg1);

    if(returnVal == 0)
    {
        //ConsolePrint("UTIL_GetPlayerMP failed!");

        uint32_t player = functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"player");

        if(IsEntityValid(player))
            return player;
        
        return functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"worldspawn");
    }

    return returnVal;
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions_Hook(uint32_t arg0)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions(arg0);
    }

    ConsolePrint("Bad Entity - GatherVehicleStateConditions");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_GetEntryTarget_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehavior_GetEntryTarget(arg0, arg1, arg2);
    }

    ConsolePrint("Bad Entity - GetEntryTarget");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence_Hook(uint32_t arg0, uint32_t arg1)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence(arg0, arg1);
    }

    ConsolePrint("Bad Entity - FindEntrySequence");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint_Hook(uint32_t arg0, uint32_t arg1)
{
    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        return synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint(arg0, arg1);
    }

    ConsolePrint("Bad Entity - ReserveEntryPoint");
    return 0;
}

// SE_SDK2013
#endif