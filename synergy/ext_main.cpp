#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"
#include "hang_watchdog.h"

#include <cstdarg>

#include "synergy/core.h"
#include "synergy/ext_main.h"
#include "synergy/hooks_specific.h"

void DeinitExtension()
{
    TakeRegionMemorySnapshot(false);

    RestoreMemorySnapshots();

    //dont restore this due to issues with fully working unloading!
    //unloading will only be supported for server quiting!

    //RestoreExecutableMemorySnapshots();

    RestoreMemoryProtections();
    
    ClearLoadedLibraries();

    ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " unloaded  ----------------------");
}

bool InitExtension()
{
    if(loaded_extension)
    {
        ConsolePrint("Attempted to load extension twice!");
        return false;
    }

    InitCore();
    TakeRegionMemorySnapshot(true);

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

    server_srv = FindLibrary(server_srv_fullpath, true);
    synergy_srv = FindLibrary(synergy_srv_fullpath, true);
    engine_srv = FindLibrary(engine_srv_fullpath, true);
    dedicated_srv = FindLibrary(dedicated_srv_fullpath, true);
    vphysics_srv = FindLibrary(vphysics_srv_fullpath, true);
    sdktools = FindLibrary(sdktools_path, true);

    if(!(engine_srv && dedicated_srv && vphysics_srv && server_srv && synergy_srv && sdktools))
    {
        RestoreMemoryProtections();
        ClearLoadedLibraries();
        ConsolePrint("----------------------  Failed to load Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

    ConsolePrint("server_srv_lib [%X] end [%X]", server_srv->start, server_srv->end);
    ConsolePrint("synergy_srv_lib [%X] end [%X]", synergy_srv->start, synergy_srv->end);
    ConsolePrint("engine_srv_lib [%X] end [%X]", engine_srv->start, engine_srv->end);
    ConsolePrint("dedicated_srv_lib [%X] end [%X]", dedicated_srv->start, dedicated_srv->end);
    ConsolePrint("vphysics_srv_lib [%X] end [%X]", vphysics_srv->start, vphysics_srv->end);
    ConsolePrint("sdktools_lib [%X] end [%X]", sdktools->start, sdktools->end);

    sdktools_passed = IsAllowedToPatchSdkTools(sdktools);

    save_frames = 0;
    restore_delay_frames = 0;
    savegame = false;
    savegame_internal = false;
    savegame_autosave = false;
    saved_game_once = false;
    disable_player_restore = false;

    save_player_vehicles_list = AllocateValuesList();
    save_map_vehicle_modelscale_list = AllocateValuesList();

    fields.sv = engine_srv->start + 0x00402E58;
    fields.sv_cheats_cvar = engine_srv->start + 0x00402D70;

    synergy_fields.m_sbStaticPoseParamsLoadedDropship = server_srv->start + 0x00F6E854;

    fields.gEntList = server_srv->start + 0x00EAB6DC;
    fields.g_EventQueue = server_srv->start + 0x00EA2690;
    fields.modelinfo = server_srv->start + 0x00EC7580;
    fields.g_DeleteList = server_srv->start + 0x00EA95C0+0x0C;
    fields.g_ModelLoader = engine_srv->start + 0x003F861C;
    fields.gpGlobals = server_srv->start + 0x00EC7574;

    fields.deferMindist = vphysics_srv->start + 0x001B3900;

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
    offsets.cbaseclient_userid_offset = 0x14;
    offsets.activeweapon_offset = 0x8F4;
    offsets.targetent_offset = 0x0A60;
    offsets.getcbasentity_offset = 0x1C;
    offsets.m_pGroup_offset = 0x0D8;
    offsets.enemy_offset = 0x0A58;
    offsets.modelscale_offset = 0x3C0;

    synergy_offsets.vehicle_model_offset = 556;
    synergy_offsets.vehicle_script_offset = 1544;
    synergy_offsets.iserver_vehicle_offset = 0x674;
    synergy_offsets.base_vehicle_offset = 0x30;
    synergy_offsets.getpassengercount_offset = 0x4C;
    synergy_offsets.player_vehicle_offset = 0x0D38;
    synergy_offsets.leavevehicle_offset = 0x648;
    synergy_offsets.entervehicle_offset = 0x644;
    synergy_offsets.dropship_container_offset = 0x1030;
    synergy_offsets.metropolice_manhack_offset = 0x11C4;
    synergy_offsets.manhack_m_pEyeGlow = 0x0F94;
    synergy_offsets.manhack_m_pLightGlow = 0x0F98;

    functions.SendNetMsg = (pThreeArgProt)(engine_srv->start + 0x002D0EB0);
    functions.ClientCommand = (pFourArgProt)(engine_srv->start + 0x0030E300);
    functions.PEntityOfEntIndex = (pTwoArgProt)(engine_srv->start + 0x0030D720);
    functions.GetPlayerUserId = (pTwoArgProt)(engine_srv->start + 0x0030D5D0);
    functions.SV_ReplicateConVarChange = (pTwoArgProt)(engine_srv->start + 0x002E5410);
    functions.host_changelevel = (pThreeArgProt)(engine_srv->start + 0x002684B0);
    functions.LevelChangedSnap = (pOneArgProt)(engine_srv->start + 0x002D98B0);
    functions.RemoveEntitySnapReference = (pTwoArgProt)(engine_srv->start + 0x002DA520);

    functions.ServiceEvents = (pOneArgProt)(server_srv->start + 0x00607B40);
    functions.InvokePerFrameMethodFastCall = (pTwoArgProtFastCall)(server_srv->start + 0x006E6400);
    functions.InvokeMethodReverseOrderFastCall = (pTwoArgProtFastCall)(server_srv->start + 0x006E6130);
    functions.Physics_RunThinkFunctions = (pOneArgProt)(server_srv->start + 0x0074E750);
    functions.SpawnPlayer = (pOneArgProt)(server_srv->start + 0x00C2F260);
    functions.RemoveNormalDirect = (pOneArgProt)(server_srv->start + 0x008A0B70);
    functions.RemoveNormal = (pOneArgProt)(server_srv->start + 0x008A0C80);
    functions.RemoveInsta = (pOneArgProt)(server_srv->start + 0x008A0EA0);
    functions.CreateEntityByName = (pTwoArgProt)(server_srv->start + 0x00700690);
    functions.PhysSimEnt = (pOneArgProt)(server_srv->start + 0x0074E480);
    functions.AcceptInput = (pSixArgProt)(server_srv->start + 0x005B48F0);
    functions.UpdateOnRemoveBase = (pOneArgProt)(server_srv->start + 0x005AF0F0);
    functions.VphysicsSetObject = (pOneArgProt)(server_srv->start + 0x005D0280);
    functions.ClearAllEntities = (pOneArgProt)(server_srv->start + 0x0064AF80);
    functions.SetSolidFlags = (pTwoArgProt)(server_srv->start + 0x006158D0);
    functions.DisableEntityCollisions = (pTwoArgProt)(server_srv->start + 0x0077C350);
    functions.EnableEntityCollisions = (pTwoArgProt)(server_srv->start + 0x0077C4B0);
    functions.FindEntityByClassname = (pThreeArgProt)(server_srv->start + 0x0064B2B0);
    functions.CleanupDeleteList = (pOneArgProt)(server_srv->start + 0x0064ACF0);
    functions.SetOwnerEntity = (pTwoArgProt)(server_srv->start + 0x005B3EE0);
    functions.VPhysicsUpdate = (pTwoArgProt)(server_srv->start + 0x005CF5F0);
    functions.CalcAbsolutePosition = (pOneArgProt)(server_srv->start + 0x005B33F0);
    functions.DispatchAnimEvents = (pTwoArgProt)(server_srv->start + 0x0056DC50);
    functions.MapEntity_ParseAllEntities = (pThreeArgProt)(server_srv->start + 0x00700EF0);
    functions.CEntityFactoryDictionary_Create = (pTwoArgProt)(server_srv->start + 0x0089FBE0);
    functions.DispatchSpawn = (pOneArgProt)(server_srv->start + 0x008A5F80);
    functions.AiSelectSchedule = (pOneArgProt)(server_srv->start + 0x004A6910);
    functions.AiCleanupOnDeath = (pOneArgProt)(server_srv->start + 0x004A3CA0);
    functions.MakeDormant = (pOneArgProt)(server_srv->start + 0x005B2820);
    functions.SetAbsOrigin = (pTwoArgProt)(server_srv->start + 0x005B0F70);
    functions.SetLocalOrigin = (pTwoArgProt)(server_srv->start + 0x005B0E00);
    functions.UnloadAllModels = (pOneArgProtFastCall)(engine_srv->start + 0x0027BBC0);
    functions.FindPickerEntity = (pOneArgProt)(server_srv->start + 0x0079EA30);

    functions.GetEnemy = (pOneArgProt)(server_srv->start + 0x0043B850);
    functions.GetEnemy2 = (pOneArgProt)(server_srv->start + 0x0043B8F0);

    functions.UTIL_SetModel = (pTwoArgProt)(server_srv->start + 0x008A4A20);
    functions.PrecacheModel = (pThreeArgProt)(engine_srv->start + 0x0030D080);

    functions.SetModelScale = (SetModelScaleProt)(server_srv->start + 0x0056C680);

    

    functions.EngineError = (Error)( (server_srv->start + 0x00700FB3) + (*(uint32_t*)(server_srv->start + 0x00700FB3+1)) + 5);

    functions.PackedStoreDestructor = (pOneArgProt)(dedicated_srv->start + 0x000C4B70);
    functions.CanSatisfyVpkCacheInternal = (pSevenArgProt)(dedicated_srv->start + 0x000C7EB0);

    functions.recheck_ov_element = (pTwoArgProt)(vphysics_srv->start + 0x0011F840);
    functions.IVP_Real_Object_Destructor = (pOneArgProt)(vphysics_srv->start + 0x00102880);

    synergy_functions.CombineDropshipSpawn = (pOneArgProt)(server_srv->start + 0x00AAE650);
    synergy_functions.SaveGameState = (pFourArgProt)(server_srv->start + 0x00BE5960);
    synergy_functions.RestorePlayer = (pTwoArgProt)(server_srv->start + 0x00BDC770);
    synergy_functions.Autosave_Silent = (pOneArgProtFastCall)(server_srv->start + 0x00BEC650);
    synergy_functions.LookupPoseParameterDropship = (pThreeArgProt)(server_srv->start + 0x0056F3F0);
    synergy_functions.PopulatePoseParametersDropship = (pOneArgProt)(server_srv->start + 0x00AAA830);
    synergy_functions.CAI_PassengerBehavior_ReserveEntryPoint = (pTwoArgProt)(server_srv->start + 0x00C5F830);
    synergy_functions.CAI_PassengerBehaviorCompanion_FindEntrySequence = (pTwoArgProt)(server_srv->start + 0x00C6E560);
    synergy_functions.CAI_PassengerBehavior_GetEntryTarget = (pThreeArgProt)(server_srv->start + 0x00C62C70);
    synergy_functions.CAI_PassengerBehavior_GetEntryPoint = (pFourArgProt)(server_srv->start + 0x00C5FD10);
    synergy_functions.CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions = (pOneArgProt)(server_srv->start + 0x00C66FB0);
    synergy_functions.UTIL_GetPlayerMP = (pTwoArgProt)(server_srv->start + 0x008A0F00);
    synergy_functions.CNPC_RollerMine_InputJoltVehicle = (pOneArgProt)(server_srv->start + 0x00B3AF40);
    synergy_functions.CSoundControllerImp_SoundChangeVolume = (pFourArgProt)(server_srv->start + 0x00851A40);
    synergy_functions.PrepForLevelTransition = (pOneArgProt)(server_srv->start + 0x0078B160);
    synergy_functions.Restore = (pTwoArgProt)(server_srv->start + 0x00BE01A0);
    synergy_functions.ReleaseManhack = (pOneArgProtFastCall)(server_srv->start + 0x00B0ECD0);
    synergy_functions.CombineBallGunDrop = (pThreeArgProt)(server_srv->start + 0x00BA7BE0);
    synergy_functions.ContentReset = (pVargArgProt)(synergy_srv->start + 0x00087140);
    synergy_functions.CombineAnimEvent = (pTwoArgProt)(server_srv->start + 0x00AA2270);
    synergy_functions.CAI_FollowBehavior_UpdateFollowPosition = (pOneArgProtFastCall)(server_srv->start + 0x004A2A10);
    synergy_functions.SaveRestoreFinish = (pTwoArgProt)(server_srv->start + 0x00BE6D50);
    synergy_functions.KillSpritesManhack = (pOneArgProtFastCall)(server_srv->start + 0x00AFCB70);
    synergy_functions.CAI_PassengerBehaviorCompanion_SelectFailSchedule = (pThreeArgProt)(server_srv->start + 0x00C6BCF0);
    synergy_functions.TestCollision = (pFourArgProt)(server_srv->start + 0x00616600);

    PopulateHookExclusionLists();

    InitUtil();

    ApplyPatches();
    ApplyPatchesSpecific();

    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();

    ConsolePrint("\n\nServer Map: [%s]\n\n", fields.sv+offsets.current_map_offset);
    ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded  ----------------------");
    loaded_extension = true;

    return true;
}

void ApplyPatches()
{
    uint32_t offset = 0;

    uint32_t patch_vpk_cache_allocation = dedicated_srv->start + 0x000C81CD;
    memset((void*)patch_vpk_cache_allocation, 0x90, 0xC);

    //ebx
    *(uint8_t*)(patch_vpk_cache_allocation) = 0x89;
    *(uint8_t*)(patch_vpk_cache_allocation+1) = 0x1C;
    *(uint8_t*)(patch_vpk_cache_allocation+2) = 0x24;

    //esi
    *(uint8_t*)(patch_vpk_cache_allocation+3) = 0x89;
    *(uint8_t*)(patch_vpk_cache_allocation+4) = 0x74;
    *(uint8_t*)(patch_vpk_cache_allocation+5) = 0x24;
    *(uint8_t*)(patch_vpk_cache_allocation+6) = 0x04;

    patch_vpk_cache_allocation = patch_vpk_cache_allocation + 7;
    offset = (uint32_t)HooksUtil::VpkCacheBufferAllocHook - patch_vpk_cache_allocation - 5;
    *(uint8_t*)(patch_vpk_cache_allocation) = 0xE8;
    *(uint32_t*)(patch_vpk_cache_allocation+1) = offset;

    uint32_t force_jump_vpk_allocation = dedicated_srv->start + 0x000C80B3;
    memset((void*)force_jump_vpk_allocation, 0x90, 6);

    *(uint8_t*)(force_jump_vpk_allocation) = 0xE9;
    *(uint32_t*)(force_jump_vpk_allocation+1) = 0x112;

    if(sdktools_passed)
    {
        memset((void*)(sdktools + 0x00016903), 0x90, 2);
        memset((void*)(sdktools + 0x00016907), 0x90, 2);
    }

    uint32_t remove_save_transition = server_srv->start + 0x00888A56;
    memset((void*)remove_save_transition, 0x90, 3);

    uint32_t phys_freeze_fix = server_srv->start + 0x0077B759;
    *(uint8_t*)(phys_freeze_fix) = 0xEB;

    uint32_t hook_game_frame = server_srv->start + 0x006B1F04;
    offset = (uint32_t)HooksUtil::SimulateEntitiesHook - hook_game_frame - 5;
    *(uint32_t*)(hook_game_frame+1) = offset;

    uint32_t hook_reverse_order = server_srv->start + 0x006B1F10;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_reverse_order - 5;
    *(uint32_t*)(hook_reverse_order+1) = offset;

    uint32_t hook_post_systems = server_srv->start + 0x006B1F1C;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_post_systems - 5;
    *(uint32_t*)(hook_post_systems+1) = offset;

    uint32_t hook_service_event_queue = server_srv->start + 0x006B1F2A;
    offset = (uint32_t)HooksUtil::EmptyCall - hook_service_event_queue - 5;
    *(uint32_t*)(hook_service_event_queue+1) = offset;

    uint32_t nearplayer_bypass = server_srv->start + 0x00C2AF5C;
    *(uint8_t*)(nearplayer_bypass) = 0xE9;
    *(uint32_t*)(nearplayer_bypass+1) = 0x1A9;

    uint32_t weapon_pitch_dropship_patch = server_srv->start + 0x00AAAA94;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - weapon_pitch_dropship_patch - 5;
    *(uint32_t*)(weapon_pitch_dropship_patch+1) = offset;

    uint32_t weapon_yaw_dropship_patch = server_srv->start + 0x00AAAB02;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - weapon_yaw_dropship_patch - 5;
    *(uint32_t*)(weapon_yaw_dropship_patch+1) = offset;

    uint32_t helicopter_sphere_fix = server_srv->start + 0x00A44AA9;
    *(uint8_t*)(helicopter_sphere_fix) = 0xE9;
    *(uint32_t*)(helicopter_sphere_fix+1) = 0xA3;

    //spawning crash
    uint32_t patch_player_spawn_crash = server_srv->start + 0x00C2F3AC;
    *(uint8_t*)(patch_player_spawn_crash) = 0xEB;

    //spawning crash
    uint32_t patch_player_restore = server_srv->start + 0x00BDD1EC;
    memset((void*)patch_player_restore, 0x90, 0x26);

    //player vehicle restoring patch
    uint32_t removebad_restorecode = server_srv->start + 0x00BDD00D;
    memset((void*)removebad_restorecode, 0x90, 2);

    //causes crash
    uint32_t remove_car_transition = server_srv->start + 0x00BDD1CC;
    memset((void*)remove_car_transition, 0x90, 5);

    uint32_t fix_save_transition = server_srv->start + 0x00BE597F;
    *(uint8_t*)(fix_save_transition) = 0xEB;

    uint32_t patch_player_transition = server_srv->start + 0x00885EF1;
    offset = (uint32_t)HooksSynergy::PrepForLevelTransitionHook - patch_player_transition - 5;
    *(uint32_t*)(patch_player_transition+1) = offset;

    //CMessageEntity
    uint32_t remove_extra_call = server_srv->start + 0x0070720B;
    offset = (uint32_t)HooksUtil::EmptyCall - remove_extra_call - 5;
    *(uint32_t*)(remove_extra_call+1) = offset;

    uint32_t script_think_patch = server_srv->start + 0x00832200;
    *(uint8_t*)(script_think_patch) = 0xE9;
    *(uint32_t*)(script_think_patch+1) = 0xCD;

    //UGLY PATCHES

    uint32_t sound_patch = server_srv->start + 0x0085BE12;
    offset = (uint32_t)HooksSynergy::Sound_UpdateForPlayer - sound_patch - 5;
    *(uint32_t*)(sound_patch+1) = offset;

    uint32_t baseai_patch = server_srv->start + 0x00461DF1;
    memset((void*)baseai_patch, 0x90, 0x15);

    *(uint8_t*)(baseai_patch) = 0x89;
    *(uint8_t*)(baseai_patch+1) = 0x04;
    *(uint8_t*)(baseai_patch+2) = 0x24;

    *(uint8_t*)(baseai_patch+3) = 0x89;
    *(uint8_t*)(baseai_patch+4) = 0x74;
    *(uint8_t*)(baseai_patch+5) = 0x24;
    *(uint8_t*)(baseai_patch+6) = 0x04;

    baseai_patch = baseai_patch+7;

    *(uint8_t*)(baseai_patch) = 0xE8;
    offset = (uint32_t)HooksSynergy::BaseAiPatch - baseai_patch - 5;
    *(uint32_t*)(baseai_patch+1) = offset;

    // -- END
}

void HookFunctions()
{
    HookFunction(synergy_srv, (void*)synergy_functions.ContentReset, (void*)HooksSynergy::ContentResetHook);
    HookFunction(server_srv, (void*)synergy_functions.SaveRestoreFinish, (void*)HooksSynergy::SaveRestoreFinishHook);
    HookFunction(server_srv, (void*)synergy_functions.Autosave_Silent, (void*)HooksSynergy::AutosaveHook);
    HookFunction(server_srv, (void*)synergy_functions.RestorePlayer, (void*)HooksSynergy::RestorePlayerHook);
    HookFunction(server_srv, (void*)synergy_functions.SaveGameState, (void*)HooksSynergy::SaveGameStateHook);
    HookFunction(server_srv, (void*)synergy_functions.CombineDropshipSpawn, (void*)HooksSynergy::CombineDropshipSpawnHook);
    HookFunction(server_srv, (void*)synergy_functions.Restore, (void*)HooksSynergy::RestoreHook);

    HookFunction(engine_srv, (void*)functions.LevelChangedSnap, (void*)HooksUtil::LevelChangedSnapHook);
    HookFunction(engine_srv, (void*)functions.host_changelevel, (void*)HooksUtil::host_changelevelhook);
    HookFunction(server_srv, (void*)functions.RemoveNormalDirect, (void*)HooksUtil::UTIL_RemoveHookFailsafe);
    HookFunction(server_srv, (void*)functions.RemoveNormal, (void*)HooksUtil::UTIL_RemoveBaseHook);
    HookFunction(server_srv, (void*)functions.RemoveInsta, (void*)HooksUtil::HookInstaKill);
    HookFunction(server_srv, (void*)functions.ClearAllEntities, (void*)HooksUtil::GlobalEntityListClear);
    HookFunction(server_srv, (void*)functions.SpawnPlayer, (void*)HooksUtil::PlayerSpawnHook);
    HookFunction(server_srv, (void*)functions.CEntityFactoryDictionary_Create, (void*)HooksUtil::CEntityFactoryDictionary_CreateHook);
    HookFunction(server_srv, (void*)functions.UTIL_SetModel, (void*)HooksUtil::UTIL_SetModelHook);
    HookFunction(engine_srv, (void*)functions.PrecacheModel, (void*)HooksUtil::PrecacheModelHook);
}

uint32_t HooksSynergy::BaseAiPatch(uint32_t arg0, uint32_t arg1)
{
    uint32_t esihandle = *(uint32_t*)(arg1);
    uint32_t esi_ent = GetCBaseEntity(esihandle);

    if(IsEntityValid(arg0))
    {
        pTwoArgProt Unknown = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(arg0))+0x3EC)  );
        Unknown(arg0, 0);
    }

    if(IsEntityValid(esi_ent))
    {
        return esihandle;
    }

    return -1;
}

