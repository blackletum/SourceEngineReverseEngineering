#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

void DeinitExtension()
{
    AllowWriteToMappedMemory();
    DeinitUtil();
    RestoreMemoryProtections();

    rootconsole->ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " unloaded  ----------------------");
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
    const size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char synergy_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char sdktools_path[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "/synergy/bin/server_srv.so");
    snprintf(synergy_srv_fullpath, max_path_length, "/synergy/bin/synergy_srv.so");
    snprintf(engine_srv_fullpath, max_path_length, "/bin/engine_srv.so");
    snprintf(dedicated_srv_fullpath, max_path_length, "/bin/dedicated_srv.so");
    snprintf(vphysics_srv_fullpath, max_path_length, "/bin/vphysics_srv.so");
    snprintf(sdktools_path, max_path_length, "/extensions/sdktools.ext.2.sdk2013.so");

    Library* server_srv_lib = FindLibrary(server_srv_fullpath, true);
    Library* synergy_srv_lib = FindLibrary(synergy_srv_fullpath, true);
    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, true);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, true);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, true);
    Library* sdktools_lib = FindLibrary(sdktools_path, true);

    if(!(engine_srv_lib && dedicated_srv_lib && vphysics_srv_lib && server_srv_lib && synergy_srv_lib && sdktools_lib))
    {
        RestoreMemoryProtections();
        ClearLoadedLibraries();
        rootconsole->ConsolePrint("----------------------  Failed to load Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

    rootconsole->ConsolePrint("server_srv_lib [%X] size [%X]", server_srv_lib->library_base_address, server_srv_lib->library_size);
    rootconsole->ConsolePrint("synergy_srv_lib [%X] size [%X]", synergy_srv_lib->library_base_address, synergy_srv_lib->library_size);
    rootconsole->ConsolePrint("engine_srv_lib [%X] size [%X]", engine_srv_lib->library_base_address, engine_srv_lib->library_size);
    rootconsole->ConsolePrint("dedicated_srv_lib [%X] size [%X]", dedicated_srv_lib->library_base_address, dedicated_srv_lib->library_size);
    rootconsole->ConsolePrint("vphysics_srv_lib [%X] size [%X]", vphysics_srv_lib->library_base_address, vphysics_srv_lib->library_size);
    rootconsole->ConsolePrint("sdktools_lib [%X] size [%X]", sdktools_lib->library_base_address, sdktools_lib->library_size);

    server_srv = server_srv_lib->library_base_address;
    synergy_srv = synergy_srv_lib->library_base_address;
    engine_srv = engine_srv_lib->library_base_address;
    dedicated_srv = dedicated_srv_lib->library_base_address;
    vphysics_srv = vphysics_srv_lib->library_base_address;
    sdktools = sdktools_lib->library_base_address;

    server_srv_size = server_srv_lib->library_size;
    synergy_srv_size = synergy_srv_lib->library_size;
    engine_srv_size = engine_srv_lib->library_size;
    dedicated_srv_size = dedicated_srv_lib->library_size;
    vphysics_srv_size = vphysics_srv_lib->library_size;
    sdktools_size = sdktools_lib->library_size;

    sdktools_passed = IsAllowedToPatchSdkTools(sdktools, sdktools_size);

    save_frames = 0;
    savegame_delayed = 0;
    savegame = false;
    savegame_internal = false;
    savegame_autosave = false;

    save_player_vehicles_list = AllocateValuesList();

    fields.sv = engine_srv + 0x00402E58;
    fields.sv_cheats_cvar = engine_srv + 0x00402D70;

    synergy_fields.m_sbStaticPoseParamsLoadedDropship = server_srv + 0x00F6E854;
    
    fields.CGlobalEntityList = server_srv + 0x00EAB6DC;
    fields.RemoveImmediateSemaphore = server_srv + 0x00F3BDD0;
    fields.g_EventQueue = server_srv + 0x00EA2690;
    fields.modelinfo = server_srv + 0x00EC7580;

    fields.deferMindist = vphysics_srv + 0x001B3900;

    offsets.classname_offset = 0x74;
    offsets.abs_origin_offset = 0x2A4;
    offsets.origin_offset = 0x344;
    offsets.abs_angles_offset = 0x338;
    offsets.angles_offset = 0x350;
    offsets.abs_velocity_offset = 0x23C;
    offsets.velocity_offset = 0x2B0;
    offsets.mnetwork_offset = 0x30;
    offsets.refhandle_offset = 0x35C;
    offsets.iserver_offset = 0x24;
    offsets.collision_property_offset = 0x170;
    offsets.ismarked_offset = 0x128;
    offsets.vphysics_object_offset = 0x208;
    offsets.m_CollisionGroup_offset = 516;
    offsets.cvarstring_offset = 0x24;
    offsets.isclientactive_offset = 0x6C;
    offsets.maxclients_offset = 0x14C;
    offsets.current_map_offset = 0x11;
    offsets.getposition_vphysics_offset = 0xC0;
    offsets.setposition_vphysics_offset = 0xB8;

    synergy_offsets.vehicle_model_offset = 556;
    synergy_offsets.vehicle_script_offset = 1544;
    synergy_offsets.iserver_vehicle_offset = 0x674;
    synergy_offsets.base_vehicle_offset = 0x30;
    synergy_offsets.getpassengercount_offset = 0x4C;
    synergy_offsets.player_vehicle_offset = 0x0D38;
    synergy_offsets.leavevehicle_offset = 0x648;
    synergy_offsets.entervehicle_offset = 0x644;
    synergy_offsets.dropship_container_offset = 0x1030;

    functions.SendNetMsg = (pThreeArgProt)(engine_srv + 0x002D0EB0);
    functions.ClientCommand = (pFourArgProt)(engine_srv + 0x0030E300);
    functions.PEntityOfEntIndex = (pTwoArgProt)(engine_srv + 0x0030D720);
    functions.GetPlayerUserId = (pTwoArgProt)(engine_srv + 0x0030D5D0);
    functions.SV_ReplicateConVarChange = (pTwoArgProt)(engine_srv + 0x002E5410);

    functions.ServiceEvents = (pOneArgProt)(server_srv + 0x00607B40);
    functions.InvokePerFrameMethodFastCall = (pTwoArgProtFastCall)(server_srv + 0x006E6400);
    functions.InvokeMethodReverseOrderFastCall = (pTwoArgProtFastCall)(server_srv + 0x006E6130);
    functions.Physics_RunThinkFunctions = (pOneArgProt)(server_srv + 0x0074E750);
    functions.SpawnPlayer = (pOneArgProt)(server_srv + 0x00C2F260);
    functions.RemoveNormalDirect = (pOneArgProt)(server_srv + 0x008A0B70);
    functions.RemoveNormal = (pOneArgProt)(server_srv + 0x008A0C80);
    functions.RemoveInsta = (pOneArgProt)(server_srv + 0x008A0EA0);
    functions.CreateEntityByName = (pTwoArgProt)(server_srv + 0x00700690);
    functions.PhysSimEnt = (pOneArgProt)(server_srv + 0x0074E480);
    functions.AcceptInput = (pSixArgProt)(server_srv + 0x005B48F0);
    functions.UpdateOnRemoveBase = (pOneArgProt)(server_srv + 0x005AF0F0);
    functions.VphysicsSetObject = (pOneArgProt)(server_srv + 0x005D0280);
    functions.ClearAllEntities = (pOneArgProt)(server_srv + 0x0064AF80);
    functions.SetSolidFlags = (pTwoArgProt)(server_srv + 0x006158D0);
    functions.DisableEntityCollisions = (pTwoArgProt)(server_srv + 0x0077C350);
    functions.EnableEntityCollisions = (pTwoArgProt)(server_srv + 0x0077C4B0);
    functions.CollisionRulesChanged = (pOneArgProt)(server_srv + 0x005D02E0);
    functions.FindEntityByClassname = (pThreeArgProt)(server_srv + 0x0064B2B0);
    functions.CleanupDeleteList = (pOneArgProt)(server_srv + 0x0064ACF0);
    functions.SetOwnerEntity = (pTwoArgProt)(server_srv + 0x005B3EE0);
    functions.VPhysicsUpdate = (pTwoArgProt)(server_srv + 0x005CF5F0);
    functions.CalcAbsolutePosition = (pOneArgProt)(server_srv + 0x005B33F0);
    functions.DispatchAnimEvents = (pTwoArgProt)(server_srv + 0x0056DC50);
    functions.MapEntity_ParseAllEntities = (pThreeArgProt)(server_srv + 0x00700EF0);
    functions.DispatchSpawn = (pOneArgProt)(server_srv + 0x008A5F80);

    functions.PackedStoreDestructor = (pOneArgProt)(dedicated_srv + 0x000C4B70);
    functions.CanSatisfyVpkCacheInternal = (pSevenArgProt)(dedicated_srv + 0x000C7EB0);

    functions.RecheckCollisionFilter = (pOneArgProt)(vphysics_srv + 0x0004E780);

    synergy_functions.CombineDropshipSpawn = (pOneArgProt)(server_srv + 0x00AAE650);
    synergy_functions.SaveGameState = (pFourArgProt)(server_srv + 0x00BE5960);
    synergy_functions.RestorePlayer = (pTwoArgProt)(server_srv + 0x00BDC770);
    synergy_functions.Autosave_Silent = (pOneArgProtFastCall)(server_srv + 0x00BEC650);
    synergy_functions.LookupPoseParameterDropship = (pThreeArgProt)(server_srv + 0x0056F3F0);
    synergy_functions.PopulatePoseParametersDropship = (pOneArgProt)(server_srv + 0x00AAA830);
    synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint = (pTwoArgProt)(server_srv + 0x00C5F830);
    synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence = (pTwoArgProt)(server_srv + 0x00C6E560);
    synergy_functions.CAI_PassengerBehavior_GetEntryTarget = (pThreeArgProt)(server_srv + 0x00C62C70);
    synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions = (pOneArgProt)(server_srv + 0x00C66FB0);

    PopulateHookExclusionLists();

    InitUtil();

    ApplyPatches();
    ApplyPatchesSpecific();

    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("\n\nServer Map: [%s]\n\n", fields.sv+offsets.current_map_offset);
    rootconsole->ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded  ----------------------");
    loaded_extension = true;

    return true;
}

void ApplyPatches()
{
    uint32_t offset = 0;

    uint32_t hook_dedicated_vpk_malloc = dedicated_srv + 0x000C81D4;
    offset = (uint32_t)HooksUtil::VpkCacheBufferAllocHook - hook_dedicated_vpk_malloc - 5;
    *(uint32_t*)(hook_dedicated_vpk_malloc+1) = offset;

    uint32_t patch_stack_vpk_cache_allocation = dedicated_srv + 0x000C81CD;
    memset((void*)patch_stack_vpk_cache_allocation, 0x90, 7);

    *(uint8_t*)(patch_stack_vpk_cache_allocation) = 0x89;
    *(uint8_t*)(patch_stack_vpk_cache_allocation+1) = 0x34;
    *(uint8_t*)(patch_stack_vpk_cache_allocation+2) = 0x24;

    uint32_t force_jump_vpk_allocation = dedicated_srv + 0x000C80B3;
    memset((void*)force_jump_vpk_allocation, 0x90, 6);

    *(uint8_t*)(force_jump_vpk_allocation) = 0xE9;
    *(uint32_t*)(force_jump_vpk_allocation+1) = 0x112;

    if(sdktools_passed)
    {
        memset((void*)(sdktools + 0x00016903), 0x90, 2);
        memset((void*)(sdktools + 0x00016907), 0x90, 2);
    }

    uint32_t phys_freeze_fix = server_srv + 0x0077B759;
    *(uint8_t*)(phys_freeze_fix) = 0xEB;

    uint32_t hook_game_frame = server_srv + 0x006B1F04;
    offset = (uint32_t)HooksSynergy::SimulateEntitiesHook - hook_game_frame - 5;
    *(uint32_t*)(hook_game_frame+1) = offset;

    uint32_t hook_reverse_order = server_srv + 0x006B1F10;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_reverse_order - 5;
    *(uint32_t*)(hook_reverse_order+1) = offset;

    uint32_t hook_post_systems = server_srv + 0x006B1F1C;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_post_systems - 5;
    *(uint32_t*)(hook_post_systems+1) = offset;

    uint32_t hook_service_event_queue = server_srv + 0x006B1F2A;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_service_event_queue - 5;
    *(uint32_t*)(hook_service_event_queue+1) = offset;

    uint32_t nearplayer_bypass = server_srv + 0x00C2AF5C;
    *(uint8_t*)(nearplayer_bypass) = 0xE9;
    *(uint32_t*)(nearplayer_bypass+1) = 0x1A9;

    uint32_t weapon_pitch_dropship_patch = server_srv + 0x00AAAA94;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - weapon_pitch_dropship_patch - 5;
    *(uint32_t*)(weapon_pitch_dropship_patch+1) = offset;

    uint32_t weapon_yaw_dropship_patch = server_srv + 0x00AAAB02;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - weapon_yaw_dropship_patch - 5;
    *(uint32_t*)(weapon_yaw_dropship_patch+1) = offset;

    uint32_t helicopter_sphere_fix = server_srv + 0x00A44AA9;
    *(uint8_t*)(helicopter_sphere_fix) = 0xE9;
    *(uint32_t*)(helicopter_sphere_fix+1) = 0xA3;

    //spawning crash
    uint32_t patch_player_spawn_crash = server_srv + 0x00C2F3AC;
    *(uint8_t*)(patch_player_spawn_crash) = 0xEB;

    //spawning crash
    uint32_t patch_player_restore = server_srv + 0x00BDD1EC;
    memset((void*)patch_player_restore, 0x90, 0x26);

    //player vehicle restoring patch
    uint32_t removebad_restorecode = server_srv + 0x00BDD00D;
    memset((void*)removebad_restorecode, 0x90, 2);

    //CMessageEntity
    uint32_t remove_extra_call = server_srv + 0x0070720B;
    offset = (uint32_t)HooksUtil::EmptyCall - remove_extra_call - 5;
    *(uint32_t*)(remove_extra_call+1) = offset;
}

void HookFunctions()
{
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.Autosave_Silent, (void*)HooksSynergy::AutosaveHook);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.RestorePlayer, (void*)HooksSynergy::RestorePlayerHook);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.SaveGameState, (void*)HooksSynergy::SaveGameStateHook);
    HookFunction(server_srv, server_srv_size, (void*)synergy_functions.CombineDropshipSpawn, (void*)HooksSynergy::CombineDropshipSpawnHook);

    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveNormalDirect, (void*)HooksSynergy::UTIL_RemoveHookFailsafe);
    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveNormal, (void*)HooksSynergy::UTIL_RemoveBaseHook);
    HookFunction(server_srv, server_srv_size, (void*)functions.RemoveInsta, (void*)HooksSynergy::HookInstaKill);
    HookFunction(server_srv, server_srv_size, (void*)functions.ClearAllEntities, (void*)HooksSynergy::GlobalEntityListClear);
    HookFunction(server_srv, server_srv_size, (void*)functions.SpawnPlayer, (void*)HooksSynergy::PlayerSpawnHook);
    HookFunction(server_srv, server_srv_size, (void*)functions.MapEntity_ParseAllEntities, (void*)HooksSynergy::MapEntity_ParseAllEntitiesHook);

    HookFunction(vphysics_srv, vphysics_srv_size, (void*)(vphysics_srv + 0x000DC6F0), (void*)HooksSynergy::fix_wheels_hook);
}

