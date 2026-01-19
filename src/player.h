#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

// Struct chứa mọi thông tin của nhân vật
// Giúp quản lý code gọn gàng, truyền 1 biến player là có đủ thông tin
typedef struct {
    Vector2 position; // Vị trí x, y
    Texture2D texture; // Hình ảnh
    float speed;      // Tốc độ
} Player;

// Khai báo các hàm sẽ dùng (chưa viết code cụ thể ở đây)
void InitPlayer(Player *player);        // Khởi tạo nhân vật
void UpdatePlayer(Player *player);      // Tính toán di chuyển (Logic)
void DrawPlayer(Player *player);        // Vẽ nhân vật (Đồ họa)
void UnloadPlayer(Player *player);      // Dọn dẹp bộ nhớ khi tắt game

#endif