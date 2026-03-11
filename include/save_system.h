// FILE: src/save_system.h
#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "raylib.h"
#include "player.h" 

#define SAVE_FILE_NAME "save_data.dat"

// Cấu trúc dữ liệu sẽ được ghi xuống ổ cứng
typedef struct {
    int mapID;              // Đang ở map nào?
    Vector2 playerPos;      // Đang đứng ở đâu?
    
    // -- TOÀN BỘ CHỈ SỐ CỦA PLAYER --
    PlayerClass pClass;     // Nghề nghiệp (Dùng để load lại đúng hình ảnh)
    PlayerStats stats;      // Máu, Thể lực, Tốc độ, storyProgress...
    CBC_Stats cbcStats;     // Chỉ số Hỏi Đáp
    
    // (Sau này nếu làm chức năng lưu túi đồ, ta sẽ thêm mảng Inventory vào đây)
} SaveData;

// Truyền hẳn con trỏ Player vào cho gọn
void Game_Save(int mapID, Vector2 pos, Player *player);
bool Game_Load(int *mapID, Vector2 *pos, Player *player);
bool Game_HasSaveFile(); 

#endif