#ifndef EXT_MAIN_H
#define EXT_MAIN_H

void DeinitExtension();
bool InitExtension();
void ApplyPatches();
void HookFunctions();

class HooksBlackMesa
{
public:
	static uint32_t SimulateEntitiesHook(uint32_t arg0);
	static uint32_t UTIL_GetLocalPlayerHook();
	static uint32_t TestGroundMove(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6);
	static uint32_t RagdollBreakHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CreateNoSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
};

#endif