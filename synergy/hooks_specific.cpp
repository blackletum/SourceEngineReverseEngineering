#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t offset = 0;
}

void HookFunctionsSpecific()
{
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint, (void*)NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.CAI_PassengerBehavior_GetEntryTarget, (void*)NativeHooks::CAI_PassengerBehavior_GetEntryTarget);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions, (void*)NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions);
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions(uint32_t arg0)
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

uint32_t NativeHooks::CAI_PassengerBehavior_GetEntryTarget(uint32_t arg0, uint32_t arg1, uint32_t arg2)
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

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence(uint32_t arg0, uint32_t arg1)
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

uint32_t NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint(uint32_t arg0, uint32_t arg1)
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