uint32_t HooksSynergy::MapEntity_ParseAllEntitiesHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    char model_one[512] = "models/airboat.mdl";
    char model_two[512] = "models\\airboat.mdl";

    char script_one[512] = "scripts/vehicles/airboat.txt";
    char script_two[512] = "scripts\\vehicles\\airboat.txt";

    rootconsole->ConsolePrint("\nMapEntity_ParseAllEntities\n");

    pDynamicThreeArgFunc = (pThreeArgProt)(functions.MapEntity_ParseAllEntities);
    uint32_t returnVal = pDynamicThreeArgFunc(arg0, arg1, arg2);

    uint32_t airboat_one = functions.CreateEntityByName((uint32_t)"prop_vehicle_airboat", (uint32_t)-1);
    uint32_t airboat_two = functions.CreateEntityByName((uint32_t)"prop_vehicle_airboat", (uint32_t)-1);

    if(airboat_one)
    {
        *(uint32_t*)(airboat_one+synergy_offsets.vehicle_model_offset) = (uint32_t)model_one;
        *(uint32_t*)(airboat_one+synergy_offsets.vehicle_script_offset) = (uint32_t)script_one;

        functions.DispatchSpawn(airboat_one);
        HandleSpecificEntityRemoval(airboat_one, true, true);
    }

    if(airboat_two)
    {
        *(uint32_t*)(airboat_two+synergy_offsets.vehicle_model_offset) = (uint32_t)model_two;
        *(uint32_t*)(airboat_two+synergy_offsets.vehicle_script_offset) = (uint32_t)script_two;

        functions.DispatchSpawn(airboat_two);
        HandleSpecificEntityRemoval(airboat_two, true, true);
    }

    //Zero memory
    memset(model_one, 0, sizeof(model_one));
    memset(model_two, 0, sizeof(model_two));
    memset(script_one, 0, sizeof(script_one));
    memset(script_two, 0, sizeof(script_two));
    
    return returnVal;
}

