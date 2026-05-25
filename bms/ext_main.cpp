#ifdef SE_BMS

#include "extension.h"
#include "util.h"

#include "bms/core.h"
#include "bms/ext_main.h"
#include "bms/hooks_specific.h"

void DeinitExtension()
{
    ForceMemoryAccess();
    RestoreMemorySnapshots();
    RestoreMemoryProtections();
    ClearLoadedLibraries();
    
    rootconsole->ConsolePrint("----------------------  Black Mesa " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " unloaded!" "  ----------------------");
}

bool InitExtension()
{
    if(loaded_extension)
    {
        rootconsole->ConsolePrint("Attempted to load extension twice!");
        return false;
    }

    InitCore();
    AllowWriteToMappedMemory();

    char* root_dir = getenv("PWD");
    size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "/bms/bin/server_srv.so");
    snprintf(engine_srv_fullpath, max_path_length, "/bin/engine_srv.so");
    snprintf(vphysics_srv_fullpath, max_path_length, "/bin/vphysics_srv.so");
    snprintf(dedicated_srv_fullpath, max_path_length, "/bin/dedicated_srv.so");

    Library* server_srv_lib = FindLibrary(server_srv_fullpath, true);
    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, true);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, true);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, true);

    if(!(engine_srv_lib && server_srv_lib && vphysics_srv_lib && dedicated_srv_lib))
    {
        RestoreMemoryProtections();
        ClearLoadedLibraries();
        rootconsole->ConsolePrint("----------------------  Failed to load Black Mesa " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

    server_srv = server_srv_lib->library_base_address;
    engine_srv = engine_srv_lib->library_base_address;
    vphysics_srv = vphysics_srv_lib->library_base_address;
    dedicated_srv = dedicated_srv_lib->library_base_address;

    server_srv_size = server_srv_lib->library_size;
    engine_srv_size = engine_srv_lib->library_size;
    vphysics_srv_size = vphysics_srv_lib->library_size;
    dedicated_srv_size = dedicated_srv_lib->library_size;

    last_ragdoll_gib = 0;
    ragdoll_breaking_gib_counter = 0;
    is_currently_ragdoll_breaking = false;

    fields.gEntList = server_srv + 0x01888440;
    fields.g_EventQueue = server_srv + 0x01880E20;

    fields.sv = engine_srv + 0x00364D60;
    fields.sv_cheats_cvar = engine_srv + 0x00364A80;

    fields.deferMindist = vphysics_srv + 0x001EA640;

    offsets.classname_offset = 0x64;
    offsets.abs_origin_offset = 0x294;
    offsets.origin_offset = 0x31C;
    offsets.abs_angles_offset = 0x310;
    offsets.angles_offset = 0x328;
    offsets.abs_velocity_offset = 0x22C;
    offsets.velocity_offset = 0x2A0;
    offsets.mnetwork_offset = 0x20;
    offsets.refhandle_offset = 0x334;
    offsets.iserver_offset = 0x14;
    offsets.collision_property_offset = 0x160;
    offsets.ismarked_offset = 0x118;
    offsets.vphysics_object_offset = 0x1F8;
    offsets.m_CollisionGroup_offset = 500;
    offsets.cvarstring_offset = 0x24;
    offsets.isclientactive_offset = 0x6C;
    offsets.maxclients_offset = 0x14C;
    offsets.current_map_offset = 0x11;
    offsets.getposition_vphysics_offset = 0xC0;
    offsets.setposition_vphysics_offset = 0xB8;
    offsets.cbaseclient_userid_offset = 0x14;
    offsets.enemy_offset = 0x9E8;

    functions.PackedStoreDestructor = (pOneArgProt)(dedicated_srv + 0x000CA410);
    functions.CanSatisfyVpkCacheInternal = (pSevenArgProt)(dedicated_srv + 0x000CE250);
    
    functions.recheck_ov_element = (pTwoArgProt)(vphysics_srv + 0x0013C8F0);
    functions.IVP_Real_Object_Destructor = (pOneArgProt)(vphysics_srv + 0x00119760);

    functions.SpawnPlayer = (pOneArgProt)(server_srv + 0x005F9B60);
    functions.RemoveNormalDirect = (pOneArgProt)(server_srv + 0x00B470F0);
    functions.RemoveNormal = (pOneArgProt)(server_srv + 0x00B47180);
    functions.RemoveInsta = (pOneArgProt)(server_srv + 0x00B47260);
    functions.PhysSimEnt = (pOneArgProt)(server_srv + 0x00A330E0);
    functions.AcceptInput = (pSixArgProt)(server_srv + 0x0054C3D0);
    functions.UpdateOnRemoveBase = (pOneArgProt)(server_srv + 0x00549CC0);
    functions.VphysicsSetObject = (pOneArgProt)(server_srv + 0x002A45D0);
    functions.ClearAllEntities = (pOneArgProt)(server_srv + 0x00863660);
    functions.FindEntityByClassname = (pThreeArgProt)(server_srv + 0x008638F0);
    functions.CleanupDeleteList = (pOneArgProt)(server_srv + 0x008633D0);
    functions.PostSystemsThink = (pZeroArgProt)(server_srv + 0x0037CD70);
    functions.PreSystemsThink = (pZeroArgProt)(server_srv + 0x0037CCE0);
    functions.ServiceEvents = (pOneArgProt)(server_srv + 0x00831DE0);
    functions.Physics_RunThinkFunctions = (pOneArgProt)(server_srv + 0x00A33680);
    functions.SetOwnerEntity = (pTwoArgProt)(server_srv + 0x005442B0);
    functions.DispatchAnimEvents = (pTwoArgProt)(server_srv + 0x005211C0);
    functions.CalcAbsolutePosition = (pOneArgProt)(server_srv + 0x0054B740);
    functions.VPhysicsUpdate = (pTwoArgProt)(server_srv + 0x002A5300);
    functions.SetAbsOrigin = (pTwoArgProt)(server_srv + 0x0054E3D0);
    functions.SetLocalOrigin = (pTwoArgProt)(server_srv + 0x0054FC10);

    functions.GetEnemy = (pOneArgProt)(server_srv + 0x0042AB80);
    functions.GetEnemy2 = (pOneArgProt)(server_srv + 0x0042ABD0);

    functions.SetEnemy = (pThreeArgProt)(server_srv + 0x00448540);
    
    black_mesa_functions.CXenShieldController_UpdateOnRemove = (pOneArgProt)(server_srv + 0x006827B0);
    black_mesa_functions.InputSetCSMVolume = (pTwoArgProt)(server_srv + 0x008781A0);
    black_mesa_functions.InputApplySettings = (pTwoArgProt)(server_srv + 0x009C4990);
    black_mesa_functions.CNihiBallzDestructor = (pOneArgProt)(server_srv + 0x0077DA80);

    PopulateHookExclusionLists();

    InitUtil();

    ApplyPatches();
    ApplyPatchesSpecific();
    
    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("\n\nServer Map: [%s]\n\n", fields.sv+offsets.current_map_offset);
    rootconsole->ConsolePrint("----------------------  Black Mesa " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
    loaded_extension = true;

    return true;
}

void ApplyPatches()
{
    uint32_t offset = 0;

    uint32_t patch_vpk_cache_buffer = dedicated_srv + 0x000CE668;
    memset((void*)patch_vpk_cache_buffer, 0x90, 0x17);

    //edx
    *(uint8_t*)(patch_vpk_cache_buffer) = 0x89;
    *(uint8_t*)(patch_vpk_cache_buffer+1) = 0x14;
    *(uint8_t*)(patch_vpk_cache_buffer+2) = 0x24;

    patch_vpk_cache_buffer = patch_vpk_cache_buffer + 3;
    offset = (uint32_t)HooksUtil::VpkCacheBufferAllocHook - patch_vpk_cache_buffer - 5;
    *(uint8_t*)(patch_vpk_cache_buffer) = 0xE8;
    *(uint32_t*)(patch_vpk_cache_buffer+1) = offset;

    uint32_t force_jump_vpk_allocation = dedicated_srv + 0x000CE5B8;
    memset((void*)force_jump_vpk_allocation, 0x90, 6);

    *(uint8_t*)(force_jump_vpk_allocation) = 0xE9;
    *(uint32_t*)(force_jump_vpk_allocation+1) = 0xAB;

    uint32_t phys_freeze_fix = server_srv + 0x0039CA56;
    *(uint8_t*)(phys_freeze_fix) = 0xEB;

    uint32_t hook_run_think_functions = server_srv + 0x008C6DF3;
    offset = (uint32_t)HooksUtil::SimulateEntitiesHook - hook_run_think_functions - 5;
    *(uint32_t*)(hook_run_think_functions+1) = offset;

    uint32_t eventqueue_hook = server_srv + 0x008C6DFD;
    offset = (uint32_t)HooksUtil::EmptyCall - eventqueue_hook - 5;
    *(uint32_t*)(eventqueue_hook+1) = offset;

    uint32_t remove_pre_systems = server_srv + 0x008C6DA1;
    offset = (uint32_t)HooksUtil::EmptyCall - remove_pre_systems - 5;
    *(uint32_t*)(remove_pre_systems+1) = offset;

    uint32_t remove_post_systems = server_srv + 0x008C6DF8;
    offset = (uint32_t)HooksUtil::EmptyCall - remove_post_systems - 5;
    *(uint32_t*)(remove_post_systems+1) = offset;

    //CMessageEntity
    uint32_t remove_extra_call = server_srv + 0x0094CF8A;
    offset = (uint32_t)HooksUtil::EmptyCall - remove_extra_call - 5;
    *(uint32_t*)(remove_extra_call+1) = offset;
}

void HookFunctions()
{
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.RagdollBreak, (void*)HooksBlackMesa::RagdollBreakHook);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.TestGroundMove, (void*)HooksBlackMesa::TestGroundMove);
    //HookFunction(server_srv, server_srv_size, (void*)functions.CreateNoSpawn, (void*)HooksBlackMesa::CreateNoSpawnHook);
    //HookFunction(server_srv, server_srv_size, (void*)black_mesa_functions.UTIL_GetLocalPlayer, (void*)HooksBlackMesa::UTIL_GetLocalPlayerHook);
    
    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveNormalDirect, (void*)HooksUtil::UTIL_RemoveHookFailsafe);
    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveNormal, (void*)HooksUtil::UTIL_RemoveBaseHook);
    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveInsta, (void*)HooksUtil::HookInstaKill);
    HookFunction(server_srv, server_srv_size, (void*)functions.ClearAllEntities, (void*)HooksUtil::GlobalEntityListClear);
    HookFunction(server_srv, server_srv_size, (void*)functions.SpawnPlayer, (void*)HooksUtil::PlayerSpawnHook);
}

