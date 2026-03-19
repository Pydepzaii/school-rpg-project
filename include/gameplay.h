#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h" 
#include "player.h"
// --- ĐỊNH NGHĨA CÁC LOẠI ENDING ---
typedef enum {
    ENDING_TRUE = 1,
    ENDING_BAD = 2,
    ENDING_DARK = 3
} EndingType;
//chọn class nhân vật
void Gameplay_SetPlayerClass(int classID);
// Hàm khởi tạo toàn bộ dữ liệu game (Player, Map, NPC...)
void Gameplay_Init();

// Hàm xử lý logic game (Di chuyển, va chạm...)
void Gameplay_Update();

// Hàm vẽ game (Map, Nhân vật, UI...)
void Gameplay_Draw();

// Hàm dọn dẹp bộ nhớ khi thoát game
void Gameplay_Shutdown();
void Gameplay_SaveGame();
void Gameplay_LoadGame();
// --- CÁC HÀM XỬ LÝ ENDING ---
// Truyền loại Ending vào để game biết đường hiện chữ tương ứng
void Gameplay_StartEnding(EndingType type);

// Hàm vẽ riêng cho kỹ xảo Ending và màn hình Credit
void Gameplay_DrawEnding();

// Hàm kiểm tra xem game có đang trong trạng thái Ending không
bool Gameplay_IsEnding();
#endif