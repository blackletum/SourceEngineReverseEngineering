#include "extension.h"
#include "util.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecificSynergy()
{
    uint32_t offset = 0;

    uint32_t fix_ai = server_srv + 0x005703B2;
    *(uint8_t*)(fix_ai) = 0xB8;
    *(uint8_t*)(fix_ai+1) = 0xFF;
    *(uint8_t*)(fix_ai+2) = 0xFF;
    *(uint8_t*)(fix_ai+3) = 0xFF;

    uint32_t weapon_hook_one = server_srv + 0x0055CC3B;
    offset = (uint32_t)NativeHooks::WeaponGetHook - weapon_hook_one - 5;
    *(uint32_t*)(weapon_hook_one+1) = offset;

    uint32_t weapon_hook_two = server_srv + 0x0084F2A3;
    offset = (uint32_t)NativeHooks::WeaponGetHook - weapon_hook_two - 5;
    *(uint32_t*)(weapon_hook_two+1) = offset;

    uint32_t weapon_hook_three = server_srv + 0x0084F2BC;
    offset = (uint32_t)NativeHooks::WeaponGetHook - weapon_hook_three - 5;
    *(uint32_t*)(weapon_hook_three+1) = offset;

    uint32_t weapon_hook_four = server_srv + 0x0085584D;
    offset = (uint32_t)NativeHooks::WeaponGetHook - weapon_hook_four - 5;
    *(uint32_t*)(weapon_hook_four+1) = offset;

    uint32_t weapon_bug_one = server_srv + 0x00934E4E;
    offset = (uint32_t)NativeHooks::WeaponBugbaitFixHook - weapon_bug_one - 5;
    *(uint32_t*)(weapon_bug_one+1) = offset;

    uint32_t sequence_patch = server_srv + 0x003BFA1F;
    offset = (uint32_t)NativeHooks::pSeqdescHook - sequence_patch - 5;
    *(uint32_t*)(sequence_patch+1) = offset;

    uint32_t hunter_fix_crash = server_srv + 0x005799BB;
    offset = (uint32_t)NativeHooks::HunterCrashFixTwo - hunter_fix_crash - 5;
    *(uint32_t*)(hunter_fix_crash+1) = offset;

    uint32_t zombie_patch = server_srv + 0x00835823;
    memset((void*)zombie_patch, 0x90, 0x28);

    //89 0C 24

    *(uint8_t*)(zombie_patch) = 0x89;
    *(uint8_t*)(zombie_patch+1) = 0x0C;
    *(uint8_t*)(zombie_patch+2) = 0x24;

    //89 44 24 0C

    *(uint8_t*)(zombie_patch+3) = 0x89;
    *(uint8_t*)(zombie_patch+4) = 0x44;
    *(uint8_t*)(zombie_patch+5) = 0x24;
    *(uint8_t*)(zombie_patch+6) = 0x0C;

    //8D 85 28 FE FF FF

    *(uint8_t*)(zombie_patch+7) = 0x8D;
    *(uint8_t*)(zombie_patch+8) = 0x85;
    *(uint8_t*)(zombie_patch+9) = 0x28;
    *(uint8_t*)(zombie_patch+10) = 0xFE;
    *(uint8_t*)(zombie_patch+11) = 0xFF;
    *(uint8_t*)(zombie_patch+12) = 0xFF;

    //89 44 24 08

    *(uint8_t*)(zombie_patch+13) = 0x89;
    *(uint8_t*)(zombie_patch+14) = 0x44;
    *(uint8_t*)(zombie_patch+15) = 0x24;
    *(uint8_t*)(zombie_patch+16) = 0x08;

    //8D 85 18 FE FF FF

    *(uint8_t*)(zombie_patch+17) = 0x8D;
    *(uint8_t*)(zombie_patch+18) = 0x85;
    *(uint8_t*)(zombie_patch+19) = 0x18;
    *(uint8_t*)(zombie_patch+20) = 0xFE;
    *(uint8_t*)(zombie_patch+21) = 0xFF;
    *(uint8_t*)(zombie_patch+22) = 0xFF;

    //89 44 24 04

    *(uint8_t*)(zombie_patch+23) = 0x89;
    *(uint8_t*)(zombie_patch+24) = 0x44;
    *(uint8_t*)(zombie_patch+25) = 0x24;
    *(uint8_t*)(zombie_patch+26) = 0x04;

    //HOOK

    zombie_patch = server_srv + 0x00835823+27;
    offset = (uint32_t)NativeHooks::ZombiePatchHook - zombie_patch - 5;
    *(uint8_t*)(zombie_patch) = 0xE8;
    *(uint32_t*)(zombie_patch+1) = offset;
}