uint32_t HooksSynergy::CombineDropshipSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(synergy_functions.CombineDropshipSpawn);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    *(uint8_t*)(synergy_fields.m_sbStaticPoseParamsLoadedDropship) = 0;

    //PopulatePoseParameters - Dropship
    pDynamicOneArgFunc = (pOneArgProt)(synergy_functions.PopulatePoseParametersDropship);
    pDynamicOneArgFunc(arg0);

    return returnVal;
}

uint32_t HooksSynergy::LookupPoseParameterDropshipHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    
    uint32_t dropship_container_refhandle = *(uint32_t*)(arg0+synergy_offsets.dropship_container_offset);
    uint32_t container_object = GetCBaseEntity(dropship_container_refhandle);
    uint32_t modelinfo = *(uint32_t*)(fields.modelinfo);

    if(container_object)
    {
        uint32_t studio_hdr = *(uint32_t*)(container_object+0x4B0);

        if(studio_hdr)
        {
            rootconsole->ConsolePrint("Dropship gun patched! x1");
    
            pDynamicThreeArgFunc = (pThreeArgProt)(synergy_functions.LookupPoseParameterDropship);
            return pDynamicThreeArgFunc(container_object, studio_hdr, arg2);
        }

        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(container_object))+0x1C) );
        uint32_t studio_object = pDynamicOneArgFunc(container_object);

        pDynamicTwoArgFunc = (pTwoArgProt)( *(uint32_t*)((*(uint32_t*)modelinfo)+8) );
        uint32_t final_studio = pDynamicTwoArgFunc(modelinfo, studio_object);

        if(final_studio)
        {
            rootconsole->ConsolePrint("Locked studio for dropship!");

            //CBaseAnimating::LockStudioHdr
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0056AFB0);
            pDynamicOneArgFunc(container_object);
        }

        studio_hdr = *(uint32_t*)(container_object+0x4B0);

        if(*(uint32_t*)(studio_hdr) == 0) studio_hdr = 0;

        if(studio_hdr)
        {
            rootconsole->ConsolePrint("Dropship gun patched! x2");
    
            pDynamicThreeArgFunc = (pThreeArgProt)(synergy_functions.LookupPoseParameterDropship);
            return pDynamicThreeArgFunc(container_object, studio_hdr, arg2);
        }
    }

    rootconsole->ConsolePrint("Failed to patch dropship gun!");

    pDynamicThreeArgFunc = (pThreeArgProt)(synergy_functions.LookupPoseParameterDropship);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksSynergy::RestorePlayerHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(!firstplayer_hasjoined)
    {
        savegame_delayed = 0;
    }

    firstplayer_hasjoined = true;

    pDynamicTwoArgFunc = (pTwoArgProt)(synergy_functions.RestorePlayer);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksSynergy::SimulateEntitiesHook(uint8_t simulating)
{
    pOneArgProt pDynamicOneArgFunc;

    pOneArgProtFastCall pDynamicFastCallOneArgFunc;
    pTwoArgProtFastCall pDynamicFastCallTwoArgFunc;

    save_frames++;
    savegame_delayed++;

    if(save_frames > 10000) save_frames = 10000;
    if(savegame_delayed > 10000) savegame_delayed = 10000;

    isTicking = true;

    SetServerSleepStatus();
    RemoveBadEnts();

    pDynamicFastCallTwoArgFunc = (pTwoArgProtFastCall)(functions.InvokeMethodReverseOrderFastCall);
    pDynamicFastCallTwoArgFunc(0x2D, 0);

    pDynamicFastCallTwoArgFunc = (pTwoArgProtFastCall)(functions.InvokePerFrameMethodFastCall);
    pDynamicFastCallTwoArgFunc(0x41, 0);

    *(uint8_t*)(fields.deferMindist) = 0;

    functions.CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(functions.Physics_RunThinkFunctions);
    pDynamicOneArgFunc(simulating);

    functions.CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(functions.ServiceEvents);
    pDynamicOneArgFunc(fields.g_EventQueue);

    functions.CleanupDeleteList(0);

    if(savegame || savegame_delayed == 150)
    {
        rootconsole->ConsolePrint("Autosave created!");

        FixCars();
        functions.CleanupDeleteList(0);

        savegame_autosave = true;

        pDynamicFastCallOneArgFunc = (pOneArgProtFastCall)(synergy_functions.Autosave_Silent);
        pDynamicFastCallOneArgFunc(0);

        savegame_autosave = false;

        functions.CleanupDeleteList(0);

        savegame = false;
    }

    ReplicateCheatsOnClient();
    EnterVehicles(save_player_vehicles_list);

    return 0;
}

