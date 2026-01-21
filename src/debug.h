// FILE: src/debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include "raylib.h"
#include "map.h" // Cần file này để lấy thông tin Map

// Hàm này làm 2 việc:
// 1. Kiểm tra phím 0 để Bật/Tắt hiển thị tường cũ
// 2. Cho phép kéo chuột vẽ tường mới
void Debug_UpdateAndDraw(GameMap *map);

#endif