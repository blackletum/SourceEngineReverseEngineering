#include "extension.h"

extern bool InitExtensionBlackMesa();
extern bool InitExtensionSynergy();

extern void DeinitExtensionBlackMesa();
extern void DeinitExtensionSynergy();

SynergyUtils g_SynUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_SynUtils);

void SynergyUtils::SDK_OnAllLoaded()
{
    InitExtensionBlackMesa();
    InitExtensionSynergy();
}

void SynergyUtils::SDK_OnUnload()
{
    DeinitExtensionBlackMesa();
    DeinitExtensionSynergy();
}