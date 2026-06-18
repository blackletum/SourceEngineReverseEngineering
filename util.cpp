#include "extension.h"
#include "util.h"

#include <link.h>
#include <sys/mman.h>
#include <cstdio>

game_fields fields;
game_offsets offsets;
game_functions functions;

float transitioning_player[3] = {};
uint32_t transitioned_clients[512] = {};

bool loaded_extension;
bool faking_cheats;
bool firstplayer_hasjoined;
bool player_collision_rules_changed;
bool player_worldspawn_collision_disabled;
bool replicating_client_cheats;

int connected_clients;
int incorrect_cheats_frames;
int correct_cheats_frames;

uint32_t hook_exclude_list_offset[512] = {};
uint32_t hook_exclude_list_base[512] = {};
uint32_t our_libraries[512] = {};
uint32_t loaded_libraries[512] = {};

Library* engine_srv;
Library* dedicated_srv;
Library* vphysics_srv;
Library* server_srv;
Library* server;
Library* sdktools;

bool isTicking;
bool server_sleeping;

uint32_t global_vpk_cache_buffer;
uint32_t current_vpk_buffer_ref;

ValueList leakedResourcesVpkSystem;
ValueList players_connect_commands_list;
ValueList ivp_list;
ValueList hook_function_patch_notes;

void ConsolePrint(const char *pMsg, ...)
{
    char buffer[2048];
    
    va_list marker;
    va_start(marker, pMsg);
    vsnprintf(buffer, sizeof(buffer), pMsg, marker);
    va_end(marker);
    
    rootconsole->ConsolePrint("%s", buffer);
}

void InitUtil()
{
    loaded_extension = false;
    faking_cheats = false;
    firstplayer_hasjoined = false;
    player_collision_rules_changed = false;
    player_worldspawn_collision_disabled = false;
    isTicking = false;
    server_sleeping = false;
    global_vpk_cache_buffer = (uint32_t)malloc(0x00100000*2);
    current_vpk_buffer_ref = 0;
    incorrect_cheats_frames = 0;
    correct_cheats_frames = 0;
    connected_clients = 0;
    replicating_client_cheats = false;
    players_connect_commands_list = AllocateValuesList();
    leakedResourcesVpkSystem = AllocateValuesList();
    ivp_list = AllocateValuesList();
    hook_function_patch_notes = AllocateValuesList();

    HookFunctionsUtil();
}

void HookFunctionsUtil()
{
    HookFunction(vphysics_srv, (void*)functions.recheck_ov_element, (void*)HooksUtil::recheck_ov_element_hook);
    HookFunction(vphysics_srv, (void*)functions.IVP_Real_Object_Destructor, (void*)HooksUtil::IVP_Real_Object_Destructor_Hook);

    HookFunction(engine_srv, (void*)functions.SendNetMsg, (void*)HooksUtil::SendNetMsgHook);

    HookFunction(server_srv, (void*)functions.PhysSimEnt, (void*)HooksUtil::PhysSimEnt);
    HookFunction(server_srv, (void*)functions.AcceptInput, (void*)HooksUtil::AcceptInputHook);
    HookFunction(server_srv, (void*)functions.UpdateOnRemoveBase, (void*)HooksUtil::UpdateOnRemove);
    HookFunction(server_srv, (void*)functions.VphysicsSetObject, (void*)HooksUtil::VPhysicsSetObjectHook);
    HookFunction(server_srv, (void*)functions.SetOwnerEntity, (void*)HooksUtil::SetOwnerEntityHook);
    HookFunction(server_srv, (void*)functions.DispatchAnimEvents, (void*)HooksUtil::DispatchAnimEventsHook);
    HookFunction(server_srv, (void*)functions.CalcAbsolutePosition, (void*)HooksUtil::CalcAbsolutePositionHook);
    HookFunction(server_srv, (void*)functions.VPhysicsUpdate, (void*)HooksUtil::VPhysicsUpdateHook);
    HookFunction(server_srv, (void*)functions.AiSelectSchedule, (void*)HooksUtil::AiSelectScheduleHook);
    HookFunction(server_srv, (void*)functions.MakeDormant, (void*)HooksUtil::EmptyCall);
    HookFunction(server_srv, (void*)functions.GetEnemy, (void*)HooksUtil::GetEnemyHook);
    HookFunction(server_srv, (void*)functions.GetEnemy2, (void*)HooksUtil::GetEnemyHook);
    HookFunction(server_srv, (void*)functions.SetEnemy, (void*)HooksUtil::SetEnemyHook);
    HookFunction(server_srv, (void*)functions.EngineError, (void*)HooksUtil::EngineErrorHook);
    HookFunction(server_srv, (void*)functions.FindPickerEntity, (void*)HooksUtil::FindPickerEntityHook);

    //HookFunction(server_srv, (void*)malloc, (void*)HooksUtil::MallocHookSmall);

    HookFunction(dedicated_srv, (void*)functions.PackedStoreDestructor, (void*)HooksUtil::PackedStoreDestructorHook);
    HookFunction(dedicated_srv, (void*)functions.CanSatisfyVpkCacheInternal, (void*)HooksUtil::CanSatisfyVpkCacheInternalHook);
    //HookFunction(dedicated_srv, (void*)malloc, (void*)HooksUtil::MallocHookLarge);
}

uint32_t HooksUtil::FindPickerEntityHook(uint32_t arg0)
{
    if(arg0)
    {
        return functions.FindPickerEntity(arg0);
    }

    ConsolePrint("FindPickerEntity failed!");
    return 0;
}

void UpdateEntityPosition(uint32_t object, float x, float y, float z)
{
    pTwoArgProt pDynamicTwoArgFunc;
    pFourArgProt pDynamicFourArgFunc;

    if(IsEntityValid(object))
    {
        Vector empty_vector;
        Vector new_position;

        new_position.x = x;
        new_position.y = y;
        new_position.z = z + 8.0f;

        //ConsolePrint("updated abs pos to %f %f %f", new_position.x, new_position.y, new_position.z);

        float* origin = (float*)(object+offsets.origin_offset);
        memcpy(origin, &new_position, sizeof(float) * 3);

        pDynamicTwoArgFunc = (pTwoArgProt)(functions.SetLocalOrigin);
        pDynamicTwoArgFunc(object, (uint32_t)&new_position);

        pDynamicTwoArgFunc = (pTwoArgProt)(functions.SetAbsOrigin);
        pDynamicTwoArgFunc(object, (uint32_t)&new_position);
    }
}

int GetEarliestClients()
{
    int maxclients = *(int*)(fields.sv+offsets.maxclients_offset);
    int earliest_clients = 0;

    for(int i = 1; i <= maxclients; i++)
    {
        uint32_t player_edict = functions.PEntityOfEntIndex(0, i);

        if(player_edict)
        {
            int userid = functions.GetPlayerUserId(0, player_edict);
            if(userid != -1 && userid != 0) earliest_clients++;
        }
    }

    return earliest_clients;
}

void SendClientConnectCommands(bool increment_frames, bool send_commands)
{
    int maxclients = *(int*)(fields.sv+offsets.maxclients_offset);
    int earliest_clients = GetEarliestClients();

    for(int i = 1; i <= maxclients; i++)
    {
        uint32_t player_edict = functions.PEntityOfEntIndex(0, i);

        if(player_edict)
        {
            int userid = functions.GetPlayerUserId(0, player_edict);
            //ConsolePrint("USERID: [%d]", userid);

            if(userid != -1 && userid != 0)
            {
                bool found_player = false;
                Value* first_connect_player = *players_connect_commands_list;
    
                while(first_connect_player && first_connect_player->nextVal)
                {
                    int player_index = (int)first_connect_player->value;
                    int frames = (int)first_connect_player->nextVal->value;
    
                    if(player_index == i)
                    {
                        if(earliest_clients != connected_clients)
                        {
                            //if(!(!increment_frames && !send_commands))
                                //ConsolePrint("Clients are not ready to send commands yet %d %d", earliest_clients, connected_clients);
                            found_player = true;
                            break;
                        }
                        
                        if(frames < 1)
                        {
                            faking_cheats = true;
                            ConsolePrint("cheats faked! set to true");
                        }
                        
                        if(frames == 1 && send_commands)
                        {
                            SendClientCommands(player_edict);
                            ConsolePrint("client commands sent");
                        }
    
                        if(frames > 1000) frames = 1000;
                        if(increment_frames) first_connect_player->nextVal->value = (void*)(frames+1);

                        found_player = true;
                        break;
                    }
    
                    first_connect_player = first_connect_player->nextVal->nextVal;
                }
    
                if(found_player) continue;
    
                Value* new_player_index = CreateNewValue((void*)i);
                Value* start_frames = CreateNewValue((void*)0);
    
                InsertToValuesList(players_connect_commands_list, new_player_index, NULL, true, false);
                InsertToValuesList(players_connect_commands_list, start_frames, NULL, true, false);
            }
            else
            {
                ValueList new_player_connects_list = AllocateValuesList();
                Value* first_connect_player = *players_connect_commands_list;
    
                while(first_connect_player && first_connect_player->nextVal)
                {
                    int player_index = (int)first_connect_player->value;
                    int frames = (int)first_connect_player->nextVal->value;
    
                    if(player_index != i)
                    {
                        Value* new_player_index = CreateNewValue((void*)player_index);
                        Value* start_frames = CreateNewValue((void*)frames);
    
                        InsertToValuesList(new_player_connects_list, new_player_index, NULL, true, false);
                        InsertToValuesList(new_player_connects_list, start_frames, NULL, true, false);
                    }
                    else
                    {
                        ConsolePrint("Removed dead player!");
                    }
    
                    Value* nextValue = first_connect_player->nextVal->nextVal;
    
                    free(first_connect_player->nextVal);
                    free(first_connect_player);
    
                    first_connect_player = nextValue;
                }
    
                free(players_connect_commands_list);
                players_connect_commands_list = new_player_connects_list;
            }
        }
    }
}