uint32_t HooksSynergy::Sound_UpdateForPlayer(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt UpdateForPlayer = (pTwoArgProt)(server_srv->start + 0x00855D00);

    if(IsEntityValid(arg0))
    {
        return UpdateForPlayer(arg0, arg1);
    }

    ConsolePrint("Entity failed at UpdateForPlayer!");
    return 0;
}

uint32_t HooksSynergy::SaveRestoreFinishHook(uint32_t arg0, uint32_t arg1)
{
    uint32_t save_buffer = *(uint32_t*)((*(uint32_t*)fields.gpGlobals)+0x2C);

    if(save_buffer)
    {
        uint32_t leak_buffer = save_buffer+0x594;

        ConsolePrint("SaveRestoreFinish Leak %X", leak_buffer);

        pOneArgProtFastCall HashTableDestructor = (pOneArgProtFastCall)(server_srv->start + 0x00BEC0C0);
        HashTableDestructor(leak_buffer);
    }

    return synergy_functions.SaveRestoreFinish(arg0, arg1);
}

uint32_t HooksUtil::PrecacheModelHook(uint32_t this_arg, uint32_t mdlname, uint32_t preload)
{
    uint32_t current_map = fields.sv+offsets.current_map_offset;

    if(strcmp((char*)mdlname, "models/props_junk/flare.mdl") == 0)
    {
        if
        (
            strncasecmp((char*)current_map, "d1_", 3) == 0
            ||
            strncasecmp((char*)current_map, "d2_", 3) == 0
            ||
            strncasecmp((char*)current_map, "d3_", 3) == 0
        )
        {
            ConsolePrint("flare hl2 ENGINE PRECACHE");
            return functions.PrecacheModel(this_arg, (uint32_t)"models/items/flare.mdl", true);
        }
    }

     return functions.PrecacheModel(this_arg, mdlname, preload);
}

