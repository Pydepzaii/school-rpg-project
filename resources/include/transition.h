// FILE: src/transition.h
#ifndef TRANSITION_H
#define TRANSITION_H

#include "raylib.h"
#include "map.h"
#include "player.h" // Cần để reset vị trí nhân vật nếu cần

// Gọi hàm này để bắt đầu quá trình chuyển cảnh sang Map mới
void Transition_StartToMap(int mapID, Vector2 newPlayerPos);

// Gọi hàm này để chuyển sang Menu (ví dụ từ Title vào Game)
// (Bạn có thể mở rộng sau, hiện tại ta tập trung vào Map trước)

// Hai hàm này đặt vào vòng lặp chính (Main Loop)
void Transition_Update(GameMap *currentMap, Player *player);
void Transition_Draw();

// Hàm tiện ích để biết game có đang chuyển cảnh không (để khóa nút bấm)
bool Transition_IsActive();
void Transition_StartExit();
#endif