void SendClientCommands(uint32_t player_edict)
{
    functions.ClientCommand(0, player_edict, (uint32_t)"reload_particleseffects_client", 0);
}

bool FixSlashes(char* string)
{
    bool fixed_name = false;

    if(string == NULL)
        return fixed_name;

    for(int i = 0; i <= (int)strlen((char*)string); i++)
    {
        char byte = (char)(*(uint8_t*)(string+i));

        if(byte == '\\')
        {
            *(uint8_t*)(string+i) = (uint8_t)'/';
            fixed_name = true;
        }
    }

    return fixed_name;
}

void CorrectCheats()
{
    char* sv_cheats_value = (char*)(*(uint32_t*)(fields.sv_cheats_cvar+offsets.cvarstring_offset));

    if(strcmp(sv_cheats_value, "1") == 0)
    {
        functions.SV_ReplicateConVarChange(fields.sv_cheats_cvar, (uint32_t)"1");
    }
    else
    {
        functions.SV_ReplicateConVarChange(fields.sv_cheats_cvar, (uint32_t)"0");
    }
}

bool IsAllowedToFakeUserId(int userid_input)
{
    Value* first_connect_player = *players_connect_commands_list;

    while(first_connect_player)
    {
        int player_index = (int)first_connect_player->value;
        int frames = (int)first_connect_player->nextVal->value;

        if(frames == 1)
        {
            uint32_t player_edict = functions.PEntityOfEntIndex(0, player_index);

            if(player_edict)
            {
                int userid = functions.GetPlayerUserId(0, player_edict);
                if(userid == userid_input) return true;
            }
        }

        first_connect_player = first_connect_player->nextVal->nextVal;
    }

    return false;
}

void ReplicateCheatsOnClient()
{
    if(incorrect_cheats_frames > 10000) incorrect_cheats_frames = 10000;
    if(correct_cheats_frames > 10000) correct_cheats_frames = 10000;

    int connected_clients_frame = 0;
    
    faking_cheats = false;
    connected_clients = 0;

    //UPDATE LIST
    SendClientConnectCommands(false, false);

    //COUNT THE AMOUNT OF READY CLIENTS
    replicating_client_cheats = true;
    functions.SV_ReplicateConVarChange(fields.sv_cheats_cvar, (uint32_t)"1");
    replicating_client_cheats = false;

    connected_clients_frame = connected_clients;

    //TELL SYSTEM THAT WE CAN SEND COMMANDS NOT YET AS WE HAVE NOT SENT THE FAKE PACKETS
    SendClientConnectCommands(true, false);

    //SEND THE FAKE PACKETS
    replicating_client_cheats = true;
    functions.SV_ReplicateConVarChange(fields.sv_cheats_cvar, (uint32_t)"1");
    replicating_client_cheats = false;

    connected_clients = connected_clients_frame;

    //SEND THE CLIENT COMMANDS NOW
    SendClientConnectCommands(false, true);

    if(!faking_cheats)
    {
        if(correct_cheats_frames <= 50) CorrectCheats();
        if(correct_cheats_frames == 0) ConsolePrint("Corrected cheats! [%d]", incorrect_cheats_frames);

        incorrect_cheats_frames = 0;
        correct_cheats_frames++;
    }
    else
    {
        if(incorrect_cheats_frames >= CLIENT_CHEATS_FRAME_LIMIT && incorrect_cheats_frames <= CLIENT_CHEATS_FRAME_LIMIT+50)
        {
            ConsolePrint("Over the limit!");
            CorrectCheats();
        }
        
        correct_cheats_frames = 0;
        incorrect_cheats_frames++;
    }
}

bool IsVphysicsEntityBad(uint32_t ent)
{
    pThreeArgProt pDynamicThreeArgFunc;
    pFourArgProt pDynamicFourArgFunc;

    if(IsEntityValid(ent))
    {
        uint32_t vphysics_object = *(uint32_t*)(ent+offsets.vphysics_object_offset);

        if(vphysics_object)
        {
            Vector current_origin;
            Vector current_angles;

            //GetPosition
            pDynamicThreeArgFunc = (pThreeArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+offsets.getposition_vphysics_offset)  );
            pDynamicThreeArgFunc(vphysics_object, (uint32_t)&current_origin, (uint32_t)&current_angles);

            //ConsolePrint("%f %f %f", current_angles.x, current_angles.y, current_angles.z);

            if(!IsEntityPositionReasonable((uint32_t)&current_origin))
            {
                return true;
            }
        }
    }

    return false;
}

void CorrectVphysicsEntity(uint32_t ent)
{
    pThreeArgProt pDynamicThreeArgFunc;
    pFourArgProt pDynamicFourArgFunc;

    if(IsEntityValid(ent))
    {
        uint32_t vphysics_object = *(uint32_t*)(ent+offsets.vphysics_object_offset);

        if(vphysics_object)
        {
            Vector current_origin;
            Vector current_angles;
            Vector empty_vector;

            bool bad_origin = false;
            bool bad_angles = false;

            //GetPosition
            pDynamicThreeArgFunc = (pThreeArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+offsets.getposition_vphysics_offset)  );
            pDynamicThreeArgFunc(vphysics_object, (uint32_t)&current_origin, (uint32_t)&current_angles);

            //ConsolePrint("%f %f %f", current_angles.x, current_angles.y, current_angles.z);

            if(!IsEntityPositionReasonable((uint32_t)&current_origin))
            {
                bad_origin = true;
                ConsolePrint("Corrected vphysics origin!");
            }

            if(!IsEntityPositionReasonable((uint32_t)&current_angles))
            {
                bad_angles = true;
                ConsolePrint("Corrected vphysics angles!");
            }

            if(bad_origin && bad_angles)
            {
                ZeroVector(ent+offsets.origin_offset);
                ZeroVector(ent+offsets.abs_origin_offset);

                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+offsets.setposition_vphysics_offset)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&empty_vector, (uint32_t)&empty_vector, 1);
            }
            else if(bad_origin)
            {
                ZeroVector(ent+offsets.origin_offset);
                ZeroVector(ent+offsets.abs_origin_offset);

                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+offsets.setposition_vphysics_offset)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&empty_vector, (uint32_t)&current_angles, 1);
            }
            else if(bad_angles)
            {
                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+offsets.setposition_vphysics_offset)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&current_origin, (uint32_t)&empty_vector, 1); 
            }
        }
    }
}

uint32_t HooksUtil::CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);

    uint32_t enlarged_size = nitems*2.5;
    return (uint32_t)calloc(enlarged_size, size);
}

uint32_t HooksUtil::MallocHookSmall(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    
    return (uint32_t)malloc(size*1.3);
}

uint32_t HooksUtil::MallocHookLarge(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);

    return (uint32_t)malloc(size*3.0);
}

uint32_t HooksUtil::OperatorNewHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)operator new(size);

    return (uint32_t)operator new(size*1.4);
}

uint32_t HooksUtil::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)operator new[](size);

    return (uint32_t)operator new[](size*3.0);
}