void HookFunctionsSpecificSynergy()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0076CFF0), (void*)NativeHooks::HelicopterCrashFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00566CA0), (void*)NativeHooks::AiHintNpcCombinePatch);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005A8680), (void*)NativeHooks::TransitionFixTheSecond);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0058FBD0), (void*)NativeHooks::PatchAnotherPlayerAccessCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0057D930), (void*)NativeHooks::BarneyThinkHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0064DD80), (void*)NativeHooks::ChkHandle);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0058BC50), (void*)NativeHooks::Outland_07_Patch);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006D4040), (void*)NativeHooks::Outland_07_Patch_Two);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006D6C90), (void*)NativeHooks::Outland_08_Patch);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00576230), (void*)NativeHooks::AssaultNpcFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0056C350), (void*)NativeHooks::BaseNPCHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008F0620), (void*)NativeHooks::SpotlightHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66AE0), (void*)NativeHooks::StuckCrashFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A184A0), (void*)NativeHooks::WeirdCrashPleaseFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00844EC0), (void*)NativeHooks::CitizenNullCrashFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00986B70), (void*)NativeHooks::OldRefUpdateFixOne);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008BF9D0), (void*)NativeHooks::PatchMissingCheck);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00943FD0), (void*)NativeHooks::FixExplodeInputCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008B10D0), (void*)NativeHooks::ManhackSpriteEntVerify);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00622240), (void*)NativeHooks::HelicopterBadDetected);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00945B80), (void*)NativeHooks::PropCombineBall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004B8F60), (void*)NativeHooks::TakeDamageHeliFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B9CF00), (void*)NativeHooks::StringCmpHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006F98E0), (void*)NativeHooks::HunterThinkCrashFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006428F0), (void*)NativeHooks::FVisibleHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A38660), (void*)NativeHooks::AnotherObjectMissingCheck);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A1E540), (void*)NativeHooks::InterPenetrationFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0056C350), (void*)NativeHooks::AiThinkFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008DA5A0), (void*)NativeHooks::CrashFixForHibernation);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00625A30), (void*)NativeHooks::FixMissingObjectHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0081E0C0), (void*)NativeHooks::PatchMissingCheckTwo);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009014F0), (void*)NativeHooks::StriderCrashFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0054A440), (void*)NativeHooks::HibernateCrashMore);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008E7C70), (void*)NativeHooks::VehicleRollermineFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00609F90), (void*)NativeHooks::FixStructNullCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0075D540), (void*)NativeHooks::FixNullCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008C1D60), (void*)NativeHooks::FixOldManhackCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00946DF0), (void*)NativeHooks::SomeEntBadUsageFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005715D0), (void*)NativeHooks::CombineAttackFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008AB540), (void*)NativeHooks::ManhackAiFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0050D010), (void*)NativeHooks::NpcSpawnFix);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0053B1E0), (void*)NativeHooks::FixAnotherAiCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0075D210), (void*)NativeHooks::FixCombineGoalCrash);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00830A40), (void*)NativeHooks::EntVerifyFixThink);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0075B850), (void*)NativeHooks::PatchNpcCrashTrainstation02);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A28DE0), (void*)NativeHooks::MissingVphysicsObjectCrashIdk);
}

uint32_t NativeHooks::FixCombineGoalCrash(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0075D210);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Combine AI Goal failed!");
    return 0;
}

