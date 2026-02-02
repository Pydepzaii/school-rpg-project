// FILE: src/debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include "raylib.h"
#include "map.h"
#include "player.h"
#include "npc.h"

// Tool 1: Map (Phím 0)
void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount);

// Tool 2: Menu (Phím =)
void Debug_RunMenuTool(); 

// Check xem menu tool có đang bật không (để vẽ viền đỏ nút)
bool IsMenuDebugActive(); 

// [NEW] Hàm cưỡng chế tắt Tool Menu (Dùng khi chuyển cảnh vào game)
void Debug_ForceCloseMenuTool();

#endif