uint32_t HooksUtil::ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);

    return (uint32_t)realloc((void*)old_ptr, new_size*1.2);
}

uint32_t HooksUtil::EmptyCall()
{
    return 0;
}

uint32_t HooksUtil::AiSelectScheduleHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    uint32_t m_pGroup = *(uint32_t*)(arg0+offsets.m_pGroup_offset);

    if(m_pGroup)
    {   
        uint32_t entity_chk_ref = *(uint32_t*)(m_pGroup+4);
        uint32_t object = GetCBaseEntity(entity_chk_ref);

        if(!IsEntityValid(object))
        {
            ConsolePrint("\nGame engine failed to cleanup death!\n");

            pDynamicOneArgFunc = (pOneArgProt)(functions.AiCleanupOnDeath);
            pDynamicOneArgFunc(arg0);
        }
    }

    pDynamicOneArgFunc = (pOneArgProt)(functions.AiSelectSchedule);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksUtil::StrictEntityValidationSlow(uint32_t arg0)
{
    HandleSpecificEntityRemoval(arg0, true, false, true, false);
    return 0;
}

uint32_t HooksUtil::EngineErrorHook(char const *pMsg, ...)
{
    va_list marker;
    va_start(marker, pMsg);
    vprintf( pMsg, marker );
    //ConsolePrint(pMsg, marker);
    va_end(marker);

    return 0;
}

uint32_t HooksUtil::IVP_Real_Object_Destructor_Hook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t entity = FindEntityByIVP(arg0, NULL);

    if(entity)
    {
        ConsolePrint("Removed entity because REAL OBJECT WAS REMOVED!");
        HandleSpecificEntityRemoval(entity, true, true, true, true);
    }

    pDynamicOneArgFunc = (pOneArgProt)(functions.IVP_Real_Object_Destructor);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    RemoveFromValuesList(ivp_list, (void*)arg0, NULL);
    
    return returnVal;
}

uint32_t HooksUtil::recheck_ov_element_hook(uint32_t arg0, uint32_t arg1)
{
    Value* ivp_real_object = CreateNewValue((void*)arg1);
    InsertToValuesList(ivp_list, ivp_real_object, NULL, true, true);

    return 0;
}

uint32_t HooksUtil::SendNetMsgHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    if(replicating_client_cheats)
    {
        int current_userid = *(uint32_t*)(arg0+offsets.cbaseclient_userid_offset);
        
        connected_clients++;

        if(faking_cheats)
        {
            if(incorrect_cheats_frames >= CLIENT_CHEATS_FRAME_LIMIT)
            {
                ConsolePrint("Blocked!");
                return 0;
            }
        }
        else
        {
            return 0;
        }

        if(IsAllowedToFakeUserId(current_userid) == false)
        {
            ConsolePrint("Blocked userid [%d]", current_userid);
            return 0;
        }

        ConsolePrint("Cheats have been faked!");
    }

    return functions.SendNetMsg(arg0, arg1, arg2);
}

uint32_t HooksUtil::CanSatisfyVpkCacheInternalHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    uint32_t returnVal = functions.CanSatisfyVpkCacheInternal(arg0, arg1, arg2, arg3, arg4, arg5, arg6);

    if(current_vpk_buffer_ref)
    {
        uint32_t allocated_vpk_buffer = *(uint32_t*)(current_vpk_buffer_ref+0x10);

        if(allocated_vpk_buffer && global_vpk_cache_buffer == allocated_vpk_buffer)
        {
            //ConsolePrint("Removed global vpk buffer from VPK tree!");
            *(uint32_t*)(current_vpk_buffer_ref+0x10) = 0;
        }
        else
        {
            ConsolePrint("Failed to remove global vpk buffer!!!");
            exit(1);
        }

        current_vpk_buffer_ref = 0;
    }

    return returnVal;
}

uint32_t HooksUtil::VpkCacheBufferAllocHook(uint32_t mutex, uint32_t buffer)
{
    uint32_t packed_store_ref = mutex-0x228;
    uint32_t vpk_buffer = *(uint32_t*)(buffer+0x10);

    if(vpk_buffer == 0)
    {
        current_vpk_buffer_ref = buffer;
        return global_vpk_cache_buffer;
    }

    bool saved_reference = false;

    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == packed_store_ref)
        {
            saved_reference = true;

            ValueList vpk_leak_list = the_leak->leaked_refs;

            Value* new_vpk_leak = CreateNewValue((void*)(vpk_buffer));
            bool added = InsertToValuesList(vpk_leak_list, new_vpk_leak, NULL, false, true);

            if(added)
            {
                ConsolePrint("[VPK Hook] " HOOK_MSG, vpk_buffer);
            }

            break;
        }

        a_leak = a_leak->nextVal;
    }

    if(!saved_reference)
    {
        VpkMemoryLeak* omg_leaks = (VpkMemoryLeak*)(malloc(sizeof(VpkMemoryLeak)));
        ValueList empty_list = AllocateValuesList();

        Value* original_vpk_buffer = CreateNewValue((void*)vpk_buffer);
        InsertToValuesList(empty_list, original_vpk_buffer, NULL, false, false);

        omg_leaks->packed_ref = packed_store_ref;
        omg_leaks->leaked_refs = empty_list;

        Value* leaked_resource = CreateNewValue((void*)omg_leaks);
        InsertToValuesList(leakedResourcesVpkSystem, leaked_resource, NULL, false, false);

        ConsolePrint("[VPK Hook First] " HOOK_MSG, vpk_buffer);
    }

    return vpk_buffer;
}

uint32_t HooksUtil::PackedStoreDestructorHook(uint32_t arg0)
{
    //Remove ref to store only valid objects!
    uint32_t returnVal = functions.PackedStoreDestructor(arg0);

    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == arg0)
        {
            ValueList vpk_leak_list = the_leak->leaked_refs;
            
            int removed_items = DeleteAllValuesInList(vpk_leak_list, true, NULL);

            ConsolePrint("[VPK Hook] released [%d] memory leaks!", removed_items);

            bool success = RemoveFromValuesList(leakedResourcesVpkSystem, the_leak, NULL);

            free(vpk_leak_list);
            free(the_leak);

            if(!success)
            {
                ConsolePrint("[VPK Hook] Expected to remove leak but failed!");
                exit(EXIT_FAILURE);
            }

            return returnVal;
        }

        a_leak = a_leak->nextVal;
    }

    return returnVal;
}

uint32_t HooksUtil::SetOwnerEntityHook(uint32_t arg0, uint32_t arg1)
{
    if(arg1 != 0 && !IsEntityValid(arg1))
    {
        ConsolePrint("Invalid entity in SetOwnerEntity! replaced with worldspawn");
        return functions.SetOwnerEntity(arg0, functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"worldspawn"));
    }

    return functions.SetOwnerEntity(arg0, arg1);
}

uint32_t HooksUtil::DispatchAnimEventsHook(uint32_t arg0, uint32_t arg1)
{
    if(IsEntityValid(arg0) && IsEntityValid(arg1))
    {
        return functions.DispatchAnimEvents(arg0, arg1);
    }

    ConsolePrint("Failed to service DispatchAnimEvents");
    return 0;
}

uint32_t HooksUtil::CalcAbsolutePositionHook(uint32_t arg0)
{
    if(IsEntityValid(arg0))
    {
        return functions.CalcAbsolutePosition(arg0);
    }

    //ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t HooksUtil::VPhysicsUpdateHook(uint32_t arg0, uint32_t arg1)
{
    if(IsVphysicsEntityBad(arg0))
    {
        ConsolePrint("Removed BAD physics entity!");
        HandleSpecificEntityRemoval(arg0, true, true, true, true);
    }

    return functions.VPhysicsUpdate(arg0, arg1);
}

uint32_t HooksUtil::UpdateOnRemove(uint32_t arg0)
{
    //FINAL CHECKS

    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(VerifyEntity(arg0, true, true) == false)
    {
        uint32_t first_return = ((uint32_t)__builtin_return_address(0)) - server_srv->start_address;
        uint32_t second_return = ((uint32_t)__builtin_return_address(1)) - server_srv->start_address;
        uint32_t third_return = ((uint32_t)__builtin_return_address(2)) - server_srv->start_address;
        uint32_t fourth_return = ((uint32_t)__builtin_return_address(3)) - server_srv->start_address;
    
        ConsolePrint("UpdateOnRemove: Failed to validate entity 1:%p 2:%p 3:%p 4:%p", first_return, second_return, third_return, fourth_return);
        exit(EXIT_FAILURE);
    }

    ExtensionUpdateOnRemove(arg0);
    return functions.UpdateOnRemoveBase(arg0);
}

uint32_t HooksUtil::PhysSimEnt(uint32_t arg0)
{
    char* clsname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(IsMarkedForDeletion(arg0+offsets.iserver_offset))
    {
        ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }

    return functions.PhysSimEnt(arg0);
}

uint32_t HooksUtil::VPhysicsSetObjectHook(uint32_t arg0, uint32_t arg1)
{
    if(IsEntityValid(arg0))
    {
        uint32_t vphysics_object = *(uint32_t*)(arg0+offsets.vphysics_object_offset);

        if(vphysics_object)
        {
            ConsolePrint("Attempting override existing vphysics object!!!!");
            return 0;
        }

        *(uint32_t*)(arg0+offsets.vphysics_object_offset) = arg1;

        return 0;
    }

    ConsolePrint("Entity was invalid failed to set vphysics object!");
    return 0;
}

uint32_t HooksUtil::AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    // CBaseEntity arg0 arg2 arg3

    bool failure = false;

    if(IsEntityValid(arg0) == 0) failure = true;

    if(failure)
    {
        //ConsolePrint("AcceptInput blocked on marked entity");
        return 0;
    }

    //Passed sanity check
    return functions.AcceptInput(arg0, arg1, arg2, arg3, arg4, arg5);
}

