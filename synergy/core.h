#ifndef CORE_H
#define CORE_H

#define EXT_PREFIX "[SynergyUtils] "

typedef struct _synergy_game_fields {
    //Synergy fields
    uint32_t m_sbStaticPoseParamsLoadedDropship;
} synergy_game_fields;

typedef struct _synergy_game_offsets {
    //Synergy offsets
    uint32_t vehicle_model_offset;
    uint32_t vehicle_script_offset;
    uint32_t iserver_vehicle_offset;
    uint32_t base_vehicle_offset;
    uint32_t getpassengercount_offset;
    uint32_t player_vehicle_offset;
    uint32_t leavevehicle_offset;
    uint32_t entervehicle_offset;
    uint32_t dropship_container_offset;
} synergy_game_offsets;

typedef struct _synergy_game_functions {
    //Synergy functions
    pOneArgProt CombineDropshipSpawn;
    pFourArgProt SaveGameState;
    pTwoArgProt RestorePlayer;
    pOneArgProtFastCall Autosave_Silent;
    pThreeArgProt LookupPoseParameterDropship;
    pOneArgProt PopulatePoseParametersDropship;
    pTwoArgProt CAI_PassengerBehavior_ReserveEntryPoint;
    pTwoArgProt CAI_PassengerBehaviorCompanion_FindEntrySequence;
    pThreeArgProt CAI_PassengerBehavior_GetEntryTarget;
    pOneArgProt CAI_PassengerBehaviorCompanion_GatherVehicleStateConditions;
    pTwoArgProt UTIL_GetPlayerMP;
    pOneArgProt CNPC_RollerMine_InputJoltVehicle;
    pFourArgProt CSoundControllerImp_SoundChangeVolume;
    pOneArgProt PrepForLevelTransition;
} synergy_game_functions;

extern synergy_game_fields synergy_fields;
extern synergy_game_offsets synergy_offsets;
extern synergy_game_functions synergy_functions;

extern uint32_t synergy_srv;
extern uint32_t synergy_srv_size;

extern bool sdktools_passed;

extern int save_frames;
extern int savegame_delayed;
extern bool savegame;
extern bool savegame_autosave;
extern bool savegame_internal;

extern ValueList save_player_vehicles_list;

void ExtensionUpdateOnRemove(uint32_t arg0);
void HandleSpecificEntityRemoval(uint32_t object, bool validate, bool validate_player, bool slow, bool crash_server);
bool IsAllowedToPatchSdkTools(uint32_t lib_base, uint32_t lib_size);
uint32_t GetCBaseEntity(uint32_t EHandle);
void PopulateHookExclusionLists();
int ReleaseLeakedMemory(ValueList leakList, bool destroy, uint32_t current_cap, uint32_t allowed_cap, uint32_t free_perc);
void InitCore();
void FixCars();
uint32_t GetPassengerIndex(uint32_t player, uint32_t player_vehicle);
void MakePlayersLeaveVehicles();
void EnterVehicles(ValueList vehi_list);

#endif