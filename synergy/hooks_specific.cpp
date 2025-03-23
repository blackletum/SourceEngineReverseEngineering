#include "extension.h"
#include "util.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecificSynergy()
{
    uint32_t offset = 0;
}

void HookFunctionsSpecificSynergy()
{
    HookFunction(server_srv, server_srv_size, (void*)(server_srv + 0x005B3E40), (void*)NativeHooks::SetOwnerEntityHook);
    HookFunction(server_srv, server_srv_size, (void*)(server_srv + 0x00C5F710), (void*)NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint);
    HookFunction(server_srv, server_srv_size, (void*)(server_srv + 0x00C6E440), (void*)NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence);
    HookFunction(server_srv, server_srv_size, (void*)(server_srv + 0x00C62B50), (void*)NativeHooks::CAI_PassengerBehavior_GetEntryTarget);
    HookFunction(server_srv, server_srv_size, (void*)(server_srv + 0x00C66E90), (void*)NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions);
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = functions.GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00C66E90);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Bad Entity - GatherVehicleStateConditions");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_GetEntryTarget(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = functions.GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00C62B50);
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    rootconsole->ConsolePrint("Bad Entity - GetEntryTarget");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehaviorCompanion_FindEntrySequence(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = functions.GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00C6E440);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Bad Entity - FindEntrySequence");
    return 0;
}

uint32_t NativeHooks::CAI_PassengerBehavior_ReserveEntryPoint(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refhandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = functions.GetCBaseEntity(refhandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00C5F710);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Bad Entity - ReserveEntryPoint");
    return 0;
}

uint32_t NativeHooks::SetOwnerEntityHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x005B3E40);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    if(arg1 != 0) rootconsole->ConsolePrint("Invalid entity in SetOwnerEntity!");

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x005B3E40);
    return pDynamicTwoArgFunc(arg0, 0);
}