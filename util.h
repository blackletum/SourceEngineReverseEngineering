#ifndef UTIL_H
#define UTIL_H

#include <math.h>

#define CLIENT_CHEATS_FRAME_LIMIT 10
#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"

typedef uint32_t (*pZeroArgProt)();
typedef uint32_t (*pOneArgProt)(uint32_t);
typedef uint32_t (*pTwoArgProt)(uint32_t, uint32_t);
typedef uint32_t (*pThreeArgProt)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFourArgProt)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFiveArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSixArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pNineArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pElevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

typedef uint32_t (__attribute__((regparm(2))) *pTwoArgProtRegParm)(uint32_t, uint32_t);
typedef uint32_t (__attribute__((fastcall)) *pOneArgProtFastCall)(uint32_t);
typedef uint32_t (__attribute__((thiscall)) *pOneArgProtThisCall)(uint32_t);
typedef uint32_t (__attribute__((fastcall)) *pTwoArgProtFastCall)(uint32_t, uint32_t);

typedef void (*Error)(char const *pMsg, ...);

class HooksUtil
{
public:
	static uint32_t EmptyCall();
	static uint32_t CallocHook(uint32_t nitems, uint32_t size);
	static uint32_t MallocHookSmall(uint32_t size);
	static uint32_t MallocHookLarge(uint32_t size);
	static uint32_t OperatorNewHook(uint32_t size);
	static uint32_t OperatorNewArrayHook(uint32_t size);
	static uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t UpdateOnRemove(uint32_t arg0);
	static uint32_t VPhysicsSetObjectHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CanSatisfyVpkCacheInternalHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6);
	static uint32_t PackedStoreDestructorHook(uint32_t arg0);
	static uint32_t SendNetMsgHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t VpkCacheBufferAllocHook(uint32_t arg0);
	static uint32_t SetOwnerEntityHook(uint32_t arg0, uint32_t arg1);
	static uint32_t DispatchAnimEventsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CalcAbsolutePositionHook(uint32_t arg0);
	static uint32_t VPhysicsUpdateHook(uint32_t arg0, uint32_t arg1);
	static uint32_t recheck_ov_element_hook(uint32_t arg0, uint32_t arg1);
	static uint32_t get_all_near_mindists_hook(uint32_t arg0);
	static uint32_t IVP_Real_Object_Destructor_Hook(uint32_t arg0);
	static uint32_t EngineErrorHook(char const *pMsg, ...);
	static uint32_t StrictEntityValidationSlow(uint32_t arg0);
	static uint32_t CEntityFactoryDictionary_CreateHook(uint32_t arg0, uint32_t arg1);
	static uint32_t PlayerSpawnHook(uint32_t arg0);
	static uint32_t SimulateEntitiesHook(uint8_t simulating);
	static uint32_t HookInstaKill(uint32_t arg0);
	static uint32_t UTIL_RemoveHookFailsafe(uint32_t arg0);
	static uint32_t UTIL_RemoveBaseHook(uint32_t arg0);
	static uint32_t GlobalEntityListClear(uint32_t arg0);
	static uint32_t host_changelevelhook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t AiSelectScheduleHook(uint32_t arg0);
	static uint32_t GetEnemyHook(uint32_t arg0);
	static uint32_t SetEnemyHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t LevelChangedSnapHook(uint32_t arg0);
};

typedef struct _game_fields {
	uint32_t gEntList = 0;
	uint32_t sv = 0;
	uint32_t sv_cheats_cvar = 0;
	uint32_t deferMindist = 0;
	uint32_t modelinfo = 0;
	uint32_t g_EventQueue = 0;
	uint32_t g_DeleteList = 0;
} game_fields;

typedef struct _game_offsets {
	uint32_t classname_offset = 0;
	uint32_t abs_origin_offset = 0;
	uint32_t origin_offset = 0;
	uint32_t abs_angles_offset = 0;
	uint32_t angles_offset = 0;
	uint32_t abs_velocity_offset = 0;
	uint32_t velocity_offset = 0;
	uint32_t refhandle_offset = 0;
	uint32_t iserver_offset = 0;
	uint32_t mnetwork_offset = 0;
	uint32_t collision_property_offset = 0;
	uint32_t m_CollisionGroup_offset = 0;
	uint32_t ismarked_offset = 0;
	uint32_t vphysics_object_offset = 0;
	uint32_t cvarstring_offset = 0;
	uint32_t isclientactive_offset = 0;
	uint32_t maxclients_offset = 0;
	uint32_t current_map_offset = 0;
	uint32_t getposition_vphysics_offset = 0;
	uint32_t setposition_vphysics_offset = 0;
	uint32_t cbaseclient_userid_offset = 0;
	uint32_t activeweapon_offset = 0;
	uint32_t targetent_offset = 0;
	uint32_t getcbasentity_offset = 0;
	uint32_t m_pGroup_offset = 0;
	uint32_t enemy_offset = 0;
} game_offsets;

