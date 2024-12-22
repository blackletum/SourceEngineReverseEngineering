#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"

ValueList leakedResourcesSaveRestoreSystem;
ValueList leakedResourcesEdtSystem;

bool sdktools_passed;
bool savegame;

void InitCoreSynergy()
{
    savegame = false;

    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/synergy/bin/server_srv.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/dedicated_srv.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[4] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[4], 1024, "%s", "/extensions/sdktools.ext.2.sdk2013.so");
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

void PopulateHookExclusionListsSynergy()
{
    
}

uint32_t GetCBaseEntitySynergy(uint32_t EHandle)
{
    uint32_t EntityList = fields.CGlobalEntityList;
    uint32_t refHandle = (EHandle & 0xFFF) << 4;

    EHandle = EHandle >> 0x0C;

    if(*(uint32_t*)(EntityList+refHandle+8) == EHandle)
    {
        uint32_t CBaseEntity = *(uint32_t*)(EntityList+refHandle+4);
        return CBaseEntity;
    }

    return 0;
}

void SaveLinkedList(ValueList leakList)
{
    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);

    FILE* list_file = fopen(path, "a");

    if(!list_file && !leakList && !*leakList)
    {
        rootconsole->ConsolePrint("Error saving leaked resources!");
        return;
    }

    char listName[256];
    snprintf(listName, 256, "Unknown List");

    if(leakList == leakedResourcesSaveRestoreSystem)
        snprintf(listName, 256, "leakedResourcesSaveRestoreSystem");
    else if(leakList == leakedResourcesEdtSystem)
        snprintf(listName, 256, "leakedResourcesEdtSystem");

    rootconsole->ConsolePrint("Saving leaked resources list [%s]", listName);

    fprintf(list_file, "%s\n", listName); 

    Value* current = *leakList;

    while(current)
    {        
        fprintf(list_file, "%X\n", (uint32_t)current->value); 
        current = current->nextVal;
    }

    fclose(list_file);
}

void RestoreLinkedLists()
{
    leakedResourcesSaveRestoreSystem = AllocateValuesList();
    leakedResourcesVpkSystem = AllocateValuesList();
    leakedResourcesEdtSystem = AllocateValuesList();

    ValueList currentRestoreList = NULL;

    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);
    FILE* list_file = fopen(path, "r");    

    if(!list_file)
    {
        rootconsole->ConsolePrint("Error restoring leaked resources!");
        return;
    }

    char* file_line = (char*) malloc(1024);
    fgets(file_line, 1024, list_file);

    int ppid_file = strtol(file_line, NULL, 10);

    if(ppid_file != getppid())
    {
        rootconsole->ConsolePrint("Leaked resources were not restored due to parent process not matching!");
        fclose(list_file);
        return;
    }

    while(fgets(file_line, 1024, list_file))
    {
        sscanf(file_line, "%[^\n]s", file_line);
        if(strcmp(file_line, "\n") == 0) 
            continue;

        if(strncmp(file_line, "leakedResourcesSaveRestoreSystem", 32) == 0)
        {
            currentRestoreList = leakedResourcesSaveRestoreSystem;
            continue;
        }
        else if(strncmp(file_line, "leakedResourcesEdtSystem", 24) == 0)
        {
            currentRestoreList = leakedResourcesEdtSystem;
            continue;
        }

        if(!currentRestoreList)
            continue;

        uint32_t parsedRef = strtoul(file_line, NULL, 16);
        Value* leak = CreateNewValue((void*)parsedRef);
        rootconsole->ConsolePrint("Restored leaked reference: [%X]", parsedRef);
        InsertToValuesList(currentRestoreList, leak, NULL, false, true);
    }

    free(file_line);
    fclose(list_file);
}

int ReleaseLeakedMemory(ValueList leakList, bool destroy, uint32_t current_cap, uint32_t allowed_cap, uint32_t free_perc)
{
    if(!leakList)
        return 0;
    
    Value* leak = *leakList;
    char listName[256];
    snprintf(listName, 256, "Unknown List");

    if(leakList == leakedResourcesSaveRestoreSystem)
        snprintf(listName, 256, "Save/Restore Hook");
    else if(leakList == leakedResourcesEdtSystem)
        snprintf(listName, 256, "EDT Hook");

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

    if(destroy)
    {
        //Save references to be freed if extension is reloaded
        SaveLinkedList(leakList);
    }

    if((current_cap < allowed_cap) && !destroy)
        return 0;

    int total_items = ValueListItems(leakList, NULL);
    int free_total_items = (float)free_perc / 100.0 * total_items;
    int has_freed_items = 0;

    while(leak)
    {
        Value* detachedValue = leak->nextVal;

        if(!destroy)
        {
            //rootconsole->ConsolePrint("[%s] FREED MEMORY LEAK WITH REF: [%X]", listName, leak->value);
            free(leak->value);
            has_freed_items++;
        }

        free(leak);
        leak = detachedValue;
        
        if((has_freed_items >= free_total_items) && !destroy)
        {
            //Re-chain the list to point to the first valid value!
            *leakList = leak;
            break;
        }
    }

    rootconsole->ConsolePrint("FREED [%d] memory allocations", has_freed_items);

    if(destroy)
    {
        free(leakList);
        leakList = NULL;
    }

    return has_freed_items;
}