void UpdateCollisionByIVP(uint32_t ivp_real_object)
{
    uint32_t manager = *(uint32_t*)((*(uint32_t*)((*(uint32_t*)(ivp_real_object+0x8E))+0x0C))+0x10);
    functions.recheck_ov_element(manager, ivp_real_object);
}

uint32_t FindEntityByIVP(uint32_t ivp_real_object, const char* search_classname)
{
    uint32_t ent_found = 0;
    if(ivp_real_object == 0) return ent_found;

    uint32_t ent = 0;

    while((ent = functions.FindEntityByClassname(fields.gEntList, ent, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(ent))
        {
            uint32_t vphysics_object = *(uint32_t*)(ent+offsets.vphysics_object_offset);

            if(vphysics_object == 0) continue;

            uint32_t ent_ivp_real_object = *(uint32_t*)(vphysics_object+8);

            if(ent_ivp_real_object == ivp_real_object)
            {
                if(ent_found)
                {
                    ConsolePrint("IVP used in more than one entity!");
                    exit(1);
                }

                ent_found = ent;
            }
        }
    }

    if(ent_found)
    {
        char* classname = (char*)(*(uint32_t*)(ent_found+offsets.classname_offset));

        if(search_classname && classname)
        {
            if(strcmp(classname, search_classname) == 0)
            {
                //ConsolePrint("Found [%s] by IVP", search_classname);
                return ent_found;
            }
            else
            return 0;
        }
    }

    return ent_found;
}

void CorrectPhysics()
{   
    uint8_t deferMindist = *(uint8_t*)(fields.deferMindist);
    
    if(deferMindist)
    {
        ConsolePrint("defered!");
        *(uint8_t*)(fields.deferMindist) = 0;
    }
}

void LogVpkMemoryLeaks()
{
    Value* firstLeak = *leakedResourcesVpkSystem;

    int running_total_of_leaks = 0;

    while(firstLeak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(firstLeak->value);

        ValueList refs = the_leak->leaked_refs;
        Value* firstLeakedRef = *refs;

        int leaked_vpk_refs = 0;

        while(firstLeakedRef)
        {
            leaked_vpk_refs++;

            firstLeakedRef = firstLeakedRef->nextVal;
        }

        running_total_of_leaks = running_total_of_leaks + leaked_vpk_refs;

        ConsolePrint("Found [%d] leaked refs in object [%p]", leaked_vpk_refs, the_leak->packed_ref);

        firstLeak = firstLeak->nextVal;
    }

    ConsolePrint("Total VPK leaks [%d]", running_total_of_leaks);
}

void* copy_val(void* val, size_t copy_size)
{
    if(val == 0)
        return 0;
    
    void* copy_ptr = malloc(copy_size);
    memcpy(copy_ptr, val, copy_size);
    return copy_ptr;
}

bool IsAddressExcluded(uint32_t base_address, uint32_t search_address)
{
    for(int i = 0; i < 512; i++)
    {
        if(hook_exclude_list_offset[i] == 0 || hook_exclude_list_base[i] == 0)
            continue;

        uint32_t patch_address = base_address + hook_exclude_list_offset[i];

        if(patch_address == search_address && hook_exclude_list_base[i] == base_address)
            return true;
    }

    return false;
}

uint32_t FindSignature(uint32_t block_start_start, uint32_t block_end_end, Signature signature_main)
{
    uint32_t block_size = block_end_end - block_start_start;

    int signature_search_counter = 0;
    int block_counter = 0;
    int block_fail_offset = 0;
    uint32_t signature_found_start = 0;

    while(true)
    {
        uint8_t arg_byte = signature_main.signature[signature_search_counter];
        uint8_t block_byte = *(uint8_t*)(block_start_start+block_counter+block_fail_offset);

        if(signature_search_counter >= signature_main.signature_size)
            break;

        if(block_start_start + block_fail_offset >= block_end_end)
            break;

        if(arg_byte == block_byte)
        {
            if(block_counter == 0)
                signature_found_start = block_start_start+block_fail_offset;

            signature_search_counter++;
            block_counter++;
        }
        else
        {
            signature_found_start = 0;
            signature_search_counter = 0;
            block_counter = 0;
            block_fail_offset++;
        }
    }

    return signature_found_start;
}

uint32_t ApplyNoOperation(uint32_t block_start_start, uint32_t block_end_end, Signature no_operation_signatures[32], int no_operation_signatures_size)
{
    for(int i = 0; i < no_operation_signatures_size; i++)
    {
        Signature no_operation_signature = no_operation_signatures[i];
        uint32_t block_start_no_operation = FindSignature(block_start_start, block_end_end, no_operation_signature);

        if(block_start_no_operation)
        {
            memset((void*)block_start_no_operation, 0x90, block_end_end - block_start_no_operation);
            return block_start_no_operation;
        }
    }

    return 0;
}

bool ApplyBlockHook(uint32_t block_start_start, uint32_t block_end_end, Signature args_signatures[32], Signature stack_machine_code[32], int stack_arguments_size, Signature no_operation_signatures[32], int no_operation_signatures_size, int expected_arguments, void* hook_pointer)
{
    uint32_t block_size = block_end_end - block_start_start;
    int actual_arguments_found = 0;

    for(int i = 0; i < stack_arguments_size; i++)
    {
        Signature argument_signature = args_signatures[i];
        Signature stack_signature = stack_machine_code[i];

        if(FindSignature(block_start_start, block_end_end, argument_signature))
        {
            uint32_t block_start_no_operation = ApplyNoOperation(block_start_start, block_end_end, no_operation_signatures, no_operation_signatures_size);

            if(block_start_no_operation)
            {
                ConsolePrint("Found the argument signature! %d", block_size);

                //SET STACK MEMORY FROM ARRAY USING CORRECT INDEX
                //APPLY HOOK POINTER

                for(int k = 0; k < stack_signature.signature_size; k++)
                    *(uint8_t*)(block_start_no_operation+k) = stack_signature.signature[k];

                uint32_t hook_block_hook_pointer_base = block_start_no_operation+stack_signature.signature_size;
                uint32_t offset = (uint32_t)hook_pointer - hook_block_hook_pointer_base - 5;

                *(uint8_t*)(hook_block_hook_pointer_base) = 0xE8;
                *(uint32_t*)(hook_block_hook_pointer_base+1) = offset;

                actual_arguments_found++;
            }
        }
        else
        {
            ConsolePrint("Failed to find argument signature %d", i);
        }
    }

    return actual_arguments_found == expected_arguments;
}

void HookMemoryBlock(uint32_t base_address, uint32_t size, Signature start_signatures[32], int start_signatures_size, Signature end_signatures[32], int end_signatures_size, Signature args_signatures[32], Signature stack_machine_code[32], int stack_arguments_size, Signature no_operation_signatures[32], int no_operation_signatures_size, uint32_t estimated_block_size_min, uint32_t estimated_block_size_max, int expected_arguments, void* hook_pointer)
{
    uint32_t search_address = base_address;
    uint32_t search_address_max = base_address+size;

    uint32_t block_start_start = 0;
    uint32_t block_end_start = 0;

    int start_signature_signature_counter = 0;
    int end_signature_signature_counter = 0;

    int signature_found_counter = 0;
    bool found_something = false;
    bool found_start_signature = false;

    while(true)
    {
        if(!found_start_signature)
        {
            if(search_address > search_address_max)
            {
                search_address = base_address;
                start_signature_signature_counter++;

                signature_found_counter = 0;
                found_something = false;
                block_start_start = 0;
            }

            if(start_signature_signature_counter >= start_signatures_size)
            {
                ConsolePrint("No more start signatures left!");
                break;
            }

            // Handle whenever the signature was found successfully

            if(signature_found_counter >= start_signatures[start_signature_signature_counter].signature_size)
            {
                if(IsAddressExcluded(base_address, block_start_start))
                {
                    ConsolePrint("Skipped block hook at [%X]", block_start_start);

                    search_address = block_start_start+1;
                    found_something = false;
                    signature_found_counter = 0;
                    continue;
                }

                signature_found_counter = 0;
                found_start_signature = true;
                found_something = false;
                continue;
            }

            // Handle finding the byte in the signature

            if(*(uint8_t*)(search_address) == start_signatures[start_signature_signature_counter].signature[signature_found_counter])
            {
                // Move on to the next byte since we found this one

                if(signature_found_counter == 0)
                    block_start_start = search_address;

                signature_found_counter++;
                found_something = true;
            }
            else
            {
                // Reset the counter since we did not find the signature

                if(found_something)
                {
                    search_address = block_start_start+1;
                    found_something = false;
                    signature_found_counter = 0;
                    continue;
                }

                signature_found_counter = 0;
            }
        }
        else
        {
            if((search_address > search_address_max) || (search_address > block_start_start+estimated_block_size_max))
            {
                search_address = block_start_start;
                end_signature_signature_counter++;

                found_something = false;
                block_end_start = 0;
                signature_found_counter = 0;
            }

            // Handle whenever it runs out of end signatures to check
            if(end_signature_signature_counter >= end_signatures_size)
            {
                //ConsolePrint("No more end signatures left!");

                next_block:

                search_address = block_start_start+1;

                end_signature_signature_counter = 0;
                
                block_start_start = 0;
                block_end_start = 0;

                signature_found_counter = 0;
                found_something = false;
                found_start_signature = false;
                continue;
            }

            //Handle whenever the signature was found successfully

            if(signature_found_counter >= end_signatures[end_signature_signature_counter].signature_size)
            {
                // Apply the hook

                uint32_t block_size = search_address - block_start_start;

                //ConsolePrint("\nFound signature base and end %p %p", block_start_start, search_address);

                if(!(block_size >= estimated_block_size_min && block_size <= estimated_block_size_max))
                {
                    //ConsolePrint("Out of bounds block size real [%d] min [%d] max [%d]", block_size, estimated_block_size_min, estimated_block_size_max);
                    goto found_nothing;
                }

                if(ApplyBlockHook(block_start_start, search_address, args_signatures, stack_machine_code, stack_arguments_size, no_operation_signatures, no_operation_signatures_size, expected_arguments, hook_pointer))
                {
                    ConsolePrint("\nFound signature base and end %p %p", block_start_start, search_address);
                    goto next_block;
                }

                goto found_nothing;
            }

            // Handle finding the byte in the signature

            if(*(uint8_t*)(search_address) == end_signatures[end_signature_signature_counter].signature[signature_found_counter])
            {
                // Move on to the next byte since we found this one

                if(signature_found_counter == 0)
                    block_end_start = search_address;

                signature_found_counter++;
                found_something = true;
            }
            else
            {
                found_nothing:
                // Reset the counter since we did not find the signature

                if(found_something)
                {
                    search_address = block_end_start+1;
                    found_something = false;
                    signature_found_counter = 0;
                    continue;
                }

                signature_found_counter = 0;
            }
        }

        search_address++;
    }
}

void NoteHookFunctionPatch(uint32_t address, uint32_t length)
{
    HookPatchRecord* patch = (HookPatchRecord*)malloc(sizeof(HookPatchRecord));

    patch->address = address;
    patch->length = length;

    Value* patch_value = CreateNewValue((void*)patch);
    InsertToValuesList(hook_function_patch_notes, patch_value, NULL, true, false);
}

void HookFunction(Library* binary, void* target_pointer, void* hook_pointer)
{
    if(!target_pointer || !hook_pointer)
    {
        ConsolePrint("Failed to hook function due to target pointer or hook pointer being NULL");
        return;
    }

    MemoryRegion* region_start = binary->region;

    while(region_start)
    {
        uint32_t region_start_address = region_start->start;
        uint32_t region_end_address = region_start->end;
        uint32_t region_protections = region_start->protections;

        uint32_t search_address = region_start_address;

        while(search_address + 3 < region_end_address)
        {
            uint32_t four_byte_addr = *(uint32_t*)(search_address);

            if(four_byte_addr == (uint32_t)target_pointer)
            {
                if(IsAddressExcluded(binary->start_address, search_address))
                {
                    ConsolePrint("(abs) Skipped patch at [%X]", search_address);
                    search_address++;
                    continue;
                }

                *(uint32_t*)(search_address) = (uint32_t)hook_pointer;
                NoteHookFunctionPatch(search_address, sizeof(uint32_t));

                search_address++;
                continue;
            }

            uint8_t byte = *(uint8_t*)(search_address);

            if(byte == 0xE8 || byte == 0xE9)
            {
                uint32_t call_address = *(uint32_t*)(search_address + 1);
                uint32_t chk = search_address + call_address + 5;

                if(chk == (uint32_t)target_pointer)
                {
                    if(IsAddressExcluded(binary->start_address, search_address))
                    {
                        ConsolePrint("(unsigned) Skipped patch at [%X]", search_address);
                        search_address++;
                        continue;
                    }

                    uint32_t offset = (uint32_t)hook_pointer - search_address - 5;
                    *(uint32_t*)(search_address+1) = offset;
                    NoteHookFunctionPatch(search_address + 1, sizeof(uint32_t));
                }
                else
                {
                    chk = search_address + (int32_t)call_address + 5;

                    if(chk == (uint32_t)target_pointer)
                    {
                        if(IsAddressExcluded(binary->start_address, search_address))
                        {
                            ConsolePrint("(signed) Skipped patch at [%X]", search_address);
                            search_address++;
                            continue;
                        }

                        ConsolePrint("(signed) Hooked address: [%X]", search_address - binary->start_address);
                        uint32_t offset = (uint32_t)hook_pointer - search_address - 5;
                        *(uint32_t*)(search_address+1) = offset;
                        NoteHookFunctionPatch(search_address + 1, sizeof(uint32_t));
                    }
                }
            }

            search_address++;
        }

        region_start = region_start->nextRegion;
    }
}

Library* FindLibrary(char* lib_name, bool less_intense_search)
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] == 0) continue;
        Library* existing_lib = (Library*)loaded_libraries[i];
        
        if(less_intense_search)
        {
            if(strcasestr(existing_lib->library_signature, lib_name) != NULL) return existing_lib;
        }

        if(strcmp(existing_lib->library_signature, lib_name) == 0) return existing_lib;
    }

    return NULL;
}

