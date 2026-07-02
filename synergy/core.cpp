#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"

#include "synergy/core.h"

synergy_game_fields synergy_fields;
synergy_game_offsets synergy_offsets;
synergy_game_functions synergy_functions;

Library* synergy_srv;

bool sdktools_passed;

int save_frames;
int restore_delay_frames;
bool savegame;
bool savegame_autosave;
bool savegame_internal;
bool saved_game_once;
bool disable_player_restore;

ValueList save_player_vehicles_list;
ValueList save_map_vehicle_modelscale_list;

bool IsTrackedVehicleClassname(const char* clsname)
{
    if(!clsname)
        return false;

    return
    (
        strcmp(clsname, "prop_vehicle_jeep") == 0
        ||
        strcmp(clsname, "prop_vehicle_mp") == 0
        ||
        strcmp(clsname, "prop_vehicle_airboat") == 0
    );
}

void InitCore()
{
    static const char* library_paths[] = {
        "/synergy/bin/server_srv.so",
        "/bin/engine_srv.so",
        "/bin/dedicated_srv.so",
        "/bin/vphysics_srv.so",
        "/synergy/bin/synergy_srv.so",
        "/extensions/sdktools.ext.2.sdk2013.so"
    };

    for(size_t i = 0; i < (sizeof(library_paths) / sizeof(library_paths[0])); i++)
    {
        our_libraries[i] = (uint32_t)library_paths[i];
    }
}

bool IsAllowedToPatchSdkTools(Library* lib)
{
    uint32_t lib_integrity_chk_addr = lib->start + 0x00057919;
    uint32_t str_len = 11;

    bool integrity_chk = (lib_integrity_chk_addr + str_len) <= (lib->end);

    if(integrity_chk)
    {
        char* ext_ver = (char*)lib_integrity_chk_addr;

        if(strcmp(ext_ver, "1.10.0.6503") == 0)
        {
            ConsolePrint("\nSDKTools Memory Integrity Passed!\n");
            return true;
        }
    }

    return false;
}

void PopulateHookExclusionLists()
{
    hook_exclude_list_base[0] = server_srv->start;
    hook_exclude_list_offset[0] = 0x008A0E2F;

    hook_exclude_list_base[1] = vphysics_srv->start;
    hook_exclude_list_offset[1] = 0x0011D336;
}

uint32_t GetCBaseEntity(uint32_t EHandle)
{
    uint32_t EntityList = fields.gEntList;
    uint32_t refHandle = (EHandle & 0xFFF) << 4;

    EHandle = EHandle >> 0x0C;

    if(*(uint32_t*)(EntityList+refHandle+8) == EHandle)
    {
        uint32_t CBaseEntity = *(uint32_t*)(EntityList+refHandle+4);
        return CBaseEntity;
    }

    return 0;
}

void ReleaseLeakedPackedEntities(uint32_t snapManager)
{
    pTwoArgProt pDynamicTwoArgFunc;

    int freed_leaks = 0;

    for(int i = 0; i < 2048; i++)
    {
        //ConsolePrint("trying [%d] ent", i);
        uint32_t computed_ref = *(uint32_t*)(snapManager+i*4+0x70);

        if(computed_ref != 0)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(functions.RemoveEntitySnapReference);
            pDynamicTwoArgFunc(snapManager, computed_ref);

            *(uint32_t*)(snapManager+i*4+0x70) = 0;
            freed_leaks++;
        }
    }

    ConsolePrint("Purged [%d] packed ents!", freed_leaks);
}

void ExtensionUpdateOnRemove(uint32_t arg0)
{
    Vector emptyVector;
    pThreeArgProt pDynamicThreeArgFunc;
    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(strcmp(classname, "player") == 0)
    {
        ConsolePrint("left vehicle before player leaves!");

        //LeaveVehicle
        pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*) ((*(uint32_t*)(arg0))+synergy_offsets.leavevehicle_offset) );
        pDynamicThreeArgFunc(arg0, (uint32_t)&emptyVector, (uint32_t)&emptyVector);
    }
}

