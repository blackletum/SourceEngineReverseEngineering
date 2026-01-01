#ifndef CORE_H
#define CORE_H

#define EXT_PREFIX "[BlackMesaUtils] "

typedef struct _black_mesa_game_fields {

} black_mesa_game_fields;

typedef struct _black_mesa_game_offsets {

} black_mesa_game_offsets;

typedef struct _black_mesa_game_functions {
    pThreeArgProt RagdollBreak = 0;
    pOneArgProt CXenShieldController_UpdateOnRemove = 0;
    pOneArgProt UTIL_GetLocalPlayer = 0;
    pSevenArgProt TestGroundMove = 0;
    pThreeArgProt ShouldHitEntity = 0;
    pOneArgProt LaunchMortar = 0;
    pTwoArgProt InputSetCSMVolume = 0;
    pTwoArgProt InputApplySettings = 0;
    pOneArgProt CNihiBallzDestructor = 0;
    pTwoArgProt EnumElement = 0;
    pTwoArgProt TakeDamage = 0;
    pTwoArgProt CPropHevCharger_ShouldApplyEffect = 0;
    pTwoArgProt CPropRadiationCharger_ShouldApplyEffect = 0;
} black_mesa_game_functions;

extern black_mesa_game_fields black_mesa_fields;
extern black_mesa_game_offsets black_mesa_offsets;
extern black_mesa_game_functions black_mesa_functions;

extern uint32_t last_ragdoll_gib;
extern int ragdoll_breaking_gib_counter;
extern bool is_currently_ragdoll_breaking;

void InitCore();
void PopulateHookExclusionLists();
uint32_t GetCBaseEntity(uint32_t EHandle);

void CheckForLocation();
void ExtensionUpdateOnRemove(uint32_t arg0);
void HandleSpecificEntityRemoval(uint32_t object, bool validate, bool validate_player, bool slow, bool crash_server);

#endif