MemoryRegion* AllocateMemoryRegion(uint32_t start, uint32_t end, uint32_t protections)
{
    MemoryRegion* region = (MemoryRegion*)malloc(sizeof(MemoryRegion));

    region->start = start;
    region->end = end;
    region->protections = protections;
    region->snapshot_size = end - start;
    region->snapshot = (uint8_t*)malloc(region->snapshot_size);
    region->nextRegion = NULL;

    memcpy(region->snapshot, (void*)region->start, region->snapshot_size);
    return region;
}

void ReleaseMemoryRegion(MemoryRegion* region)
{
    while(region)
    {
        MemoryRegion* nextRegion = region->nextRegion;

        free(region->snapshot);
        free(region);

        region = nextRegion;
    }
}

void ClearLoadedLibraries()
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] != 0)
        {
            Library* delete_this = (Library*)loaded_libraries[i];

            dlclose(delete_this->library_linkmap);
            free(delete_this->library_signature);

            ReleaseMemoryRegion(delete_this->region);
            ReleaseMemoryRegion(delete_this->original_memory);

            free(delete_this);

            loaded_libraries[i] = 0;
        }
    }
}

void ClearLoadedLibraryRegions()
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] != 0)
        {
            Library* lib = (Library*)loaded_libraries[i];
            MemoryRegion* region_start = lib->region;

            while(region_start)
            {
                MemoryRegion* nextRegion = region_start->nextRegion;

                free(region_start->snapshot);
                free(region_start);

                region_start = nextRegion;
            }

            lib->region = NULL;
        }
    }
}