uint32_t HooksSynergy::AutosaveHook(uint32_t arg0)
{
    savegame = true;
    return 0;
}

uint32_t HooksSynergy::SaveGameStateHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    save_frames = 0;

    MakePlayersLeaveVehicles();
    FixCars();

    rootconsole->ConsolePrint("Saving game!");

    savegame_internal = true;

    pDynamicFourArgFunc = (pFourArgProt)(synergy_functions.SaveGameState);
    uint32_t returnVal = pDynamicFourArgFunc(arg0, arg1, arg2, arg3);

    savegame_internal = false;

    return returnVal;
}

uint32_t HooksSynergy::fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(save_frames < 30)
    {
        rootconsole->ConsolePrint("Prevented vehicle exit!");
        return 0;
    }

    //rootconsole->ConsolePrint("Allowed usage!");
    
    pDynamicThreeArgFunc = (pThreeArgProt)(vphysics_srv + 0x000DC6F0);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksSynergy::UTIL_RemoveHookFailsafe(uint32_t arg0)
{
    // THIS IS UTIL_Remove(IServerNetworable*)
    // THIS HOOK IS FOR UNUSUAL CALLS TO UTIL_Remove probably from sourcemod!

    if(arg0 == 0) return 0;
    uint32_t cbase = arg0-offsets.iserver_offset;

    HandleSpecificEntityRemoval(cbase, true, true);
    return 0;
}

uint32_t HooksSynergy::UTIL_RemoveBaseHook(uint32_t arg0)
{
    HandleSpecificEntityRemoval(arg0, true, true);
    return 0;
}

uint32_t HooksSynergy::HookInstaKill(uint32_t arg0)
{
    HandleSpecificEntityRemoval(arg0, true, false);
    return 0;
}

uint32_t HooksSynergy::GlobalEntityListClear(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    //LogVpkMemoryLeaks();

    DeleteAllValuesInList(players_connect_commands_list, false, NULL);
    DeleteAllValuesInList(save_player_vehicles_list, false, NULL);

    isTicking = false;
    firstplayer_hasjoined = false;

    pDynamicOneArgFunc = (pOneArgProt)(functions.ClearAllEntities);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::PlayerSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(!firstplayer_hasjoined)
    {
        savegame_delayed = 0;
    }

    firstplayer_hasjoined = true;

    pDynamicOneArgFunc = (pOneArgProt)(functions.SpawnPlayer);
    return pDynamicOneArgFunc(arg0);
}

// SE_SDK2013
#endif