typedef struct _game_functions {
	pOneArgProt SpawnPlayer = 0;
	pOneArgProt RemoveNormalDirect = 0;
	pOneArgProt RemoveNormal = 0;
	pOneArgProt RemoveInsta = 0;
	pTwoArgProt SetSolidFlags = 0;
	pTwoArgProt DisableEntityCollisions = 0;
	pTwoArgProt EnableEntityCollisions = 0;
	pThreeArgProt FindEntityByClassname = 0;
	pOneArgProt CleanupDeleteList = 0;
	pTwoArgProt CreateEntityByName = 0;
	pOneArgProt PhysSimEnt = 0;
	pSixArgProt AcceptInput = 0;
	pOneArgProt UpdateOnRemoveBase = 0;
	pOneArgProt VphysicsSetObject = 0;
	pOneArgProt ClearAllEntities = 0;
	pOneArgProt PackedStoreDestructor = 0;
	pSevenArgProt CanSatisfyVpkCacheInternal = 0;
	pTwoArgProt SV_ReplicateConVarChange = 0;
	pThreeArgProt SendNetMsg = 0;
	pFourArgProt ClientCommand = 0;
	pTwoArgProt PEntityOfEntIndex = 0;
	pTwoArgProt GetPlayerUserId = 0;
	pTwoArgProtFastCall InvokeMethodReverseOrderFastCall = 0;
	pTwoArgProtFastCall InvokePerFrameMethodFastCall = 0;
	pTwoArgProtRegParm InvokeMethodReverseOrderRegParm = 0;
	pTwoArgProtRegParm InvokePerFrameMethodRegParm = 0;
	pZeroArgProt PreSystemsThink = 0;
	pZeroArgProt PostSystemsThink = 0;
	pOneArgProt ServiceEvents = 0;
	pOneArgProt Physics_RunThinkFunctions = 0;
	pTwoArgProt SetOwnerEntity = 0;
	pTwoArgProt DispatchAnimEvents = 0;
	pOneArgProt CalcAbsolutePosition = 0;
	pTwoArgProt VPhysicsUpdate = 0;
	pFourArgProt CreateNoSpawn = 0;
	pThreeArgProt MapEntity_ParseAllEntities = 0;
	pOneArgProt DispatchSpawn = 0;
	pTwoArgProt recheck_ov_element = 0;
	pOneArgProt IVP_Real_Object_Destructor = 0;
	pThreeArgProt host_changelevel = 0;
	pTwoArgProt CEntityFactoryDictionary_Create = 0;
	pOneArgProt AiSelectSchedule = 0;
	pOneArgProt AiCleanupOnDeath = 0;
	pOneArgProt MakeDormant = 0;
	Error EngineError = 0;
	pOneArgProt LevelChangedSnap = 0;
	pTwoArgProt RemoveEntitySnapReference = 0;
	pTwoArgProt SetLocalOrigin = 0;
	pTwoArgProt SetAbsOrigin = 0;
	pOneArgProt GetEnemy = 0;
	pOneArgProt GetEnemy2 = 0;
	pThreeArgProt SetEnemy = 0;
} game_functions;

typedef struct _Signature {
	uint8_t signature[512];
	int signature_size;
} Signature;

typedef struct _Vector {
	float x = 0;
	float y = 0;
	float z = 0;
} Vector;

typedef struct _MemoryRegion {
	uint32_t start;
	uint32_t end;
	uint32_t protections;
	struct _MemoryRegion* nextRegion;
} MemoryRegion;

typedef struct _Library {
	void* library_linkmap;
	char* library_signature;
	MemoryRegion* region;
	uint32_t start_address;
	uint32_t end_address;
} Library;

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

typedef struct _EntityKV {
	uint32_t entityRef;
	uint32_t key;
	uint32_t value;
} EntityKV;

typedef struct _VpkMemoryLeak {
	uint32_t packed_ref;
	ValueList leaked_refs;
} VpkMemoryLeak;

typedef struct _EntityFrameCount {
	uint32_t entity_ref;
	int frames;
} EntityFrameCount;

typedef struct _EntityOrigin {
	uint32_t refHandle;
	float x;
	float y;
	float z;
} EntityOrigin;

extern void ExtensionUpdateOnRemove(uint32_t arg0);
extern void HandleSpecificEntityRemoval(uint32_t object, bool validate, bool validate_player, bool slow, bool crash_server);
extern uint32_t GetCBaseEntity(uint32_t EHandle);

extern game_fields fields;
extern game_offsets offsets;
extern game_functions functions;

extern float transitioning_player[3];
extern uint32_t transitioned_clients[512];

extern bool loaded_extension;