uint32_t HooksUtil::UTIL_SetModelHook(uint32_t this_arg, uint32_t mdlname)
{
    uint32_t current_map = fields.sv+offsets.current_map_offset;

    if(strcmp((char*)mdlname, "models/props_junk/flare.mdl") == 0)
    {
        if
        (
            strncasecmp((char*)current_map, "d1_", 3) == 0
            ||
            strncasecmp((char*)current_map, "d2_", 3) == 0
            ||
            strncasecmp((char*)current_map, "d3_", 3) == 0
        )
        {
            ConsolePrint("flare hl2");
            return functions.UTIL_SetModel(this_arg, (uint32_t)"models/items/flare.mdl");
        }
    }

    return functions.UTIL_SetModel(this_arg, mdlname);
}

uint32_t HooksUtil::LevelChangedSnapHook(uint32_t arg0)
{
    ReleaseLeakedPackedEntities(arg0);
    return functions.LevelChangedSnap(arg0);
}

uint32_t HooksSynergy::ContentResetHook(char *format, ...)
{
    va_list marker;
    va_start(marker, format);
    vprintf(format, marker);
    va_end(marker);
    printf("\n");

    functions.UnloadAllModels(fields.g_ModelLoader);

    return 0;
}

uint32_t HooksSynergy::PrepForLevelTransitionHook(uint32_t arg0)
{
    uint32_t refHandle = *(uint32_t*)(arg0+offsets.refhandle_offset);
    float* abs_origin_player = (float*)(arg0+offsets.abs_origin_offset);
    
    InsertToArrayList(transitioned_clients, refHandle);
    memcpy(transitioning_player, abs_origin_player, sizeof(float) * 3);

    //ConsolePrint("player detected with %f.2 %f.2 %f.2", transitioning_player[0], transitioning_player[1], transitioning_player[2]);

    return synergy_functions.PrepForLevelTransition(arg0);
}

