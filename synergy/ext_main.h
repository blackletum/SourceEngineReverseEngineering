#ifndef EXT_MAIN_H
#define EXT_MAIN_H

void DeinitExtension();
bool InitExtension();
void ApplyPatches();
void HookFunctions();

class HooksSynergy
{
public:
	static uint32_t SaveGameStateHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t RestorePlayerHook(uint32_t arg0, uint32_t arg1);
	static uint32_t fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t LookupPoseParameterDropshipHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CombineDropshipSpawnHook(uint32_t arg0);
	static uint32_t PrepForLevelTransitionHook(uint32_t arg0);

	static __attribute__((fastcall)) uint32_t AutosaveHook(uint32_t arg0);
};

#endif