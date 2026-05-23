#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"

#include "synergy/core.h"
#include "synergy/hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t offset = 0;

    //CBaseEntity* corruption fix
    uint32_t antlion_guard_fix_one = server_srv->start_address + 0x00A2B5F4;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - antlion_guard_fix_one - 5;
    *(uint32_t*)(antlion_guard_fix_one+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t antlion_guard_fix_two = server_srv->start_address + 0x00A2B610;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - antlion_guard_fix_two - 5;
    *(uint32_t*)(antlion_guard_fix_two+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t tripmine_fix_one = server_srv->start_address + 0x00D11E75;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - tripmine_fix_one - 5;
    *(uint32_t*)(tripmine_fix_one+1) = offset;

    //CBaseEntity* corruption fix
    uint32_t combineball_fix = server_srv->start_address + 0x00BA6675;
    offset = (uint32_t)HooksUtil::StrictEntityValidationSlow - combineball_fix - 5;
    *(uint32_t*)(combineball_fix+1) = offset;
}

void HookFunctionsSpecific()
{
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint, (void*)NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehavior_GetEntryTarget, (void*)NativeHooks::CAI_PassengerBehavior_GetEntryTarget_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions_Hook);

    HookFunction(server_srv, (void*)synergy_functions.CSoundControllerImp_SoundChangeVolume, (void*)NativeHooks::CSoundControllerImp_SoundChangeVolume_Hook);
    HookFunction(server_srv, (void*)synergy_functions.CNPC_RollerMine_InputJoltVehicle, (void*)NativeHooks::CNPC_RollerMine_InputJoltVehicle_Hook);
    HookFunction(server_srv, (void*)synergy_functions.UTIL_GetPlayerMP, (void*)NativeHooks::UTIL_GetPlayerMP_Hook);
    HookFunction(server_srv, (void*)synergy_functions.ReleaseManhack, (void*)NativeHooks::ReleaseManhackHook);
    HookFunction(server_srv, (void*)synergy_functions.CombineBallGunDrop, (void*)NativeHooks::CombineBallGunDropHook);
}

uint32_t NativeHooks::CombineBallGunDropHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgProt;

    uint32_t vphysics_object = *(uint32_t*)(arg0+offsets.vphysics_object_offset);

    if(!IsEntityValid(arg0) || !vphysics_object)
    {
        rootconsole->ConsolePrint("Entity failed - combine ball!");
        return 0;
    }

    pDynamicThreeArgProt = (pThreeArgProt)(synergy_functions.CombineBallGunDrop);
    return pDynamicThreeArgProt(arg0, arg1, arg2);
}

uint32_t NativeHooks::ReleaseManhackHook(uint32_t arg0)
{
    pOneArgProtFastCall pDynamicOneArgFastCall;

    pDynamicOneArgFastCall = (pOneArgProtFastCall)(synergy_functions.ReleaseManhack);
    uint32_t returnVal = pDynamicOneArgFastCall(arg0);

    uint32_t refhandle = *(uint32_t*)(arg0+synergy_offsets.metropolice_manhack_offset);
    uint32_t object = GetCBaseEntity(refhandle);

    if(!IsEntityValid(object))
    {
        rootconsole->ConsolePrint("Manhack failed!");

        uint32_t new_object = functions.CreateEntityByName((uint32_t)"npc_manhack", -1);
        functions.DispatchSpawn(new_object);

        uint32_t refhandle_new = *(uint32_t*)(new_object+offsets.refhandle_offset);
        *(uint32_t*)(arg0+synergy_offsets.metropolice_manhack_offset) = refhandle_new; 
    }

    return returnVal;
}

uint32_t NativeHooks::CSoundControllerImp_SoundChangeVolume_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    if(arg1)
    {
        pDynamicFourArgFunc = (pFourArgProt)(synergy_functions.CSoundControllerImp_SoundChangeVolume);
        return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
    }

    rootconsole->ConsolePrint("Prevented crash in helicopter sound system");
    return 0;
}

uint32_t NativeHooks::CNPC_RollerMine_InputJoltVehicle_Hook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t ent_check = GetCBaseEntity(*(uint32_t*)(arg0+0x0F64));

    if(IsEntityValid(ent_check))
    {
        pDynamicOneArgFunc = (pOneArgProt)(synergy_functions.CNPC_RollerMine_InputJoltVehicle);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Failed to service jolt on vehicle");
    return 0;
}

uint32_t NativeHooks::UTIL_GetPlayerMP_Hook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(synergy_functions.UTIL_GetPlayerMP);
    uint32_t returnVal = pDynamicTwoArgFunc(arg0, arg1);

    if(returnVal == 0)
    {
        //rootconsole->ConsolePrint("UTIL_GetPlayerMP failed!");

        uint32_t player = functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"player");

        if(IsEntityValid(player))
            return player;
        
        return functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"worldspawn");
    }

    return returnVal;
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions_Hook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Bad Entity - GatherVehicleStateConditions");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_GetEntryTarget_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicThreeArgFunc = (pThreeArgProt)(synergy_functions.CAI_PassengerBehavior_GetEntryTarget);
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    rootconsole->ConsolePrint("Bad Entity - GetEntryTarget");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence_Hook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Bad Entity - FindEntrySequence");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint_Hook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Bad Entity - ReserveEntryPoint");
    return 0;
}

// SE_SDK2013
#endif