uint32_t NativeHooks::ZombiePatchHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    pSixArgProt pDynamicSixArgFunc;

    if(arg3)
    {
        uint32_t arg3_ready = *(uint32_t*)(*(uint32_t*)(arg3+4));

        pDynamicSixArgFunc = (pSixArgProt)(  *(uint32_t*)((*(uint32_t*)(arg0))+0x60)  );
        return pDynamicSixArgFunc(arg0, arg1, arg2, arg3_ready, arg4, arg5);
    }

    rootconsole->ConsolePrint("Failed arg3 was NULL!");
    return 0;
}

uint32_t NativeHooks::FixOldManhackCrash(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t ref = *(uint32_t*)(arg0+0x11B8);
    uint32_t object = GetCBaseEntitySynergy(ref);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C1D60);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Manhack NULL");
    return 0;
}

uint32_t NativeHooks::FixNullCrash(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0075D540);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Found NULL!");
    return 0;
}

uint32_t NativeHooks::FixStructNullCrash(uint32_t arg0)
{
    if(*(uint32_t*)(arg0) != 0)
    {
        return *(uint32_t*)((*(uint32_t*)(arg0))+0x20) >> 3 & 1;
    }

    rootconsole->ConsolePrint("AI ROUTE DIVERT DETECTED!");
    //DONT RETURN 0 WHEN NULL
    return -1;
}

uint32_t NativeHooks::VehicleRollermineFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x0F58);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008E7C70);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Vehicle was invalid!");
    return 0;
}

uint32_t NativeHooks::HunterCrashFixTwo(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(server_sleeping && firstplayer_hasjoined)
    {
        rootconsole->ConsolePrint("Server Sleeping!");
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00579750);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::HibernateCrashMore(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(server_sleeping && firstplayer_hasjoined)
    {
        //rootconsole->ConsolePrint("Invalid Ent Hibernation!!");
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0054A440);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::pSeqdescHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    int iVar1;
    int iVar2;
    int iVar3;
    
    iVar2 = *(int *)(arg0 + 4);
    
    if(-1 < (int)arg1)
    {
        if (iVar2 == 0)
        {
            iVar3 = *(int *)(*(int *)arg0 + 0x0BC);
        }
        else
        {
            iVar3 = *(int *)(iVar2 + 0x14);
        }
        
        if((int)arg1 < iVar3)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x005246D0);
            return pDynamicTwoArgFunc(arg0, arg1);
            //goto LAB_0054e115;
        }
    }
    
    if(iVar2 == 0)
    {
        iVar3 = *(int *)(*(int *)arg0 + 0x0BC);
    }
    else
    {
        iVar3 = *(int *)(iVar2 + 0x14);
    }
    
    arg1 = 0;
    
    if(iVar3 < 1)
    {
        //create fake memory
        
        rootconsole->ConsolePrint("\n\nERROR: fake sequence forced!\n\n");

        *(uint32_t*)(fake_sequence_mem+4) = (uint32_t)"";
        return fake_sequence_mem;
    }

    rootconsole->ConsolePrint("pSeq param_1 set to 0");

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x005246D0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::WeaponBugbaitFixHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x0A54);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0063EB90);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Fixed bugbait!");
    return 0;
}

uint32_t NativeHooks::WeaponGetHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003C2120);
    uint32_t weapon_ent = pDynamicOneArgFunc(arg0);

    if(!weapon_ent)
    {
        uint32_t isWeaponValid = GetCBaseEntitySynergy(weapon_substitute);

        if(!isWeaponValid)
        {
            uint32_t newEntity = CreateEntityByNameHook__External((uint32_t)"weapon_crowbar", (uint32_t)-1);
            pDispatchSpawnFunc(newEntity);

            weapon_substitute = *(uint32_t*)(newEntity+0x350);
            isWeaponValid = newEntity;
        }

        rootconsole->ConsolePrint("Warning: failed to find a valid weapon entity!");
        return isWeaponValid;
    }

    return weapon_ent;
}