uint32_t HooksBlackMesa::RagdollBreakHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(IsEntityValid(arg0))
    {
        ragdoll_breaking_gib_counter = 0;
        is_currently_ragdoll_breaking = true;

        pDynamicThreeArgFunc = (pThreeArgProt)(black_mesa_functions.RagdollBreak);
        pDynamicThreeArgFunc(arg0, arg1, arg2);

        is_currently_ragdoll_breaking = false;
        HandleSpecificEntityRemoval(last_ragdoll_gib, true, true, true, true);
        last_ragdoll_gib = 0;

        if(IsEntityValid(arg0) == 0)
        {
            rootconsole->ConsolePrint("Runtime error - failed to maintain entity integrity! (Ragdoll breaking)");
            exit(EXIT_FAILURE);
            return 0;
        }
    }

    rootconsole->ConsolePrint("Failed to ragdoll break!");
    return 0;
}

uint32_t HooksBlackMesa::CreateNoSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    pDynamicFourArgFunc = (pFourArgProt)(functions.CreateNoSpawn);
    uint32_t new_object = pDynamicFourArgFunc(arg0, arg1, arg2, arg3);

    if(is_currently_ragdoll_breaking)
    {
        ragdoll_breaking_gib_counter++;

        if(ragdoll_breaking_gib_counter == 4)
        {
            last_ragdoll_gib = new_object;
        }
        else if(ragdoll_breaking_gib_counter >= 4)
        {
            rootconsole->ConsolePrint("Ignored gib from ragdoll breaker!");
            HandleSpecificEntityRemoval(new_object, true, true, true, true);
    
            if(IsEntityValid(last_ragdoll_gib))
            {
                return last_ragdoll_gib;
            }
    
            rootconsole->ConsolePrint("First gib was removed!!! - Critical Error");
            exit(EXIT_FAILURE);
            return 0;
        }
    }

    return new_object;
}

