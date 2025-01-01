#include "extension.h"
#include "util.h"
#include "core.h"

uint32_t last_ragdoll_gib;
int ragdoll_breaking_gib_counter;
bool is_currently_ragdoll_breaking;

void InitCoreBlackMesa()
{
    //Populate our libraries

    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/bms/bin/server_srv.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/dedicated_srv.so");
}

uint32_t GetCBaseEntityBlackMesa(uint32_t EHandle)
{
    uint32_t shift_right = EHandle >> 0x0D;
    uint32_t disassembly = EHandle & 0x1FFF;
    disassembly = disassembly << 0x4;
    disassembly = fields.CGlobalEntityList + disassembly;

    if( ((*(uint32_t*)(disassembly+0x08))) == shift_right)
    {
        uint32_t CBaseEntity = *(uint32_t*)(disassembly+0x04);
        return CBaseEntity;
    }

    return 0;
}

void PopulateHookExclusionListsBlackMesa()
{
    hook_exclude_list_base[0] = server_srv;
    hook_exclude_list_offset[0] = 0x00A92201;
}

void CheckForLocation()
{
    uint32_t current_map = fields.sv+0x11;

    if(strcmp((char*)current_map, "bm_c2a3a") != 0)
    {
        //rootconsole->ConsolePrint("Location fix disabled!");
        return;
    }

    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(fields.CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        bool in_area = false;
        uint32_t player_abs = player+0x294;
        //rootconsole->ConsolePrint("[%f] [%f] [%f]", *(float*)(player_abs), *(float*)(player_abs+0x4), *(float*)(player_abs+0x8));

        Vector* trigger_vecMinsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMinsAbs->x = 1314.0;
        trigger_vecMinsAbs->y = 106.0;
        trigger_vecMinsAbs->z = -1349.0;

        Vector* trigger_vecMaxsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMaxsAbs->x = 2282.0;
        trigger_vecMaxsAbs->y = 947.0;
        trigger_vecMaxsAbs->z = -1102.0;

        if(trigger_vecMinsAbs->x <= *(float*)(player_abs) && *(float*)(player_abs) <= trigger_vecMaxsAbs->x)
        {
            if(trigger_vecMinsAbs->y <= *(float*)(player_abs+0x4) && *(float*)(player_abs+0x4) <= trigger_vecMaxsAbs->y)
            {
                if(trigger_vecMinsAbs->z <= *(float*)(player_abs+0x8) && *(float*)(player_abs+0x8) <= trigger_vecMaxsAbs->z)
                {
                    uint32_t collision_property = player+offsets.collision_property_offset;
                    uint16_t current_flags = *(uint16_t*)(collision_property+0x3C);

                    functions.SetSolidFlags(collision_property, 4);
                    //rootconsole->ConsolePrint("bad area!");
                    in_area = true;
                }
            }
        }

        if(!in_area)
        {
            uint32_t collision_property = player+offsets.collision_property_offset;
            uint16_t current_flags = *(uint16_t*)(collision_property+0x3C);

            functions.SetSolidFlags(collision_property, 16);
        }

        free(trigger_vecMinsAbs);
        free(trigger_vecMaxsAbs);
    }
}