uint32_t HooksUtil::host_changelevelhook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pFourArgProt pDynamicFourArgFunc;

    ConsolePrint("Manual Save on Transition!");

    SaveMapVehicleModelScales();
    MakePlayersLeaveVehicles();
    FixCars();

    RemoveHl2Ragdolls();
    TeleportPlayersToTransition();

    functions.CleanupDeleteList(0);

    // Save the game
    uint32_t save_thing = *(uint32_t*)(server_srv->start + 0x00EC7550+0x0C);
    uint32_t save_thing_two = *(uint32_t*)(server_srv->start + 0x0023D6D0);
    uint32_t save_thing_three = *(uint32_t*)(server_srv->start + 0x0023D6D0+4);
    uint32_t save_thing_four = *(uint32_t*)(server_srv->start + 0x0023D6D0+4+4);

    pDynamicFourArgFunc = (pFourArgProt)( *(uint32_t*)((*(uint32_t*)(save_thing))+0x48) );
    pDynamicFourArgFunc(save_thing, save_thing_two, save_thing_three, save_thing_four);

    functions.CleanupDeleteList(0);

    ZeroArrayList(transitioned_clients);
    memset(transitioning_player, 0, sizeof(transitioning_player));

    return functions.host_changelevel(arg0, arg1, arg2);
}