uint32_t HooksBlackMesa::UTIL_GetLocalPlayerHook()
{
    pZeroArgProt pDynamicZeroArgFunc;

    pDynamicZeroArgFunc = (pZeroArgProt)(black_mesa_functions.UTIL_GetLocalPlayer);
    uint32_t returnVal = pDynamicZeroArgFunc();

    if(!returnVal)
        return functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"player");

    return returnVal;
}

uint32_t HooksUtil::SimulateEntitiesHook(uint8_t simulating)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    isTicking = true;

    RemoveBadEnts();

    SetServerSleepStatus();
    CorrectPhysics();

    UpdateCollisions(true);

    if(server_sleeping)
    {
        //rootconsole->ConsolePrint("No players exist on server skipping simulation!");
        return 0;
    }

    functions.CleanupDeleteList(0);

    pDynamicZeroArgFunc = (pZeroArgProt)(functions.PreSystemsThink);
    pDynamicZeroArgFunc();

    functions.CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(functions.Physics_RunThinkFunctions);
    pDynamicOneArgFunc(simulating);

    functions.CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(functions.ServiceEvents);
    pDynamicOneArgFunc(fields.g_EventQueue);

    UpdateCollisions(true);

    pDynamicZeroArgFunc = (pZeroArgProt)(functions.PostSystemsThink);
    pDynamicZeroArgFunc();

    UpdateCollisions(true);

    return 0;
}