void DestroyLinkedLists()
{
    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, true, 0, 0, 100);
    ReleaseLeakedMemory(leakedResourcesVpkSystem, true, 0, 0, 100);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, true, 0, 0, 100);

    rootconsole->ConsolePrint("---  Linked lists successfully destroyed  ---");
}

void SaveProcessId()
{
    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);

    FILE* list_file = fopen(path, "w");

    if(!list_file)
    {
        rootconsole->ConsolePrint("Error saving leaked resources!");
        return;
    }

    fprintf(list_file, "%d\n", getppid()); 
    fclose(list_file);
}

void InstaKillSynergy(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    uint32_t refHandleInsta = *(uint32_t*)(entity_object+offsets.refhandle_offset);
    char* classname = (char*)( *(uint32_t*)(entity_object+offsets.classname_offset));
    uint32_t cbase_chk = functions.GetCBaseEntity(refHandleInsta);

    if(cbase_chk == 0)
    {
        if(!validate)
        {
            if(classname)
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation! [%s]", classname);
            }
            else
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            }

            cbase_chk = entity_object;
        }
        else
        {
            rootconsole->ConsolePrint("\n\nFailed to verify entity for fast kill [%X]\n\n", (uint32_t)__builtin_return_address(0) - server_srv);
            exit(EXIT_FAILURE);
            return;
        }
    }

    uint32_t isMarked = functions.IsMarkedForDeletion(cbase_chk+offsets.iserver_offset);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_RemoveImmediate(CBaseEntity*)");
        return;
    }

    if((*(uint32_t*)(cbase_chk+offsets.ismarked_offset) & 1) == 0)
    {
        if(*(uint32_t*)(fields.RemoveImmediateSemaphore) == 0)
        {
            // FAST DELETE ONLY

            //if(classname) rootconsole->ConsolePrint("Removed [%s]", classname);

            *(uint32_t*)(cbase_chk+offsets.ismarked_offset) = *(uint32_t*)(cbase_chk+offsets.ismarked_offset) | 1;

            //UpdateOnRemove
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x1AC) );
            pDynamicOneArgFunc(cbase_chk);

            //CALL RELEASE
            uint32_t iServerObj = cbase_chk+offsets.iserver_offset;

            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(iServerObj))+0x10) );
            pDynamicOneArgFunc(iServerObj);
        }
        else
        {
            RemoveEntityNormalSynergy(cbase_chk, validate);
        }
    }
}

void RemoveEntityNormalSynergy(uint32_t entity_object, bool validate)
{
    pOneArgProt pDynamicOneArgFunc;
    
    if(entity_object == 0)
    {
        rootconsole->ConsolePrint("Could not kill entity [NULL]");
        return;
    }

    char* classname = (char*)(*(uint32_t*)(entity_object+offsets.classname_offset));
    uint32_t m_refHandle = *(uint32_t*)(entity_object+offsets.refhandle_offset);
    uint32_t chk_ref = functions.GetCBaseEntity(m_refHandle);

    if(chk_ref == 0)
    {
        if(!validate)
        {
            if(classname)
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation! [%s]", classname);
            }
            else
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            }
            
            chk_ref = entity_object;
        }
    }

    if(chk_ref)
    {   
        if(strcmp(classname, "player") == 0)
        {
            rootconsole->ConsolePrint(EXT_PREFIX "Tried killing player but was protected!");
            return;
        }

        //Check if entity was already removed!
        uint32_t isMarked = functions.IsMarkedForDeletion(chk_ref+offsets.iserver_offset);

        if(isMarked) return;

        //if(classname) rootconsole->ConsolePrint("Removed [%s]", classname);

        //UTIL_Remove(IServerNetworkable*)
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008A0AB0);
        pDynamicOneArgFunc(chk_ref+offsets.iserver_offset);

        //Check if entity was removed!
        isMarked = functions.IsMarkedForDeletion(chk_ref+offsets.iserver_offset);

        if(isMarked)
        {
            if(*(uint32_t*)(fields.RemoveImmediateSemaphore) != 0)
            {
                hooked_delete_counter++;
            }
        }

        return;
    }

    if(classname)
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle] [%s] [%X]", classname, (uint32_t)__builtin_return_address(0) - server_srv);
    }
    else
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle] [%X]", (uint32_t)__builtin_return_address(0) - server_srv);
    }

    exit(EXIT_FAILURE);
}