uint32_t HooksSynergy::CombineDropshipSpawnHook(uint32_t arg0)
{
    uint32_t returnVal = synergy_functions.CombineDropshipSpawn(arg0);

    *(uint8_t*)(synergy_fields.m_sbStaticPoseParamsLoadedDropship) = 0;
    synergy_functions.PopulatePoseParametersDropship(arg0);

    return returnVal;
}

uint32_t HooksSynergy::LookupPoseParameterDropshipHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t dropship_container_refhandle = *(uint32_t*)(arg0+synergy_offsets.dropship_container_offset);
    uint32_t container_object = GetCBaseEntity(dropship_container_refhandle);
    uint32_t modelinfo = *(uint32_t*)(fields.modelinfo);

    if(container_object)
    {
        uint32_t studio_hdr = *(uint32_t*)(container_object+0x4B0);

        if(studio_hdr)
        {
            ConsolePrint("Dropship gun patched! x1");
            return synergy_functions.LookupPoseParameterDropship(container_object, studio_hdr, arg2);
        }

        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(container_object))+0x1C) );
        uint32_t studio_object = pDynamicOneArgFunc(container_object);

        pDynamicTwoArgFunc = (pTwoArgProt)( *(uint32_t*)((*(uint32_t*)modelinfo)+8) );
        uint32_t final_studio = pDynamicTwoArgFunc(modelinfo, studio_object);

        if(final_studio)
        {
            ConsolePrint("Locked studio for dropship!");

            //CBaseAnimating::LockStudioHdr
            pDynamicOneArgFunc = (pOneArgProt)(server_srv->start + 0x0056AFB0);
            pDynamicOneArgFunc(container_object);
        }

        studio_hdr = *(uint32_t*)(container_object+0x4B0);

        if(*(uint32_t*)(studio_hdr) == 0) studio_hdr = 0;

        if(studio_hdr)
        {
            ConsolePrint("Dropship gun patched! x2");
            return synergy_functions.LookupPoseParameterDropship(container_object, studio_hdr, arg2);
        }
    }

    ConsolePrint("Failed to patch dropship gun!");
    return synergy_functions.LookupPoseParameterDropship(arg0, arg1, arg2);
}

