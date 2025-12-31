#ifdef SE_SDK2013

#include "extension.h"
#include "util.h"

#include "synergy/core.h"

synergy_game_fields synergy_fields;
synergy_game_offsets synergy_offsets;
synergy_game_functions synergy_functions;

uint32_t synergy_srv;
uint32_t synergy_srv_size;

bool sdktools_passed;

int save_frames;
bool savegame;
bool savegame_autosave;
bool savegame_internal;
bool saved_game_once;
bool disable_player_restore;

ValueList save_player_vehicles_list;

void InitCore()
{
    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/synergy/bin/server_srv.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/dedicated_srv.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[4] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[4], 1024, "%s", "/synergy/bin/synergy_srv.so");

    our_libraries[5] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[5], 1024, "%s", "/extensions/sdktools.ext.2.sdk2013.so");
}

bool IsAllowedToPatchSdkTools(uint32_t lib_base, uint32_t lib_size)
{
    uint32_t lib_integrity_chk_addr = lib_base + 0x00057919;
    uint32_t str_len = 11;

    bool integrity_chk = (lib_integrity_chk_addr + str_len) <= (lib_base + lib_size);

    if(integrity_chk)
    {
        char* ext_ver = (char*)lib_integrity_chk_addr;

        if(strcmp(ext_ver, "1.10.0.6503") == 0)
        {
            rootconsole->ConsolePrint("\nSDKTools Memory Integrity Passed!\n");
            return true;
        }
    }

    return false;
}

void PopulateHookExclusionLists()
{
    hook_exclude_list_base[0] = server_srv;
    hook_exclude_list_offset[0] = 0x008A0E2F;

    hook_exclude_list_base[1] = vphysics_srv;
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

        rootconsole->ConsolePrint("[%s] Attempted to free leaks from an empty leaked resources list!", listName);
        return 0;
    }

    int total_items = ValueListItems(leakList, NULL);

    while(leak)
    {
        Value* detachedValue = leak->nextVal;

        //rootconsole->ConsolePrint("[%s] FREED MEMORY LEAK WITH REF: [%X]", listName, leak->value);
        free(leak->value);
        free(leak);

        leak = detachedValue;
    }

    *leakList = NULL;

    rootconsole->ConsolePrint("FREED [%d] memory allocations", total_items);

    if(destroy)
    {
        free(leakList);
        leakList = NULL;
    }

    return total_items;
}

void ReleaseLeakedPackedEntities(uint32_t snapManager)
{
    pTwoArgProt pDynamicTwoArgFunc;

    int freed_leaks = 0;

    for(int i = 0; i < 2048; i++)
    {
        //rootconsole->ConsolePrint("trying [%d] ent", i);
        uint32_t computed_ref = *(uint32_t*)(snapManager+i*4+0x70);

        if(computed_ref != 0)
        {
            //RemoveEntityReference
            pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x002DA520);
            pDynamicTwoArgFunc(snapManager, computed_ref);

            *(uint32_t*)(snapManager+i*4+0x70) = 0;
            freed_leaks++;
        }
    }

    rootconsole->ConsolePrint("Purged [%d] packed ents!", freed_leaks);
}

void ExtensionUpdateOnRemove(uint32_t arg0)
{
    Vector emptyVector;
    pThreeArgProt pDynamicThreeArgFunc;
    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(strcmp(classname, "player") == 0)
    {
        rootconsole->ConsolePrint("left vehicle before player leaves!");

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
                rootconsole->ConsolePrint("Tried killing player but was protected & respawned!");
    
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
            rootconsole->ConsolePrint("WARNING: Removing [%s] while a save file is being made!", classname);
            functions.RemoveInsta(object);
            return;
        }

        if(slow)    functions.RemoveNormal(object);
        else        functions.RemoveInsta(object);

        return;
    }

    uint32_t first_return = ((uint32_t)__builtin_return_address(0)) - server_srv;
    uint32_t second_return = ((uint32_t)__builtin_return_address(1)) - server_srv;
    uint32_t third_return = ((uint32_t)__builtin_return_address(2)) - server_srv;
    uint32_t fourth_return = ((uint32_t)__builtin_return_address(3)) - server_srv;

    rootconsole->ConsolePrint("Failed to validate entity 1:%p 2:%p 3:%p 4:%p", first_return, second_return, third_return, fourth_return);
    if(crash_server) exit(EXIT_FAILURE);
}

void SaveGame_Extension()
{
    pOneArgProtFastCall pDynamicFastCallOneArgFunc;

    save_frames = 0;
    
    MakePlayersLeaveVehicles();
    FixCars();

    functions.CleanupDeleteList(0);

    savegame_autosave = true;

    pDynamicFastCallOneArgFunc = (pOneArgProtFastCall)(synergy_functions.Autosave_Silent);
    pDynamicFastCallOneArgFunc(0);

    savegame_autosave = false;

    functions.CleanupDeleteList(0);
    
    saved_game_once = true;
}

void FixCars()
{
    uint32_t mainEnt = 0;

    while((mainEnt = functions.FindEntityByClassname(fields.gEntList, mainEnt, (uint32_t)"*")) != 0)
    {
        if(IsEntityValid(mainEnt))
        {
            char* clsname = (char*) ( *(uint32_t*)(mainEnt+offsets.classname_offset) );
        
            if(strcmp(clsname, "prop_vehicle_jeep") != 0 && strcmp(clsname, "prop_vehicle_mp") != 0 && strcmp(clsname, "prop_vehicle_airboat") != 0)
                continue;
    
            char* model = (char*)(*(uint32_t*)(mainEnt+synergy_offsets.vehicle_model_offset));
            char* script = (char*)(*(uint32_t*)(mainEnt+synergy_offsets.vehicle_script_offset));
    
            bool fixed_model = FixSlashes(model);
            bool fixed_script = FixSlashes(script);
    
            if(fixed_model)
            {
                rootconsole->ConsolePrint("FIXED_MODEL_NAME: [%s]", model);
            }
    
            if(fixed_script)
            {
                rootconsole->ConsolePrint("FIXED_SCRIPT_NAME: [%s]", script);
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

        if
        (
        (vehicle_classname && strcmp(vehicle_classname, "prop_vehicle_airboat")) == 0
            ||
        (vehicle_classname && strcmp(vehicle_classname, "prop_vehicle_mp")) == 0
            ||
        (vehicle_classname && strncmp(vehicle_classname, "prop_vehicle_jeep", 17)) == 0
        )
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

    rootconsole->ConsolePrint("Failed to get passenger index!");
    return 0;
}

void MakePlayersLeaveVehicles()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(fields.gEntList, player, (uint32_t)"player")) != 0)
    {
        if(IsEntityValid(player))
        {
            uint32_t player_vehicle = GetCBaseEntity(*(uint32_t*)(player+synergy_offsets.player_vehicle_offset));

            if(IsEntityValid(player_vehicle))
            {
                char* vehicle_classname = (char*)(*(uint32_t*)(player_vehicle+offsets.classname_offset));
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

            rootconsole->ConsolePrint("Vehicle Entered! passenger [%d]", passenger);

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