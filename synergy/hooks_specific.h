#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void ApplyPatchesSpecific();
void HookFunctionsSpecific();

class NativeHooks
{
public:
    static uint32_t CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions_Hook(uint32_t arg0);
    static uint32_t CAI_PassengerBehavior_GetEntryTarget_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
    static uint32_t CAI_PassengerBehaviorCompanion_FindEntrySequence_Hook(uint32_t arg0, uint32_t arg1);
    static uint32_t CAI_PassengerBehavior_ReserveEntryPoint_Hook(uint32_t arg0, uint32_t arg1);
    static uint32_t CSoundControllerImp_SoundChangeVolume_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
    static uint32_t CNPC_RollerMine_InputJoltVehicle_Hook(uint32_t arg0);
    static uint32_t UTIL_GetPlayerMP_Hook(uint32_t arg0, uint32_t arg1);
    static uint32_t CombineBallGunDropHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
    static uint32_t CombineAnimEventHook(uint32_t arg0, uint32_t arg1);
    static uint32_t CAI_PassengerBehavior_GetEntryPoint_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
    static uint32_t CAI_PassengerBehaviorCompanion_SelectFailSchedule_Hook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
    static uint32_t TestCollisionHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
    static uint32_t CAI_Squad_GetSquadMemberNearestToHook(uint32_t arg0, uint32_t arg1);

    static __attribute__((fastcall)) uint32_t KillSpritesManhackHook(uint32_t arg0);
    static __attribute__((fastcall)) uint32_t ReleaseManhackHook(uint32_t arg0);
    static __attribute__((fastcall)) uint32_t CAI_FollowBehavior_UpdateFollowPosition_Hook(uint32_t arg0);
};

#endif