#include "player.h"

void InitPlayer(Player *player) {
    // Load ảnh từ thư mục resources
    // Lưu ý: Dùng đường dẫn "resources/..." để máy nào cũng hiểu
    player->texture = LoadTexture("resources/main_hocba.png"); 
    player->position.x = 400; // Giữa màn hình (tạm tính)
    player->position.y = 225;
    player->speed = 5.0f;
}

void UpdatePlayer(Player *player) {
    // Xử lý Logic: Chỉ tính toán vị trí, không vẽ vời gì ở đây
    if (IsKeyDown(KEY_RIGHT)) player->position.x += player->speed;
    if (IsKeyDown(KEY_LEFT)) player->position.x -= player->speed;
    if (IsKeyDown(KEY_UP)) player->position.y -= player->speed;
    if (IsKeyDown(KEY_DOWN)) player->position.y += player->speed;
}

void DrawPlayer(Player *player) {
    // Xử lý Đồ họa: Chỉ vẽ
    DrawTextureV(player->texture, player->position, WHITE);
}

void UnloadPlayer(Player *player) {
    UnloadTexture(player->texture); // Giải phóng RAM
}