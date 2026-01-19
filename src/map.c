#include "map.h"
#include <stdio.h>

void LoadMap(GameMap *map, int mapID) {
    // Nếu đang có map cũ thì xóa đi trước khi load map mới
    if (map->texture.id > 0) UnloadTexture(map->texture);

    map->currentMapID = mapID;
    map->wallCount = 0; // Reset tường
    map->scale = 2.0f;

    switch (mapID) {
        case MAP_THU_VIEN:
            map->texture = LoadTexture("resources/thuvien.png");
            // -- Tạo tường cho Thư Viện --
            // Tường trái
           // map->walls[0] = (Rectangle){0, 0, 50, 450}; 
            // Cái bàn ở giữa (Vị trí X, Y, Rộng, Cao) - Bạn phải tự căn chỉnh
            //map->walls[1] = (Rectangle){300, 250, 200, 80}; 
            map->wallCount = 0; 
            break;

        case MAP_NHA_AN:
            // map->texture = LoadTexture("resources/nhaan.png");
            // Thêm tường nhà ăn...
            break;
            
        // Thêm các case khác...
        default:
            break;
    }
}

void DrawMap(GameMap *map) {
    DrawTextureEx(map->texture, (Vector2){0, 0}, 0.0f, map->scale, WHITE);
}

void DrawMapDebug(GameMap *map) {
    // Vẽ khung đỏ quanh các bức tường để dễ kiểm tra
    for (int i = 0; i < map->wallCount; i++) {
        DrawRectangleLinesEx(map->walls[i], 2.0f, RED);
    }
}

void UnloadMap(GameMap *map) {
    UnloadTexture(map->texture);
}