uint32_t NativeHooks::StriderCrashFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refCheck = *(uint32_t*)(arg0+0x1F0);
    uint32_t object = GetCBaseEntitySynergy(refCheck);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x009014F0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Failed to run strider function!");
    return 0;
}

uint32_t NativeHooks::FixMissingObjectHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    if(IsEntityValid(arg0))
    {
        if (*(float*)(arg0+0x3B4) != 1.0)
        {
            uint32_t vphysics_object = *(uint32_t*)(arg0+0x1FC);

            if(vphysics_object)
            {
                pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x00625A30);
                return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
            }

            rootconsole->ConsolePrint("Failed because object missing!");
            return 0;
        }
        else
        {
            pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x00625A30);
            return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
        }
    }

    rootconsole->ConsolePrint("Failed because entity was marked!");
    return 0;
}

uint32_t NativeHooks::CrashFixForHibernation(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(server_sleeping && firstplayer_hasjoined)
    {
        rootconsole->ConsolePrint("Do not run while server sleeps!");
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008DA5A0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::InterPenetrationFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    pFiveArgProt pDynamicFiveArgProt;

    if(restoring)
    {
        rootconsole->ConsolePrint("Penetration disabled while restoring!");
        return 0;
    }

    pDynamicFiveArgProt = (pFiveArgProt)(server_srv + 0x00A1E540);
    return pDynamicFiveArgProt(arg0, arg1, arg2, arg3, arg4);
}

uint32_t NativeHooks::AnotherObjectMissingCheck(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00A38660);
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    rootconsole->ConsolePrint("Entity is NOT valid!");
    return 0;
}

uint32_t NativeHooks::FVisibleHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    if(arg1)
    {
        pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x006428F0);
        return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
    }

    rootconsole->ConsolePrint("FVisible failed!");
    return 0;
}

uint32_t NativeHooks::HunterThinkCrashFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(server_sleeping && firstplayer_hasjoined)
    {
        rootconsole->ConsolePrint("Hunter failed to think!");
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006F98E0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::StringCmpHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    
    if(arg0 && arg1)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B9CF00);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("NULL string detected!");
    return 0;
}

uint32_t NativeHooks::TakeDamageHeliFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    if(arg1)
    {
        pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x004B8F60);
        return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
    }

    rootconsole->ConsolePrint("TakeDamage heli fix!");
    return 0;
}

uint32_t NativeHooks::PropCombineBall(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00945B80);
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    rootconsole->ConsolePrint("Failed to run function due to entity being marked!");
    return 0;
}

uint32_t NativeHooks::HelicopterBadDetected(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x0EB0);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00622240);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Eli_01 crash fix!");
    RemoveEntityNormalSynergy(arg0, true);
    return 0;
}

uint32_t NativeHooks::SomeEntBadUsageFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t object_direct = *(uint32_t*)(arg0+0x4D8);

    if(IsEntityValid(object_direct) == 0)
    {
        rootconsole->ConsolePrint("Manual correction: 0x4D8");
        *(uint32_t*)(arg0+0x4D8) = 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00946DF0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::ManhackSpriteEntVerify(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t sprite_one_object = *(uint32_t*)(arg0+0x0F88);
    uint32_t sprite_two_object = *(uint32_t*)(arg0+0x0F8C);

    if(IsEntityValid(sprite_one_object) == 0)
    {
        rootconsole->ConsolePrint("Manual correction: 0x0F88");
        *(uint32_t*)(arg0+0x0F88) = 0;
    }

    if(IsEntityValid(sprite_two_object) == 0)
    {
        rootconsole->ConsolePrint("Manual correction: 0x0F8C");
        *(uint32_t*)(arg0+0x0F8C) = 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008B10D0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::EntVerifyFixThink(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t cbase_object = *(uint32_t*)(arg0+0x0F8C);

    if(IsEntityValid(cbase_object) == 0)
    {
        rootconsole->ConsolePrint("Manual correction: 0x0F8C");
        *(uint32_t*)(arg0+0x0F8C) = 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00830A40);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::FixExplodeInputCrash(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        rootconsole->ConsolePrint("Explode [%s]", *(uint32_t*)(arg0+0x68));

        uint32_t refHandle_checktwo = *(uint32_t*)(arg0+0x4E8);
        uint32_t object_two = GetCBaseEntitySynergy(refHandle_checktwo);

        if(IsEntityValid(object_two))
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00943FD0);
            return pDynamicOneArgFunc(arg0);
        }
    }

    rootconsole->ConsolePrint("Failed to service explode!");
    return 0;
}

uint32_t NativeHooks::PatchMissingCheckTwo(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(arg0)) +0x180 ) );
    uint32_t entity_object = pDynamicOneArgFunc(arg0);

    if(IsEntityValid(entity_object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0081E0C0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Warning: Failed to obtain the object");
    return 0;
}

uint32_t NativeHooks::PatchMissingCheck(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(arg1)) +0x180 ) );
    uint32_t entity_object = pDynamicOneArgFunc(arg1);

    if(IsEntityValid(entity_object))
    {
        pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x008BF9D0);
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    rootconsole->ConsolePrint("Warning: Failed to obtain the object");
    return 0;
}

