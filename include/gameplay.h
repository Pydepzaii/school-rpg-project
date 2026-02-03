#ifndef GAMEPLAY_H
#define GAMEPLAY_H
<<<<<<< Updated upstream
=======

#include "raylib.h" 
#include "player.h"
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream

=======
// Hàm lấy dữ liệu để lưu game
void Gameplay_GetSaveInfo(int *mapID, Vector2 *pos, PlayerStats *stats);

// Hàm nạp dữ liệu chỉ số nhân vật khi load game
void Gameplay_LoadStats(PlayerStats stats);
>>>>>>> Stashed changes
#endif