// FILE: src/debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include "raylib.h"
#include "map.h"
#include "player.h"
#include "npc.h"

// Tool 1: Map (Phím 0)
void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount);

// Tool 2: Menu (Phím =) [UPDATED]
void Debug_RunMenuTool(); 

// Check xem menu tool có đang bật không (để vẽ viền đỏ nút)
bool IsMenuDebugActive(); 

// [NEW] Hàm cưỡng chế tắt Tool Menu (Dùng khi chuyển cảnh vào game)
void Debug_ForceCloseMenuTool();

// Tool 3: Prop (Phím P)
void Debug_RunPropTool(GameMap *map);

// Tool 4: Dialog Editor (Bật bằng Shift + D khi đứng gần NPC)
void Debug_OpenDialogTool(Npc* targetNpc);
void Debug_CloseDialogTool();
void Debug_RunDialogTool(); // Hàm vẽ UI
bool IsDialogDebugActive(); // Dùng để chặn nhân vật di chuyển khi đang sửa thoại

#endif