Library* LoadLibrary(char* library_full_path)
{
    if(library_full_path)
    {
        Library* found_lib = FindLibrary(library_full_path, false);
        if(found_lib) return found_lib;

        struct link_map* library_lm = (struct link_map*)(dlopen(library_full_path, RTLD_NOLOAD | RTLD_NOW));

        if(library_lm)
        {
            for(int i = 0; i < 512; i++)
            {
                if(loaded_libraries[i] == 0)
                {
                    Library* new_lib = (Library*)(malloc(sizeof(Library)));

                    new_lib->library_linkmap = (void*)library_lm;
                    new_lib->library_signature = (char*)copy_val(library_full_path, strlen(library_full_path)+1);

                    new_lib->region = NULL;
                    new_lib->original_memory = NULL;

                    new_lib->start_address = library_lm->l_addr;
                    new_lib->end_address = 0;

                    loaded_libraries[i] = (uint32_t)new_lib;
                    
                    ConsolePrint("Loaded [%s]", library_full_path);
                    return new_lib;
                }
            }

            ConsolePrint("Failed to save library to list!");
            exit(EXIT_FAILURE);
        }
    }

    return NULL;
}

bool IsOurLibraryPath(char* abs_path)
{
    if(!abs_path) return false;

    for(int i = 0; i < 512; i++)
    {
        if(our_libraries[i] == 0) continue;
        if(strcasestr(abs_path, (char*)our_libraries[i]) != NULL) return true;
    }

    return false;
}

char* getlibrary(char* file_line)
{
    if(!file_line) return NULL;

    uint32_t start_addr = 0;
    uint32_t end_addr = 0;
    uint32_t file_offset = 0;
    char perms[8] = {0};
    char dev[32] = {0};
    unsigned long inode = 0;
    int path_start = 0;

    int parsed = sscanf(file_line, "%x-%x %7s %x %31s %lu %n", &start_addr, &end_addr, perms, &file_offset, dev, &inode, &path_start);
    if(parsed < 6) return NULL;

    while(file_line[path_start] == ' ' || file_line[path_start] == '\t')
    {
        path_start++;
    }

    if(file_line[path_start] != '/') return NULL;

    char* abs_path = (char*)copy_val(file_line + path_start, strlen(file_line + path_start) + 1);
    if(!abs_path) return NULL;

    if(access(abs_path, F_OK) == 0)
    {
        return abs_path;
    }

    free(abs_path);
    return NULL;
}

void TakeRegionMemorySnapshot(bool original_memory)
{
    FILE* smaps_file = fopen("/proc/self/smaps", "r");    

    if(!smaps_file)
    {
        ConsolePrint("Error opening smaps");
        return;
    }

    ClearLoadedLibraryRegions();

    char* file_line = (char*) malloc(1024);
    char* current_abs_path = (char*) malloc(1024);

    snprintf(file_line, 1024, "%s", "");
    snprintf(current_abs_path, 1024, "%s", "");
    Library* currentLibrary = NULL;

    while(fgets(file_line, 1024, smaps_file))
    {
        sscanf(file_line, "%[^\n]s", file_line);
        char* abs_path = getlibrary(file_line);
        
        if(abs_path != NULL && strcasecmp(abs_path, current_abs_path) != 0)
        {
            snprintf(current_abs_path, 1024, "%s", abs_path);
            currentLibrary = NULL;

            if(IsOurLibraryPath(current_abs_path))
            {
                currentLibrary = LoadLibrary(current_abs_path);
                //ConsolePrint("%s", currentLibrary->library_signature);
            }
        }

        free(abs_path);

        if(!currentLibrary) continue;

        char* file_line_cpy = (char*) malloc(strlen(file_line)+1);
        snprintf(file_line_cpy, strlen(file_line)+1, "%s", file_line);

        char* address_range = strtok(file_line_cpy, " \t");
        char* protections = strtok(NULL, " \t");

        char* start_address = strtok(address_range, "-");
        char* end_address = strtok(NULL, "-");

        uint32_t start_address_parsed = 0;
        uint32_t end_address_parsed = 0;

        if(start_address) start_address_parsed = strtoul(start_address, NULL, 16);
        if(end_address) end_address_parsed = strtoul(end_address, NULL, 16);

        if(start_address_parsed && end_address_parsed && protections)
        {
            uint32_t save_protections = PROT_NONE;

            if(strstr(protections, "r") != 0)
                save_protections = PROT_READ;
            if(strstr(protections, "w") != 0)
                save_protections = save_protections | PROT_WRITE;
            if(strstr(protections, "x") != 0)
                save_protections = save_protections | PROT_EXEC;

            currentLibrary->end_address = end_address_parsed;

            size_t pagesize = sysconf(_SC_PAGE_SIZE);
            uint32_t pagestart = start_address_parsed & -pagesize;
            uint32_t protect_length = end_address_parsed - pagestart;

            if(mprotect((void*)pagestart, protect_length, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
            {
                free(file_line_cpy);
                continue;
            }

            MemoryRegion* new_region = AllocateMemoryRegion(start_address_parsed, end_address_parsed, save_protections);

            new_region->nextRegion = currentLibrary->region;
            currentLibrary->region = new_region;

            if(original_memory)
            {
                MemoryRegion* new_region_original = AllocateMemoryRegion(start_address_parsed, end_address_parsed, save_protections);

                new_region_original->nextRegion = currentLibrary->original_memory;
                currentLibrary->original_memory = new_region_original;
            }
        }

        free(file_line_cpy);
    }

    free(file_line);
    free(current_abs_path);
    fclose(smaps_file);
}

void ForceMemoryAccess()
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] != 0)
        {
            Library* current_lib = (Library*)loaded_libraries[i];
            MemoryRegion* region_start = current_lib->region;

            while(region_start)
            {
                uint32_t region_start_address = region_start->start;
                uint32_t region_end_address = region_start->end;
                uint32_t region_protections = region_start->protections;

                size_t pagesize = sysconf(_SC_PAGE_SIZE);
                uint32_t pagestart = region_start_address & -pagesize;
                uint32_t protect_length = region_end_address - pagestart;

                if(mprotect((void*)pagestart, protect_length, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
                {
                    //ConsolePrint("Failed protection change: [%X] [%X]", memory_prots_save_list[i+1], memory_prots_save_list[i]);

                    //SELINUX shite

                    //perror("mprotect");
                    //exit(EXIT_FAILURE);
                }
                else
                {
                    //ConsolePrint("Passed protection change: [%X] [%X]", region_end_address, region_start_address);
                }

                region_start = region_start->nextRegion;
            }
        }
    }
}

void RestoreMemorySnapshots()
{
    Value* patch_note = *hook_function_patch_notes;

    while(patch_note)
    {
        HookPatchRecord* patch = (HookPatchRecord*)patch_note->value;

        if(patch && patch->address != 0 && patch->length > 0)
        {
            for(int i = 0; i < 512; i++)
            {
                if(loaded_libraries[i] == 0)
                    continue;

                Library* current_lib = (Library*)loaded_libraries[i];
                MemoryRegion* region_start = current_lib->original_memory;
                bool restored_patch = false;

                while(region_start)
                {
                    if(patch->address >= region_start->start && patch->address < region_start->end)
                    {
                        size_t snapshot_offset = (size_t)(patch->address - region_start->start);
                        memcpy((void*)patch->address, region_start->snapshot + snapshot_offset, patch->length);
                        restored_patch = true;
                        break;
                    }

                    region_start = region_start->nextRegion;
                }

                if(restored_patch)
                    break;
            }
        }

        patch_note = patch_note->nextVal;
    }
}

void RestoreExecutableMemorySnapshots()
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] == 0)
            continue;

        Library* current_lib = (Library*)loaded_libraries[i];
        MemoryRegion* region_start = current_lib->original_memory;

        while(region_start)
        {
            if((region_start->protections & PROT_EXEC) != 0)
            {
                memcpy((void*)region_start->start, region_start->snapshot, region_start->snapshot_size);
            }

            region_start = region_start->nextRegion;
        }
    }
}