uint32_t NativeHooks::Outland_07_Patch(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0058BC50);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Missing entity!");
    return 0;
}

uint32_t NativeHooks::Outland_07_Patch_Two(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x006D4040);
        return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
    }

    rootconsole->ConsolePrint("Missing Entity! - Two");
    return 0;
}

uint32_t NativeHooks::Outland_08_Patch(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t refCheck = *(uint32_t*)(arg0+0x44);
    uint32_t object = GetCBaseEntitySynergy(refCheck);

    if(IsEntityValid(object))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x006D6C90);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Missing Entity! - Three");
    return 0;
}

uint32_t NativeHooks::PatchNpcCrashTrainstation02(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x28);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0075B850);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Failed to service NPC call with offset 0x28");
    return 0;
}

uint32_t NativeHooks::OldRefUpdateFixOne(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t real_object_from_struct = *(uint32_t*)(arg0+0x518);

    if(IsEntityValid(real_object_from_struct) == 0)
    {
        rootconsole->ConsolePrint("Manual correction: 0x518");
        *(uint32_t*)(arg0+0x518) = 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00986B70);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::CitizenNullCrashFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t refHandle = *(uint32_t*)(arg0+0x0A54);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(IsEntityValid(object))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00844EC0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Null ent!");
    return 0;
}

uint32_t NativeHooks::WeirdCrashPleaseFix(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProtOptLink pDynamicTwoArgFuncOptLink;

    if(IsEntityValid(arg0))
    {
        pDynamicTwoArgFuncOptLink = (pTwoArgProtOptLink)(server_srv + 0x00A184A0);
        return pDynamicTwoArgFuncOptLink(arg0, arg1);
    }

    rootconsole->ConsolePrint("Passed in object but was freed already!");
    return 0;
}

uint32_t NativeHooks::StuckCrashFix(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(!arg1)
    {
        rootconsole->ConsolePrint("Failed to find entity!");
        return 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B66AE0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::SpotlightHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    bool continue_to_code = true;
  
    if
    (
        (*(char*)(arg0+0x1038) != '\0')
    ||
        ((1 < *(int*)(arg0+0xF38) - 1U && (*(int*)(arg0+0xF38) != 4)))
    )
    {
        continue_to_code = false;
    }

    if(continue_to_code)
    {
        uint32_t uVar4 = *(uint32_t*)(arg0+0xFD0);
        uint32_t object_uVar4 = GetCBaseEntitySynergy(uVar4);

        if(IsEntityValid(object_uVar4) == 0)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008ED690);
            pDynamicOneArgFunc(arg0);

            uVar4 = *(uint32_t*)(arg0+0xFD0);
            object_uVar4 = GetCBaseEntitySynergy(uVar4);

            if(IsEntityValid(object_uVar4) == 0)
            {
                continue_to_code = false;
            }
        }
    }

    if(continue_to_code)
    {
        uint32_t ref = *(uint32_t*)(arg0+0x0FD4);
        uint32_t object = GetCBaseEntitySynergy(ref);

        if(IsEntityValid(object) == 0)
        {
            rootconsole->ConsolePrint("Failed spotlight!");
            return 0;
        }
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008F0620);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::AiThinkFix(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(server_sleeping && firstplayer_hasjoined)
    {
        rootconsole->ConsolePrint("Blocked while server sleeps!");
        return 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0056C350);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::BaseNPCHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0056C350);
        return pDynamicTwoArgFunc(arg0, arg1);
    }
    
    rootconsole->ConsolePrint("Start Task failed!");
    return 0;
}

