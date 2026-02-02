#ifndef GAMEPLAY_H
#define GAMEPLAY_H
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

#endif