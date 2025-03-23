#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void ApplyPatchesSpecificSynergy();
void HookFunctionsSpecificSynergy();

class NativeHooks
{
public:
    static uint32_t SetOwnerEntityHook(uint32_t arg0, uint32_t arg1);
    static uint32_t CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions(uint32_t arg0);
    static uint32_t CAI_PassengerBehavior_GetEntryTarget(uint32_t arg0, uint32_t arg1, uint32_t arg2);
    static uint32_t CAI_PassengerBehaviorCompanion_FindEntrySequence(uint32_t arg0, uint32_t arg1);
    static uint32_t CAI_PassengerBehavior_ReserveEntryPoint(uint32_t arg0, uint32_t arg1);
};

#endif