void RestoreMemoryProtections()
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] != 0)
        {
            Library* current_lib = (Library*)loaded_libraries[i];
            MemoryRegion* region_start = current_lib->region;

            while(region_start)
            {
                uint32_t region_start_address = region_start->start;
                uint32_t region_end_address = region_start->end;
                uint32_t region_protections = region_start->protections;

                size_t pagesize = sysconf(_SC_PAGE_SIZE);
                uint32_t pagestart = region_start_address & -pagesize;
                uint32_t protect_length = region_end_address - pagestart;

                if(mprotect((void*)pagestart, protect_length, region_protections) == -1)
                {
                    perror("mprotect");
                    exit(EXIT_FAILURE);
                }

                region_start = region_start->nextRegion;
            }
        }
    }
}

inline void ZeroVector(uint32_t vector)
{
    *(float*)(vector) = 0;
    *(float*)(vector+4) = 0;
    *(float*)(vector+8) = 0;
}

inline bool IsVectorNaN(uint32_t base)
{
    float s0 = *(float*)(base);
    float s1 = *(float*)(base+4);
    float s2 = *(float*)(base+8);

    return (s0 != s0) || (s1 != s1) || (s2 != s2);
}

inline bool IsVectorInf(uint32_t base)
{
    float s0 = *(float*)(base);
    float s1 = *(float*)(base+4);
    float s2 = *(float*)(base+8);

    return ((s0 != 0) && ((s0 / 2) == s0)) ||
           ((s1 != 0) && ((s1 / 2) == s1)) ||
           ((s2 != 0) && ((s2 / 2) == s2));
}


inline bool IsValidVector(uint32_t base)
{
    if(IsVectorNaN(base) || IsVectorInf(base)) return false;
    return true;
}

inline bool IsEntityPositionReasonable(uint32_t v)
{
    float x = *(float*)(v);
    float y = *(float*)(v+4);
    float z = *(float*)(v+8);

    if(IsValidVector(v))
    {
        float r = 32767.0f;

        return
            x > -r && x < r &&
            y > -r && y < r &&
            z > -r && z < r;
    }

    return false;
}

void UpdatePlayerCollisions()
{
    functions.CleanupDeleteList(0);

    uint32_t entity = 0;

    while((entity = functions.FindEntityByClassname(fields.gEntList, entity, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(entity))
        {
            uint32_t vphysics_object = *(uint32_t*)(entity+offsets.vphysics_object_offset);

            if(vphysics_object == 0) continue;

            uint32_t ivp_real_object = *(uint32_t*)(vphysics_object+8);
            uint32_t manager = *(uint32_t*)((*(uint32_t*)((*(uint32_t*)(ivp_real_object+0x8E))+0x0C))+0x10);

            functions.recheck_ov_element(manager, ivp_real_object);
        }
    }

    functions.CleanupDeleteList(0);
}

void UpdateOtherCollisions()
{
    functions.CleanupDeleteList(0);

    uint32_t entity = 0;

    while((entity = functions.FindEntityByClassname(fields.gEntList, entity, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(entity))
        {
            uint32_t m_Network = *(uint32_t*)(entity+offsets.mnetwork_offset);

            if(!m_Network)
            {
                uint32_t vphysics_object = *(uint32_t*)(entity+offsets.vphysics_object_offset);

                if(vphysics_object == 0) continue;
    
                uint32_t ivp_real_object = *(uint32_t*)(vphysics_object+8);
                uint32_t manager = *(uint32_t*)((*(uint32_t*)((*(uint32_t*)(ivp_real_object+0x8E))+0x0C))+0x10);
    
                functions.recheck_ov_element(manager, ivp_real_object);
            }
        }
    }

    functions.CleanupDeleteList(0);
}

void UpdateAllCollisions(bool cleanup)
{
    if(cleanup) functions.CleanupDeleteList(0);

    uint32_t entity = 0;

    while((entity = functions.FindEntityByClassname(fields.gEntList, entity, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(entity))
        {
            uint32_t vphysics_object = *(uint32_t*)(entity+offsets.vphysics_object_offset);

            if(vphysics_object == 0) continue;

            uint32_t ivp_real_object = *(uint32_t*)(vphysics_object+8);
            uint32_t manager = *(uint32_t*)((*(uint32_t*)((*(uint32_t*)(ivp_real_object+0x8E))+0x0C))+0x10);

            functions.recheck_ov_element(manager, ivp_real_object);
        }
    }

    if(cleanup) functions.CleanupDeleteList(0);
}

void UpdateCollisions(bool flush)
{
    functions.CleanupDeleteList(0);

    Value* first_ivp = *ivp_list;

    while(first_ivp)
    {
        Value* nextIvp = first_ivp->nextVal;
        uint32_t ivp_real_object = (uint32_t)first_ivp->value;

        UpdateCollisionByIVP(ivp_real_object);

        if(flush) free(first_ivp);
        first_ivp = nextIvp;
    }

    if(flush) *ivp_list = NULL;

    functions.CleanupDeleteList(0);
}

void SetServerSleepStatus()
{
    uint32_t firstPlayer = functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"player");

    if(firstPlayer)
        server_sleeping = false;
    else if(firstplayer_hasjoined)
        server_sleeping = true;
}

void FixPlayerCollisionGroup()
{
    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(fields.gEntList, player, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(player))
        {
            uint32_t collision_flags = *(uint32_t*)(player+offsets.m_CollisionGroup_offset);

            if(collision_flags & 4)
            {
                *(uint32_t*)(player+offsets.m_CollisionGroup_offset) -= 4;
            }

            if(!(collision_flags & 2))
            {
                *(uint32_t*)(player+offsets.m_CollisionGroup_offset) += 2;
            }
        }
    }
}

void DisablePlayerWorldSpawnCollision()
{
    uint32_t worldspawn = functions.FindEntityByClassname(fields.gEntList, 0, (uint32_t)"worldspawn");
    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(fields.gEntList, player, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(worldspawn) && IsEntityValid(player))
        {
            float player_velocity_x = *(uint32_t*)(player+offsets.abs_velocity_offset);
            float player_velocity_y = *(uint32_t*)(player+offsets.abs_velocity_offset+4);
            float player_velocity_z = *(uint32_t*)(player+offsets.abs_velocity_offset+8);

            if(player_velocity_x > 3000000000.0 || player_velocity_y > 3000000000.0)
            {
                functions.DisableEntityCollisions(player, worldspawn);
            }
            else
            {
                functions.EnableEntityCollisions(player, worldspawn);
            }

            //ConsolePrint("%f %f %f", player_velocity_x, player_velocity_y, player_velocity_z);

            //if(!player_worldspawn_collision_disabled)
            //{
            //    functions.DisableEntityCollisions(player, worldspawn);
            //}
        }
    }

    player_worldspawn_collision_disabled = true;
}