uint32_t NativeHooks::AssaultNpcFix(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg0)
    {
        uint32_t refCheck = *(uint32_t*)(arg0+0x14);
        uint32_t object = GetCBaseEntitySynergy(refCheck);

        if(IsEntityValid(object))
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00576230);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Failed to find valid entity!");
    return 0;
}

uint32_t NativeHooks::ManhackAiFix(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    CorrectNpcAi(arg0);

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008AB540);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::CombineAttackFix(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    CorrectNpcAi(arg0);

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x005715D0);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t NativeHooks::FixAnotherAiCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    CorrectNpcAi(arg0);

    pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x0053B1E0);
    return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
}

uint32_t NativeHooks::ChkHandle(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1 == 0)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0064DD80);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    if(IsEntityValid(arg0) && IsEntityValid(arg1))
    {
        //call orig function
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0064DD80);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Invalid entity!!!");
    return 0;
}

uint32_t NativeHooks::HelicopterCrashFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0076CFF0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Bad heli!");
    return 0;
}

uint32_t NativeHooks::AiHintNpcCombinePatch(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    
    if(arg1 == 0)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00566CA0);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    if(IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00566CA0);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Broken entity!");
    return 0;
}

uint32_t NativeHooks::TransitionFixTheSecond(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0)
    {
        uint32_t refHandle = *(uint32_t*)(arg0+0x150);
        uint32_t object = GetCBaseEntitySynergy(refHandle);

        if(IsEntityValid(object))
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x005A8680);
            return pDynamicOneArgFunc(arg0);
        }
    }

    rootconsole->ConsolePrint("Failed to validate resource!");
    return 0;
}

uint32_t NativeHooks::PatchAnotherPlayerAccessCrash(uint32_t arg0)
{
    uint32_t npc_combine_s = *(uint32_t*)(arg0+0x4);

    if(IsEntityValid(npc_combine_s))
    {
        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(npc_combine_s))+0x180)    );
        uint32_t returnVal = pDynamicOneArgFunc(npc_combine_s); 

        if(IsEntityValid(returnVal))
        {
            //Call orig
            pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0058FBD0);
            return pDynamicOneArgFunc(arg0);
        }
    }
    
    return 0;
}

uint32_t NativeHooks::NpcSpawnFix(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0050D010);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("NULL found!");
    return 0;
}

uint32_t NativeHooks::BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t deref_arg4 = *(uint32_t*)(arg1);

    if(deref_arg4)
    {
        uint32_t eHandle = *(uint32_t*)(deref_arg4+0x4);
        uint32_t object = GetCBaseEntitySynergy(eHandle);

        if(IsEntityValid(object))
        {
            //Call orig func
            pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x0057D930);
            return pDynamicThreeArgFunc(arg0, arg1, arg2);
        }
    }

    rootconsole->ConsolePrint("Failed to obtain object!");
    return 0;
}

uint32_t NativeHooks::MissingVphysicsObjectCrashIdk(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(IsEntityValid(arg0))
    {
        uint32_t vphysics_object = *(uint32_t*)(arg0+0x1FC);

        if(vphysics_object)
        {
            pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00A28DE0);
            pDynamicThreeArgFunc(arg0, arg1, arg2);
        }
    }

    rootconsole->ConsolePrint("Missing vphysics object - Nov 2024");
    return 0;
}