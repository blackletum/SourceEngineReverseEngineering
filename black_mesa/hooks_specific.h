#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void ApplyPatchesSpecific();
void HookFunctionsSpecific();

class NativeHooks
{
public:
	static uint32_t CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1);
	static uint32_t CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1);
	static uint32_t TakeDamageHook(uint32_t arg0, uint32_t arg1);
	static uint32_t EnumElementHook(uint32_t arg0, uint32_t arg1);
	static uint32_t InputSetCSMVolumeHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CNihiBallzDestructor(uint32_t arg0);
	static uint32_t InputApplySettingsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t LaunchMortarHook(uint32_t arg0);
	static uint32_t CXenShieldController_UpdateOnRemoveHook(uint32_t arg0);
	static uint32_t ShouldHitEntityHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
};

#endif