void DisablePlayerCollisions()
{
    uint32_t current_player = 0;

    while((current_player = functions.FindEntityByClassname(fields.gEntList, current_player, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(current_player))
        {
            uint32_t other_players = 0;

            while((other_players = functions.FindEntityByClassname(fields.gEntList, other_players, (uint32_t)"player")) != 0)
            {
                if(IsEntityValid(other_players) && other_players != current_player)
                {
                    //ConsolePrint("Disable player collisions!");
                    functions.DisableEntityCollisions(current_player, other_players);
                }
            }
        }
    }
}

void RemoveBadEnts()
{
    uint32_t ent = 0;

    while((ent = functions.FindEntityByClassname(fields.gEntList, ent, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(ent))
        {
            uint32_t abs_origin = ent+offsets.abs_origin_offset;
            uint32_t origin = ent+offsets.origin_offset;
            uint32_t abs_angles = ent+offsets.abs_angles_offset;
            uint32_t angles = ent+offsets.angles_offset;
            uint32_t abs_velocity = ent+offsets.abs_velocity_offset;
            uint32_t velocity = ent+offsets.velocity_offset;

            if
            (
            
            !IsEntityPositionReasonable(abs_origin)
            || 
            !IsEntityPositionReasonable(abs_angles)
            ||
            !IsEntityPositionReasonable(abs_velocity)

            ||

            !IsEntityPositionReasonable(origin)
            ||
            !IsEntityPositionReasonable(angles)
            ||
            !IsEntityPositionReasonable(velocity)
            
            )
            {
                char* classname = (char*)(*(uint32_t*)(ent+offsets.classname_offset));
                
                if(classname)
                    ConsolePrint("Removed bad ent! [%s]", classname);
                else
                    ConsolePrint("Removed bad ent!");
                
                HandleSpecificEntityRemoval(ent, true, true, true, true);
            }
        }
    }
}

bool VerifyEntity(uint32_t entity_object, bool validate, bool validate_player)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    if(entity_object == 0) return false;

    char* classname = (char*)(*(uint32_t*)(entity_object+offsets.classname_offset));
    uint32_t refHandle = *(uint32_t*)(entity_object+offsets.refhandle_offset);
    uint32_t object_verify = GetCBaseEntity(refHandle);

    if(object_verify == 0)
    {
        if(validate_player)
        {
            if(classname && strcmp(classname, "player") == 0)
            {
                ConsolePrint("Allowed player entity without validation");
                object_verify = entity_object;
            }
        }
        else if(!validate)
        {
            ConsolePrint("Warning: Entity delete request granted without validation!");
            object_verify = entity_object;
        }
    }

    if(object_verify)   return true;
    else                return false;
}

inline bool IsMarkedForDeletion(uint32_t arg0)
{
    return *(uint32_t*)(*(uint32_t*)(arg0 + 8) + offsets.ismarked_offset) & 1;
}

inline uint32_t IsEntityValid(uint32_t entity)
{
    pOneArgProt pDynamicOneArgFunc;
    if(entity == 0) return entity;

    uint32_t object = GetCBaseEntity(*(uint32_t*)(entity+offsets.refhandle_offset));

    if(object)
    {
        if(IsMarkedForDeletion(object+offsets.iserver_offset))
            return 0;

        return entity;
    }

    return 0;
}

inline ValueList AllocateValuesList()
{
    ValueList list = (ValueList) malloc(sizeof(ValueList));
    *list = NULL;
    return list;
}

inline Value* CreateNewValue(void* valueInput)
{
    Value* val = (Value*) malloc(sizeof(Value));

    val->value = valueInput;
    val->nextVal = NULL;
    return val;
}

int DeleteAllValuesInList(ValueList list, bool free_val, pthread_mutex_t* lockInput)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    int removed_items = 0;

    if(!list || !*list)
    {
        
        if(lockInput)
        {
            pthread_mutex_unlock(lockInput);
        }

        return removed_items;
    }
    
    Value* aValue = *list;

    while(aValue)
    {
        Value* detachedValue = aValue->nextVal;
        if(free_val) free(aValue->value);
        free(aValue);
        aValue = detachedValue;

        removed_items++;
    }

    *list = NULL;

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return removed_items;
}

Value* FindStringInList(ValueList list, const char* search_val, pthread_mutex_t* lockInput, bool substring, Value* start_value)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    Value* aValue = *list;

    if(start_value)
    {
        aValue = start_value;
    }

    while(aValue)
    {
        char* list_item = (char*)aValue->value;

        if(substring)
        {
            if(list_item && strstr(list_item, search_val) != NULL)
            {
                if(lockInput)
                {
                    pthread_mutex_unlock(lockInput);
                }

                return aValue;
            }
        }
        else
        {
            if(list_item && strcmp(list_item, search_val) == 0)
            {
                if(lockInput)
                {
                    pthread_mutex_unlock(lockInput);
                }

                return aValue;
            }
        }
        
        aValue = aValue->nextVal;
    }

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return NULL;
}

bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    Value* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->value == (uint32_t)searchVal)
        {
            if(lockInput)
            {
                pthread_mutex_unlock(lockInput);
            }

            return true;
        }
        
        aValue = aValue->nextVal;
    }

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return false;
}

bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    Value* aValue = *list;

    if(aValue == NULL)
    {
        if(lockInput)
        {
            pthread_mutex_unlock(lockInput);
        }

        return false;
    }

    //search at the start of the list
    if(((uint32_t)aValue->value) == ((uint32_t)searchVal))
    {
        Value* detachedValue = aValue->nextVal;
        free(*list);
        *list = detachedValue;

        if(lockInput)
        {
            pthread_mutex_unlock(lockInput);
        }

        return true;
    }

    //search the rest of the list
    while(aValue->nextVal)
    {
        if(((uint32_t)aValue->nextVal->value) == ((uint32_t)searchVal))
        {
            Value* detachedValue = aValue->nextVal->nextVal;

            free(aValue->nextVal);
            aValue->nextVal = detachedValue;

            if(lockInput)
            {
                pthread_mutex_unlock(lockInput);
            }

            return true;
        }

        aValue = aValue->nextVal;
    }

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return false;
}

int ValueListItems(ValueList list, pthread_mutex_t* lockInput)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    Value* aValue = *list;
    int counter = 0;

    while(aValue)
    {
        counter++;
        aValue = aValue->nextVal;
    }

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return counter;
}

bool InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* lockInput, bool tail, bool duplicate_chk)
{
    if(lockInput)
    {
        while(pthread_mutex_trylock(lockInput) != 0);
    }

    if(duplicate_chk)
    {
        Value* aValue = *list;

        while(aValue)
        {
            if((uint32_t)aValue->value == (uint32_t)head->value)
            {
                free(head);

                if(lockInput)
                {
                    pthread_mutex_unlock(lockInput);
                }

                return false;
            }
        
            aValue = aValue->nextVal;
        }
    }

    if(tail)
    {
        Value* aValue = *list;

        while(aValue)
        {
            if(aValue->nextVal == NULL)
            {
                aValue->nextVal = head;

                if(lockInput)
                {
                    pthread_mutex_unlock(lockInput);
                }

                return true;
            }

            aValue = aValue->nextVal;
        }
    }

    head->nextVal = *list;
    *list = head;

    if(lockInput)
    {
        pthread_mutex_unlock(lockInput);
    }

    return true;
}

EntityKV* CreateNewEntityKV(uint32_t refHandle, uint32_t keyIn, uint32_t valueIn)
{
    EntityKV* kv = (EntityKV*) malloc(sizeof(EntityKV));

    kv->entityRef = refHandle;
    kv->key = keyIn;
    kv->value = valueIn;

    return kv;
}

void ZeroArrayList(uint32_t* list)
{
    for(int i = 0; i < 512; i++)
    {
        list[i] = 0;
    }
}

bool IsValueInArrayList(uint32_t* list, uint32_t value)
{
    for(int i = 0; i < 512; i++)
    {
        if(list[i] == value) return true;
    }

    return false;
}

void InsertToArrayList(uint32_t* list, uint32_t value)
{
    if(value == 0) return;

    for(int i = 0; i < 512; i++)
    {
        if(list[i] == value) return;
    }

    for(int i = 0; i < 512; i++)
    {
        if(list[i] == 0)
        {
            list[i] = value;
            return;
        }
    }
}

void RemoveHl2Ragdolls()
{
    uint32_t entity = 0;

    while((entity = functions.FindEntityByClassname(fields.gEntList, entity, (uint32_t)"hl2mp_ragdoll")) != 0)
    {
        if(IsEntityValid(entity))
        {
            ConsolePrint("Removed hl2mp_ragdoll entity!");
            HandleSpecificEntityRemoval(entity, true, true, true, true);
        }
    }
}

void TeleportPlayersToTransition()
{
    uint32_t entity = 0;

    while((entity = functions.FindEntityByClassname(fields.gEntList, entity, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(entity))
        {
            uint32_t player_check_ref = *(uint32_t*)(entity+offsets.refhandle_offset);

            if(!IsValueInArrayList(transitioned_clients, player_check_ref))
            {
                if(transitioning_player[0] == 0 && transitioning_player[1] == 0 && transitioning_player[2] == 0)
                {
                    ConsolePrint("Transition position not set, cannot teleport player!");
                    continue;
                }

                UpdateEntityPosition(entity, transitioning_player[0], transitioning_player[1], transitioning_player[2]);
                ConsolePrint("Teleported player to transition!");
            }
        }
    }
}

int ReleaseLeakedMemory(ValueList leakList, bool destroy)
{
    if(!leakList)
        return 0;
    
    Value* leak = *leakList;
    char listName[256];
    snprintf(listName, 256, "Unknown List");

    if(!leak)
    {
        if(destroy)
        {
            free(leakList);
            leakList = NULL;
            return 0;
        }

        ConsolePrint("[%s] Attempted to free leaks from an empty leaked resources list!", listName);
        return 0;
    }

    int total_items = ValueListItems(leakList, NULL);

    while(leak)
    {
        Value* detachedValue = leak->nextVal;

        //ConsolePrint("[%s] FREED MEMORY LEAK WITH REF: [%X]", listName, leak->value);
        free(leak->value);
        free(leak);

        leak = detachedValue;
    }

    *leakList = NULL;

    ConsolePrint("FREED [%d] memory allocations", total_items);

    if(destroy)
    {
        free(leakList);
        leakList = NULL;
    }

    return total_items;
}