uint32_t HooksSynergy::RestorePlayerHook(uint32_t arg0, uint32_t arg1)
{
    if(disable_player_restore)
    {
        ConsolePrint("Blocked RestorePlayer!");
        return 0;
    }

    if(!firstplayer_hasjoined)
    {
    }

    firstplayer_hasjoined = true;

    return synergy_functions.RestorePlayer(arg0, arg1);
}

uint32_t HooksUtil::SimulateEntitiesHook(uint8_t simulating)
{
    TouchMainThreadHeartbeat();

    save_frames++;
    restore_delay_frames++;

    if(save_frames > 10000) save_frames = 10000;
    if(restore_delay_frames > 10000) restore_delay_frames = 10000;

    isTicking = true;

    functions.CleanupDeleteList(0);

    SetServerSleepStatus();
    ReplicateCheatsOnClient();
    CorrectPhysics();

    if(savegame && save_frames >= 25)
    {
        ConsolePrint("Autosave created!");
        SaveGame_Extension();

        savegame = false;
    }

    EnterVehicles(save_player_vehicles_list);
    RestoreMapVehicleModelScales();

    functions.CleanupDeleteList(0);
    functions.Physics_RunThinkFunctions(simulating);
    functions.CleanupDeleteList(0);

    RemoveBadEnts();
    UpdateCollisions(true);

    //PostSystems
    functions.InvokeMethodReverseOrderFastCall(0x2D, 0);
    functions.InvokePerFrameMethodFastCall(0x41, 0);

    functions.CleanupDeleteList(0);
    functions.ServiceEvents(fields.g_EventQueue);
    functions.CleanupDeleteList(0);

    return 0;
}

