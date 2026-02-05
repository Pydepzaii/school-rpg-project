#include "map.h"
#include <stdio.h>

void LoadMap(GameMap *map, int mapID) {
    // 1. Cleanup Map Cũ
    // [GIẢI THÍCH]: Luôn phải unload texture cũ trước khi load map mới để tránh tràn RAM.
    if (map->texture.id > 0) UnloadTexture(map->texture);
    if (map->layerTexture.id > 0) {
        UnloadTexture(map->layerTexture);
        map->layerTexture.id = 0; // Reset ID về 0 để an toàn
    }
    // 2. Setup Map Mới
    map->currentMapID = mapID;
    map->wallCount = 0; 
    map->propCount = 0;
    map->scale = 2.0f;  // [CONFIG] Global Map Scale

    // 3. Load Data riêng từng Map
    switch (mapID) {
        case MAP_TOA_ALPHA:
             map->texture = LoadTexture("resources/game_map/map1/alpha.png");
             
            break;
        case MAP_NHA_VO:
             map->texture = LoadTexture("resources/game_map/map2/nhavo.png");
             map->layerTexture = LoadTexture("resources/game_map/map2/layer.png");
             //render máy bán nước
            map->props[map->propCount++] = (GameProp){ (Rectangle){167, 3, 28, 44}, (Vector2){334, 5}, 89, &map->layerTexture };
             map->walls[map->wallCount++] = (Rectangle){ 334, 84, 54, 10 };
             //render cột 1
             map->props[map->propCount++] = (GameProp){ (Rectangle){254, 5, 8, 76}, (Vector2){507, 10}, 152, &map->layerTexture };
             map->walls[map->wallCount++] = (Rectangle){ 508, 153, 14, 7 };
            break;
        case MAP_THU_VIEN:
            map->texture = LoadTexture("resources/game_map/map3/thuvien.png");
            
            // [COLLISION DATA]
            // Copy output từ chế độ Debug (Phím 0 -> Vẽ -> Console) dán vào đây.
            // Format: (Rectangle){ x, y, width, height }
            map->walls[map->wallCount++] = (Rectangle){ 472, 113, 189, 118 };
            map->walls[map->wallCount++] = (Rectangle){ 286, 356, 120, 63 };
            break;
        //testMap không sử dụng trong bản chính
        case MAP_DEN:
            map->texture = LoadTexture("resources/game_map/test/wibu.png");
            break;

        case MAP_TRANG:
            map->texture = LoadTexture("resources/game_map/test/wibu2.png");
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
    // [NEW] Vẽ luôn lớp Layer đè lên sàn (Vẽ phẳng, chưa có độ sâu)
    // Cái này làm nền để khi nhân vật đi "trước" vật thể thì vẫn thấy vật thể nằm dưới chân
    if (map->layerTexture.id > 0) {
        DrawTextureEx(map->layerTexture, (Vector2){0, 0}, 0.0f, map->scale, WHITE);
    }
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
    if (map->layerTexture.id > 0) UnloadTexture(map->layerTexture);
}