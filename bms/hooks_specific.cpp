#ifdef SE_BMS

#include "extension.h"
#include "util.h"

#include "bms/core.h"
#include "bms/hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t offset = 0;
}

void HookFunctionsSpecific()
{
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.EnumElement, (void*)NativeHooks::EnumElementHook);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.TakeDamage, (void*)NativeHooks::TakeDamageHook);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.CPropHevCharger_ShouldApplyEffect, (void*)NativeHooks::CPropHevCharger_ShouldApplyEffect);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.CPropRadiationCharger_ShouldApplyEffect, (void*)NativeHooks::CPropRadiationCharger_ShouldApplyEffect);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.LaunchMortar, (void*)NativeHooks::LaunchMortarHook);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.ShouldHitEntity, (void*)NativeHooks::ShouldHitEntityHook);

    HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.CNihiBallzDestructor, (void*)NativeHooks::CNihiBallzDestructor);
    HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.InputApplySettings, (void*)NativeHooks::InputApplySettingsHook);
    HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.InputSetCSMVolume, (void*)NativeHooks::InputSetCSMVolumeHook);
    HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.CXenShieldController_UpdateOnRemove, (void*)NativeHooks::CXenShieldController_UpdateOnRemoveHook);
}

uint32_t NativeHooks::ShouldHitEntityHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    if(arg1)
    {
        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(arg1))+0x18) );
        uint32_t object = pDynamicOneArgFunc(arg1);

        if(IsEntityValid(object))
        {
            uint32_t vphysics_object = *(uint32_t*)(object+offsets.vphysics_object_offset);

            if(vphysics_object)
            {
                pDynamicThreeArgFunc = (pThreeArgProt)(black_mesa_functions.ShouldHitEntity);
                return pDynamicThreeArgFunc(arg0, arg1, arg2);
            }
        }
    }

    rootconsole->ConsolePrint("ShouldHitEntity failed!");
    return 0;
}

uint32_t NativeHooks::CXenShieldController_UpdateOnRemoveHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(black_mesa_functions.CXenShieldController_UpdateOnRemove);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    //Add missing call to UpdateOnRemove
    functions.UpdateOnRemoveBase(arg0);

    return returnVal;
}

uint32_t NativeHooks::LaunchMortarHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(black_mesa_functions.LaunchMortar);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Gonarch was invalid!");
    return 0;
}

uint32_t NativeHooks::InputSetCSMVolumeHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t base_offset = *(uint32_t*)(arg1);
        uint32_t fourth_offset = *(uint32_t*)(arg1+4);

        if(base_offset && fourth_offset)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.InputSetCSMVolume);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Entity was NULL");
    return 0;
}

uint32_t NativeHooks::InputApplySettingsHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t entity_offset = 0x368;
    uint32_t object = *(uint32_t*)(arg0+entity_offset);

    if(IsEntityValid(object) == 0)
    {
        *(uint32_t*)(arg0+entity_offset) = 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.InputApplySettings);
    return pDynamicTwoArgFunc(arg0, arg1);
}


uint32_t NativeHooks::CNihiBallzDestructor(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t entity_one_offset = 0x738;
    uint32_t entity_two_offset = 0x73C;

    uint32_t cbaseobject_one = *(uint32_t*)(arg0+entity_one_offset);
    uint32_t cbaseobject_two = *(uint32_t*)(arg0+entity_two_offset);

    if(IsEntityValid(cbaseobject_one) == 0)
    {
        *(uint32_t*)(arg0+entity_one_offset) = 0;
    }

    if(IsEntityValid(cbaseobject_two) == 0)
    {
        *(uint32_t*)(arg0+entity_two_offset) = 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(black_mesa_functions.CNihiBallzDestructor);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::EnumElementHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.EnumElement);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    //rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t NativeHooks::TakeDamageHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t chkRef = *(uint32_t*)(arg1+0x28);
        uint32_t object = GetCBaseEntity(chkRef);

        if(IsEntityValid(object))
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.TakeDamage);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Fixed crash in take damage function");
    return 0;
}

uint32_t NativeHooks::CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.CPropHevCharger_ShouldApplyEffect);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(black_mesa_functions.CPropRadiationCharger_ShouldApplyEffect);
    return pDynamicTwoArgFunc(arg0, arg1);
}

// SE_BMS
#endif