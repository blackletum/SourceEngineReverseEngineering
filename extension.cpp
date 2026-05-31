#include "extension.h"
#include "hang_watchdog.h"

extern bool InitExtension();
extern void DeinitExtension();

ServerUtils g_ServerUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_ServerUtils);

void ServerUtils::SDK_OnAllLoaded()
{
    if (InitExtension())
    {
        //InitHangWatchdog();
    }
}

void ServerUtils::SDK_OnUnload()
{
    DeinitHangWatchdog();
    DeinitExtension();
}