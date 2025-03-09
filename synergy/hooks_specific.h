#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void ApplyPatchesSpecificSynergy();
void HookFunctionsSpecificSynergy();

class NativeHooks
{
public:
    static uint32_t SetOwnerEntityHook(uint32_t arg0, uint32_t arg1);
};

#endif