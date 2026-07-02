#ifndef CORE_H
#define CORE_H

#define EXT_PREFIX "[SynergyUtils] "

typedef struct _synergy_game_fields {
    //Synergy fields
    uint32_t m_sbStaticPoseParamsLoadedDropship = 0;
} synergy_game_fields;

typedef struct _synergy_game_offsets {
    //Synergy offsets
    uint32_t vehicle_model_offset = 0;
    uint32_t vehicle_script_offset = 0;
    uint32_t iserver_vehicle_offset = 0;
    uint32_t base_vehicle_offset = 0;
    uint32_t getpassengercount_offset = 0;
    uint32_t player_vehicle_offset = 0;
    uint32_t leavevehicle_offset = 0;
    uint32_t entervehicle_offset = 0;
    uint32_t dropship_container_offset = 0;
    uint32_t metropolice_manhack_offset = 0;
    uint32_t manhack_m_pEyeGlow = 0;
    uint32_t manhack_m_pLightGlow = 0;
} synergy_game_offsets;

typedef struct _synergy_game_functions {
    //Synergy functions
    pOneArgProt CombineDropshipSpawn = 0;
    pFourArgProt SaveGameState = 0;
    pTwoArgProt RestorePlayer = 0;
    pOneArgProtFastCall Autosave_Silent = 0;
    pThreeArgProt LookupPoseParameterDropship = 0;
    pOneArgProt PopulatePoseParametersDropship = 0;
    pTwoArgProt CAI_PassengerBehavior_ReserveEntryPoint = 0;
    pTwoArgProt CAI_PassengerBehaviorCompanion_FindEntrySequence = 0;
    pThreeArgProt CAI_PassengerBehavior_GetEntryTarget = 0;
    pOneArgProt CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions = 0;
    pFourArgProt CAI_PassengerBehavior_GetEntryPoint = 0;
    pTwoArgProt UTIL_GetPlayerMP = 0;
    pOneArgProt CNPC_RollerMine_InputJoltVehicle = 0;
    pFourArgProt CSoundControllerImp_SoundChangeVolume = 0;
    pOneArgProt PrepForLevelTransition = 0;
    pTwoArgProt Restore = 0;
    pOneArgProtFastCall ReleaseManhack = 0;
    pThreeArgProt CombineBallGunDrop = 0;
    pVargArgProt ContentReset = 0;
    pTwoArgProt CombineAnimEvent = 0;
    pOneArgProtFastCall CAI_FollowBehavior_UpdateFollowPosition = 0;
    pTwoArgProt SaveRestoreFinish = 0;
    pOneArgProtFastCall KillSpritesManhack = 0;
} synergy_game_functions;

extern synergy_game_fields synergy_fields;
extern synergy_game_offsets synergy_offsets;
extern synergy_game_functions synergy_functions;

extern Library* synergy_srv;

extern bool sdktools_passed;

extern int save_frames;
extern int restore_delay_frames;
extern bool savegame;
extern bool savegame_autosave;
extern bool savegame_internal;
extern bool saved_game_once;
extern bool disable_player_restore;

extern ValueList save_player_vehicles_list;
extern ValueList save_map_vehicle_modelscale_list;

void ExtensionUpdateOnRemove(uint32_t arg0);
void HandleSpecificEntityRemoval(uint32_t object, bool validate, bool validate_player, bool slow, bool crash_server);
bool IsAllowedToPatchSdkTools(Library* lib);
uint32_t GetCBaseEntity(uint32_t EHandle);
void PopulateHookExclusionLists();
void InitCore();
bool IsTrackedVehicleClassname(const char* clsname);
void FixCars();
void SaveMapVehicleModelScales();
void RestoreMapVehicleModelScales();
uint32_t GetPassengerIndex(uint32_t player, uint32_t player_vehicle);
void MakePlayersLeaveVehicles();
void EnterVehicles(ValueList vehi_list);
void SaveGame_Extension();
void ReleaseLeakedPackedEntities(uint32_t snapManager);

#endif