uint32_t HooksBlackMesa::TestGroundMove(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pSevenArgProt pDynamicSevenArgProt;

    if(arg6)
    {
        float inf_val_chk = *(float*)(arg6+0x20);

        if((inf_val_chk != 0) && ((inf_val_chk / 2) == inf_val_chk))
        {
            rootconsole->ConsolePrint("+Inf detected!");
            return 0;
        }
    }

    pDynamicSevenArgProt = (pSevenArgProt)(black_mesa_functions.TestGroundMove);
    return pDynamicSevenArgProt(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}

uint32_t HooksUtil::UTIL_RemoveHookFailsafe(uint32_t arg0)
{
    // THIS IS UTIL_Remove(IServerNetworable*)
    // THIS HOOK IS FOR UNUSUAL CALLS TO UTIL_Remove probably from sourcemod!

    if(arg0 == 0) return 0;
    uint32_t cbase = arg0-offsets.iserver_offset;

    HandleSpecificEntityRemoval(cbase, true, true, true, true);
    return 0;
}

uint32_t HooksUtil::UTIL_RemoveBaseHook(uint32_t arg0)
{
    HandleSpecificEntityRemoval(arg0, true, true, true, true);
    return 0;
}

uint32_t HooksUtil::HookInstaKill(uint32_t arg0)
{
    HandleSpecificEntityRemoval(arg0, true, true, false, true);
    return 0;
}

uint32_t HooksUtil::GlobalEntityListClear(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    //LogVpkMemoryLeaks();

    DeleteAllValuesInList(players_connect_commands_list, false, NULL);

    isTicking = false;
    firstplayer_hasjoined = false;

    pDynamicOneArgFunc = (pOneArgProt)(functions.ClearAllEntities);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksUtil::PlayerSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    firstplayer_hasjoined = true;

    pDynamicOneArgFunc = (pOneArgProt)(functions.SpawnPlayer);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksUtil::SetEnemyHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(classname && strcmp(classname, "npc_nihilanth") == 0)
    {
        rootconsole->ConsolePrint("\nBlocked SetEnemy for npc_nihilanth\n");
        return 0;
    }

    pDynamicThreeArgFunc = (pThreeArgProt)(functions.SetEnemy);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksUtil::GetEnemyHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(functions.GetEnemy);
    uint32_t enemy = pDynamicOneArgFunc(arg0);

    //apply return address check here

    return enemy;
}

// SE_BMS
#endif
