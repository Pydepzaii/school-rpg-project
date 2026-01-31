#include "map.h"
#include <stdio.h>

void LoadMap(GameMap *map, int mapID) {
    // 1. Cleanup Map Cũ
    // [GIẢI THÍCH]: Luôn phải unload texture cũ trước khi load map mới để tránh tràn RAM.
    if (map->texture.id > 0) UnloadTexture(map->texture);

    // 2. Setup Map Mới
    map->currentMapID = mapID;
    map->wallCount = 0; 
    map->scale = 2.0f;  // [CONFIG] Global Map Scale

    // 3. Load Data riêng từng Map
    switch (mapID) {
        case MAP_THU_VIEN:
            map->texture = LoadTexture("resources/thuvien.png");
            
            // [COLLISION DATA]
            // Copy output từ chế độ Debug (Phím 0 -> Vẽ -> Console) dán vào đây.
            // Format: (Rectangle){ x, y, width, height }
            map->walls[map->wallCount++] = (Rectangle){ 472, 113, 189, 118 };
            map->walls[map->wallCount++] = (Rectangle){ 286, 356, 120, 63 };
            break;
        //testMap không sử dụng trong bản chính
        case MAP_DEN:
             // Tạm dùng lại ảnh thư viện nếu chưa có ảnh nền đen
            map->texture = LoadTexture("resources/wibu.png");
            break;

        case MAP_TRANG:
            map->texture = LoadTexture("resources/wibu2.png");
            break;

        case MAP_NHA_AN:
            // TODO: Load texture nhà ăn và tường
            // [THỪA]: Hiện tại case này trống, nếu load map Nhà Ăn sẽ lỗi texture (màn hình đen/hồng).
            break;
            
        default:
            break;
    }
}

void DrawMap(GameMap *map) {
    // Vẽ texture background, scale theo thông số đã set
    DrawTextureEx(map->texture, (Vector2){0, 0}, 0.0f, map->scale, WHITE);
}

// Debug Visualizer: Vẽ đè các ô vuông đỏ lên vị trí có tường va chạm
void DrawMapDebug(GameMap *map) {
    for (int i = 0; i < map->wallCount; i++) {
        DrawRectangleRec(map->walls[i], Fade(RED, 0.6f)); // Fill
        DrawRectangleLinesEx(map->walls[i], 3.0f, RED);   // Border
    }
}

void UnloadMap(GameMap *map) {
    UnloadTexture(map->texture);
}