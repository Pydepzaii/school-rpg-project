// FILE: src/save_system.h
#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "raylib.h"
#include "player.h" // Để lấy struct PlayerStats

// Định nghĩa file lưu
#define SAVE_FILE_NAME "save_data.dat"

// Cấu trúc dữ liệu sẽ được ghi xuống ổ cứng
typedef struct {
    int mapID;              // Đang ở map nào?
    Vector2 playerPos;      // Đang đứng ở đâu?
    PlayerStats stats;      // Máu, chỉ số...
    // Sau này có thể thêm: int gold, int questStep, v.v.
} SaveData;

// Hàm chức năng
void Game_Save(int mapID, Vector2 pos, PlayerStats stats);
bool Game_Load(int *mapID, Vector2 *pos, PlayerStats *stats);
bool Game_HasSaveFile(); // Kiểm tra xem có file save để hiện nút Continue không

#endif