void HandleSpecificEntityRemoval(uint32_t object, bool validate, bool validate_player, bool slow, bool crash_server)
{
    Vector emptyVector;
    pThreeArgProt pDynamicThreeArgFunc;

    if(object == 0) return;

    if(VerifyEntity(object, validate, validate_player))
    {
        char* classname = (char*)(*(uint32_t*)(object+offsets.classname_offset));

        if(classname && strcmp(classname, "player") == 0)
        {
            if(isTicking && slow)
            {
                ConsolePrint("Tried killing player but was protected & respawned!");
    
                //LeaveVehicle
                pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*) ((*(uint32_t*)(object))+synergy_offsets.leavevehicle_offset) );
                pDynamicThreeArgFunc(object, (uint32_t)&emptyVector, (uint32_t)&emptyVector);
    
                UpdateEntityPosition(object, 0, 0, 0);
    
                emptyVector.x = 0;
                emptyVector.y = 0;
                emptyVector.z = 0;
    
                //LeaveVehicle
                pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*) ((*(uint32_t*)(object))+synergy_offsets.leavevehicle_offset) );
                pDynamicThreeArgFunc(object, (uint32_t)&emptyVector, (uint32_t)&emptyVector);

                UpdateEntityPosition(object, 0, 0, 0);
                return;
            }
        }
    
        if(savegame_autosave || savegame_internal)
        {
            ConsolePrint("WARNING: Removing [%s] while a save file is being made!", classname);
            functions.RemoveInsta(object);
            return;
        }

        if(slow)    functions.RemoveNormal(object);
        else        functions.RemoveInsta(object);

        return;
    }

    uint32_t first_return = ((uint32_t)__builtin_return_address(0)) - server_srv->start;
    uint32_t second_return = ((uint32_t)__builtin_return_address(1)) - server_srv->start;
    uint32_t third_return = ((uint32_t)__builtin_return_address(2)) - server_srv->start;
    uint32_t fourth_return = ((uint32_t)__builtin_return_address(3)) - server_srv->start;

    ConsolePrint("Failed to validate entity 1:%p 2:%p 3:%p 4:%p", first_return, second_return, third_return, fourth_return);
    if(crash_server) exit(EXIT_FAILURE);
}

void SaveGame_Extension()
{
    save_frames = 0;

    SaveMapVehicleModelScales();
    MakePlayersLeaveVehicles();
    FixCars();

    functions.CleanupDeleteList(0);

    savegame_autosave = true;

    synergy_functions.Autosave_Silent(0);

    savegame_autosave = false;

    functions.CleanupDeleteList(0);
    
    saved_game_once = true;
}

void SaveMapVehicleModelScales()
{
    uint32_t mainEnt = 0;

    while((mainEnt = functions.FindEntityByClassname(fields.gEntList, mainEnt, (uint32_t)"*")) != 0)
    {
        if(!IsEntityValid(mainEnt))
            continue;

        char* clsname = (char*)(*(uint32_t*)(mainEnt+offsets.classname_offset));

        if(!IsTrackedVehicleClassname(clsname))
            continue;

        float* modelscale_copy = (float*)malloc(sizeof(float));
        *modelscale_copy = *(float*)(mainEnt+offsets.modelscale_offset);

        Value* vehicle_value = CreateNewValue((void*)*(uint32_t*)(mainEnt+offsets.refhandle_offset));
        Value* modelscale_value = CreateNewValue((void*)modelscale_copy);

        InsertToValuesList(save_map_vehicle_modelscale_list, vehicle_value, NULL, true, false);
        InsertToValuesList(save_map_vehicle_modelscale_list, modelscale_value, NULL, true, false);

        functions.SetModelScale(mainEnt, 1.0, 0);
    }
}

void RestoreMapVehicleModelScales()
{
    Value* first_vehicle = *save_map_vehicle_modelscale_list;

    while(first_vehicle && first_vehicle->nextVal)
    {
        uint32_t vehicle = GetCBaseEntity((uint32_t)first_vehicle->value);
        float* modelscale_ptr = (float*)first_vehicle->nextVal->value;

        if(IsEntityValid(vehicle))
        {
            functions.SetModelScale(vehicle, *modelscale_ptr, 0);
        }

        Value* next_vehicle = first_vehicle->nextVal->nextVal;

        free(modelscale_ptr);
        free(first_vehicle->nextVal);
        free(first_vehicle);

        first_vehicle = next_vehicle;
    }

    *save_map_vehicle_modelscale_list = NULL;
}

