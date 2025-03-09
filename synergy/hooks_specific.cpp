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