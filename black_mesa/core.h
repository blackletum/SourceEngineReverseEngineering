#ifndef CORE_H
#define CORE_H

#define EXT_PREFIX "[BlackMesaUtils] "

typedef struct _black_mesa_game_fields {

} black_mesa_game_fields;

typedef struct _black_mesa_game_offsets {

} black_mesa_game_offsets;

typedef struct _black_mesa_game_functions {
    pThreeArgProt RagdollBreak;
    pOneArgProt CXenShieldController_UpdateOnRemove;
    pOneArgProt UTIL_GetLocalPlayer;
    pSevenArgProt TestGroundMove;
    pThreeArgProt ShouldHitEntity;
    pOneArgProt LaunchMortar;
    pTwoArgProt InputSetCSMVolume;
    pTwoArgProt InputApplySettings;
    pOneArgProt CNihiBallzDestructor;
    pTwoArgProt EnumElement;
    pTwoArgProt TakeDamage;
    pTwoArgProt CPropHevCharger_ShouldApplyEffect;
    pTwoArgProt CPropRadiationCharger_ShouldApplyEffect;
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