extern bool firstplayer_hasjoined;
extern bool player_collision_rules_changed;
extern bool player_worldspawn_collision_disabled;

extern int min_collision_frames;

extern uint32_t hook_exclude_list_offset[512];
extern uint32_t hook_exclude_list_base[512];
extern uint32_t our_libraries[512];
extern uint32_t loaded_libraries[512];

extern Library* engine_srv;
extern Library* dedicated_srv;
extern Library* vphysics_srv;
extern Library* server_srv;
extern Library* server;
extern Library* sdktools;

extern bool isTicking;
extern bool server_sleeping;
extern uint32_t global_vpk_cache_buffer;
extern uint32_t current_vpk_buffer_ref;
extern ValueList leakedResourcesVpkSystem;
extern ValueList players_connect_commands_list;

uint32_t FindEntityByIVP(uint32_t ivp_real_object, const char* search_classname);
void UpdateCollisionByIVP(uint32_t ivp_real_object);
void CorrectPhysics();
void DeinitUtil();
void InitUtil();
void* copy_val(void* val, size_t copy_size);
bool IsAddressExcluded(uint32_t base_address, uint32_t search_address);
void HookFunction(Library* binary, void* target_pointer, void* hook_pointer);
void HookMemoryBlock(uint32_t base_address, uint32_t size, Signature start_signatures[32], int start_signatures_size, Signature end_signatures[32], int end_signatures_size, Signature args_signatures[32], Signature stack_machine_code[32], int stack_arguments_size, Signature no_operation_signatures[32], int no_operation_signatures_size, uint32_t estimated_block_size_min, uint32_t estimated_block_size_max, int expected_arguments, void* hook_pointer);
bool ApplyBlockHook(uint32_t block_start_start, uint32_t block_end_end, Signature args_signatures[32], Signature stack_machine_code[32], int stack_arguments_size, Signature no_operation_signatures[32], int no_operation_signatures_size, int expected_arguments, void* hook_pointer);
uint32_t ApplyNoOperation(uint32_t block_start_start, uint32_t block_end_end, Signature no_operation_signatures[32], int no_operation_signatures_size);
uint32_t FindSignature(uint32_t block_start_start, uint32_t block_end_end, Signature signature_main);
Library* FindLibrary(char* lib_name, bool less_intense_search);
Library* LoadLibrary(char* library_full_path);
void ClearLoadedLibraries();
char* getlibrary(char* file_line);
bool IsOurLibraryPath(char* abs_path);
void AllowWriteToMappedMemory();
void ForceMemoryAccess();
void RestoreMemoryProtections();
void ZeroVector(uint32_t vector);
bool IsVectorNaN(uint32_t base);
bool IsVectorInf(uint32_t base);
void UpdateCollisions(bool flush);
void UpdateOtherCollisions();
void UpdatePlayerCollisions();
void UpdateAllCollisions(bool cleanup);
void RemoveBadEnts();
bool IsMarkedForDeletion(uint32_t arg0);
bool IsEntityPositionReasonable(uint32_t v);
uint32_t IsEntityValid(uint32_t entity);
void LogVpkMemoryLeaks();
void FixPlayerCollisionGroup();
void HookFunctionsUtil();
void ReplicateCheatsOnClient();
void CorrectCheats();
void SetServerSleepStatus();
void SendClientConnectCommands(bool increment_frames, bool send_commands);
void SendClientCommands(uint32_t player_edict);
int GetEarliestClients();
bool VerifyEntity(uint32_t entity_object, bool validate, bool validate_player);
void CorrectVphysicsEntity(uint32_t ent);
bool IsVphysicsEntityBad(uint32_t ent);
void UpdateEntityPosition(uint32_t object, float x, float y, float z);
bool IsAllowedToFakeUserId(int userid_input);
void ZeroArrayList(uint32_t* list);
void InsertToArrayList(uint32_t* list, uint32_t value);
bool IsValueInArrayList(uint32_t* list, uint32_t value);
void TeleportPlayersToTransition();
void RemoveHl2Ragdolls();
int ReleaseLeakedMemory(ValueList leakList, bool destroy);

ValueList AllocateValuesList();
Value* CreateNewValue(void* valueInput);
int DeleteAllValuesInList(ValueList list, bool free_val, pthread_mutex_t* lockInput);
bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
int ValueListItems(ValueList list, pthread_mutex_t* lockInput);
bool InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* lockInput, bool tail, bool duplicate_chk);
Value* FindStringInList(ValueList list, const char* search_val, pthread_mutex_t* lockInput, bool substring, Value* start_value);
EntityKV* CreateNewEntityKV(uint32_t refHandle, uint32_t keyIn, uint32_t valueIn);
void DisablePlayerCollisions();
void DisablePlayerWorldSpawnCollision();
bool FixSlashes(char* string);
bool IsValidVector(uint32_t base);

#endif