uint32_t HooksSynergy::AutosaveHook(uint32_t arg0)
{
    savegame = true;
    return 0;
}

uint32_t HooksSynergy::RestoreHook(uint32_t arg0, uint32_t arg1)
{
    if(saved_game_once)
    {
        if(restore_delay_frames >= 50)
        {
            uint32_t returnVal = synergy_functions.Restore(arg0, arg1);
            SaveGame_Extension();
            restore_delay_frames = 0;
            return returnVal;
        }

        ConsolePrint("Restore failed!");
        return 0;
    }

    SaveGame_Extension();
    sleep(2);
    ConsolePrint("Skipped Restore! (loaded current game back from save)");

    disable_player_restore = true;

    uint32_t returnVal = synergy_functions.Restore(arg0, arg1);

    disable_player_restore = false;

    savegame = true;
    save_frames = 25;
    return returnVal;
}

uint32_t HooksSynergy::SaveGameStateHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    int32_t deleted_ents = *(int32_t*)fields.g_DeleteList;

    if(deleted_ents > 0)
    {
        ConsolePrint("Failed to save game");
        exit(EXIT_FAILURE);
        return 0;
    }

    ConsolePrint("Saving game!");

    savegame_internal = true;

    uint32_t returnVal = synergy_functions.SaveGameState(arg0, arg1, arg2, arg3);

    savegame_internal = false;

    if(savegame_autosave)
    {
        ConsolePrint("Blocked autosave.sav!");
        return 0;
    }

    return returnVal;
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
    //LogVpkMemoryLeaks();

    DeleteAllValuesInList(players_connect_commands_list, false, NULL);
    DeleteAllValuesInList(save_player_vehicles_list, false, NULL);
    DeleteAllValuesInList(save_map_vehicle_modelscale_list, false, NULL);

    isTicking = false;
    firstplayer_hasjoined = false;
    saved_game_once = false;

    save_frames = 0;

    return functions.ClearAllEntities(arg0);
}

