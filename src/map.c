#include "map.h"
#include <stdio.h>

void LoadMap(GameMap *map, int mapID) {
    // BƯỚC 1: Dọn dẹp map cũ
    if (map->texture.id > 0) UnloadTexture(map->texture);

    // BƯỚC 2: Thiết lập thông số map mới
    map->currentMapID = mapID;
    map->wallCount = 0; 
    map->scale = 2.0f;  

    // BƯỚC 3: Nạp dữ liệu riêng
    switch (mapID) {
        case MAP_THU_VIEN:
            map->texture = LoadTexture("resources/thuvien.png");
            
            // --- KHU VỰC ĐẶT TƯỜNG ---
            // Paste code bạn copy từ Terminal vào đây:
            map->walls[map->wallCount++] = (Rectangle){ 183, 204, 94, 40 };
            map->walls[map->wallCount++] = (Rectangle){ 168, 314, 154, 36 };
            break;

        case MAP_NHA_AN:
            break;
            
        default:
            break;
    }
}

void DrawMap(GameMap *map) {
    DrawTextureEx(map->texture, (Vector2){0, 0}, 0.0f, map->scale, WHITE);
}

// HÀM NÀY QUẢN LÝ MÀU CỦA TƯỜNG CŨ (ĐÃ LƯU)
void DrawMapDebug(GameMap *map) {
    for (int i = 0; i < map->wallCount; i++) {
        // Tô nền ĐỎ mờ
        DrawRectangleRec(map->walls[i], Fade(RED, 0.6f));
        // Vẽ viền ĐỎ đậm
        DrawRectangleLinesEx(map->walls[i], 3.0f, RED);
    }
}

void UnloadMap(GameMap *map) {
    UnloadTexture(map->texture);
}