void FixCars()
{
    uint32_t mainEnt = 0;

    while((mainEnt = functions.FindEntityByClassname(fields.gEntList, mainEnt, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(mainEnt))
        {
            char* clsname = (char*) ( *(uint32_t*)(mainEnt+offsets.classname_offset) );
        
            if(!IsTrackedVehicleClassname(clsname))
                continue;
    
            char* model = (char*)(*(uint32_t*)(mainEnt+synergy_offsets.vehicle_model_offset));
            char* script = (char*)(*(uint32_t*)(mainEnt+synergy_offsets.vehicle_script_offset));
    
            bool fixed_model = FixSlashes(model);
            bool fixed_script = FixSlashes(script);
    
            if(fixed_model)
            {
                ConsolePrint("FIXED_MODEL_NAME: [%s]", model);
            }
    
            if(fixed_script)
            {
                ConsolePrint("FIXED_SCRIPT_NAME: [%s]", script);
            }
        }
    }
}

uint32_t GetPassengerIndex(uint32_t player, uint32_t player_vehicle)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(player) && IsEntityValid(player_vehicle))
    {
        char* vehicle_classname = (char*)(*(uint32_t*)(player_vehicle+offsets.classname_offset));

        if(IsTrackedVehicleClassname(vehicle_classname))
        {
            uint32_t iserver_vehicle = *(uint32_t*)(player_vehicle+synergy_offsets.iserver_vehicle_offset);
            uint32_t base_vehicle = *(uint32_t*)(iserver_vehicle+synergy_offsets.base_vehicle_offset);

            //GetPassengerCount
            pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)(base_vehicle))+synergy_offsets.getpassengercount_offset));
            uint32_t passengers = pDynamicOneArgFunc(base_vehicle);

            for(uint32_t i = 0; i < passengers; i++)
            {
                pDynamicTwoArgFunc = (pTwoArgProt)(*(uint32_t*)(*(uint32_t*)(iserver_vehicle)));
                uint32_t passenger = pDynamicTwoArgFunc(iserver_vehicle, i);

                if(IsEntityValid(passenger))
                {
                    if(passenger == player)
                        return i;
                }
            }
        }
        else
            return -1;
    }

    ConsolePrint("Failed to get passenger index!");
    return 0;
}

void MakePlayersLeaveVehicles()
{
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(fields.gEntList, player, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(player))
        {
            uint32_t player_vehicle = GetCBaseEntity(*(uint32_t*)(player+synergy_offsets.player_vehicle_offset));

            if(IsEntityValid(player_vehicle))
            {
                uint32_t passenger = GetPassengerIndex(player, player_vehicle);

                if(passenger != -1u)
                {
                    Value* player_value = CreateNewValue((void*)*(uint32_t*)(player+offsets.refhandle_offset));
                    Value* vehicle_value = CreateNewValue((void*)*(uint32_t*)(player_vehicle+offsets.refhandle_offset));
                    Value* passenger_value = CreateNewValue((void*)passenger);
                    Value* steam_id_copy_one = CreateNewValue((void*)*(uint32_t*)(player_vehicle+0x0C));
                    Value* steam_id_copy_two = CreateNewValue((void*)*(uint32_t*)(player_vehicle+0x10));

                    InsertToValuesList(save_player_vehicles_list, player_value, NULL, true, false);
                    InsertToValuesList(save_player_vehicles_list, vehicle_value, NULL, true, false);
                    InsertToValuesList(save_player_vehicles_list, passenger_value, NULL, true, false);
                    InsertToValuesList(save_player_vehicles_list, steam_id_copy_one, NULL, true, false);
                    InsertToValuesList(save_player_vehicles_list, steam_id_copy_two, NULL, true, false);
                }

                Vector emptyVector;

                //remove steam ownership!
                *(uint32_t*)(player_vehicle+0x0C) = 0;
                *(uint32_t*)(player_vehicle+0x10) = 0;

                //LeaveVehicle
                pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*) ((*(uint32_t*)(player))+synergy_offsets.leavevehicle_offset) );
                pDynamicThreeArgFunc(player, (uint32_t)&emptyVector, (uint32_t)&emptyVector);
            }
        }
    }
}

void EnterVehicles(ValueList vehi_list)
{
    pThreeArgProt pDynamicThreeArgFunc;
    Value* first_player = *vehi_list;

    while(first_player && first_player->nextVal)
    {
        uint32_t player = GetCBaseEntity((uint32_t)first_player->value);
        uint32_t vehicle = GetCBaseEntity((uint32_t)first_player->nextVal->value);

        if(IsEntityValid(player) && IsEntityValid(vehicle))
        {
            uint32_t passenger = (uint32_t)first_player->nextVal->nextVal->value;
            uint32_t steam_id_copy_one = (uint32_t)first_player->nextVal->nextVal->nextVal->value;
            uint32_t steam_id_copy_two = (uint32_t)first_player->nextVal->nextVal->nextVal->nextVal->value;

            ConsolePrint("Vehicle Entered! passenger [%d]", passenger);

            *(uint32_t*)(vehicle+0x0C) = steam_id_copy_one;
            *(uint32_t*)(vehicle+0x10) = steam_id_copy_two;

            //EnterVehicle
            pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*) ((*(uint32_t*)(player))+synergy_offsets.entervehicle_offset) );
            pDynamicThreeArgFunc(player, *(uint32_t*)(vehicle+synergy_offsets.iserver_vehicle_offset), passenger);
        }

        Value* nextPlayer = first_player->nextVal->nextVal->nextVal->nextVal->nextVal;

        free(first_player->nextVal->nextVal->nextVal->nextVal);
        free(first_player->nextVal->nextVal->nextVal);
        free(first_player->nextVal->nextVal);
        free(first_player->nextVal);
        free(first_player);

        first_player = nextPlayer;
    }

    *vehi_list = NULL;
}

// SE_SDK2013
#endif