uint32_t HooksUtil::CEntityFactoryDictionary_CreateHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t returnVal = functions.CEntityFactoryDictionary_Create(arg0, arg1);

    if(returnVal)
    {
        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(returnVal))+offsets.getcbasentity_offset) );
        uint32_t cbase_entity = pDynamicOneArgFunc(returnVal);

        if(cbase_entity)
        {
            if(strcmp((const char*)arg1, "prop_vehicle_mp") == 0)
            {
                *(uint8_t*)(cbase_entity+0x836) = 1;
                ConsolePrint("Vehicle Created: [%s]", arg1);
            }

            //ConsolePrint("Entity Created: [%s]", *(uint32_t*)(cbase_entity+offsets.classname_offset));
        }
    }

    return returnVal;
}

uint32_t HooksUtil::PlayerSpawnHook(uint32_t arg0)
{
    if(!firstplayer_hasjoined)
    {
    }

    firstplayer_hasjoined = true;

    return functions.SpawnPlayer(arg0);
}

uint32_t HooksUtil::SetEnemyHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));
    return functions.SetEnemy(arg0, arg1, arg2);
}

uint32_t HooksUtil::GetEnemyHook(uint32_t arg0)
{
    uint32_t enemy = functions.GetEnemy(arg0);

    if(!enemy)
    {
        if((uint32_t)__builtin_return_address(0) == (server_srv->start + 0x004BBE8B))
        {
            ConsolePrint("GetEnemy returned NULL");

            uint32_t player = functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"player");

            if(IsEntityValid(player))
                return player;
            
            return functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"worldspawn");
        }
    }

    return enemy;
}

// SE_SDK2013
#endif
