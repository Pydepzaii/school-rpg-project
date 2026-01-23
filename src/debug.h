// FILE: src/debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include "raylib.h"
#include "map.h"
#include "player.h" // [MỚI] Cần struct Player
#include "npc.h"    // [MỚI] Cần struct Npc

// Hàm này làm 2 việc:
// 1. Kiểm tra phím 0 để Bật/Tắt hiển thị tường cũ + Hitbox nhân vật
// 2. Cho phép kéo chuột vẽ tường mới
// [CẬP NHẬT] Thêm tham số player, npcList, npcCount
void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount);

#endif