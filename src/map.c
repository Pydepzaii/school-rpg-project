#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//hàm đọc file map_date(debug support)
void LoadMapDataFromFile(GameMap *map, const char *fileName) {
    FILE *f = fopen(fileName, "r");
    if (f == NULL) {
        printf(">> [WARNING] Khong tim thay file data: %s\n", fileName);
        return;
    }

    char type[20]; 
    // Doc tung dong: WALL hoac PROP
    while (fscanf(f, "%s", type) != EOF) {
        // 1. DOC TUONG: WALL x y w h
        if (strcmp(type, "WALL") == 0) { 
            float x, y, w, h;
            fscanf(f, "%f %f %f %f", &x, &y, &w, &h);
            
            if (map->wallCount < MAX_MAP_WALLS) {
                map->walls[map->wallCount++] = (Rectangle){x, y, w, h};
            }
        }
        // 2. DOC PROP: PROP sx sy sw sh px py originY
        else if (strcmp(type, "PROP") == 0) {
            float sx, sy, sw, sh, px, py, originY;
            fscanf(f, "%f %f %f %f %f %f %f", &sx, &sy, &sw, &sh, &px, &py, &originY);
            
            if (map->propCount < MAX_MAP_PROPS) {
                map->props[map->propCount++] = (GameProp){
                    (Rectangle){sx, sy, sw, sh}, // Source Rec
                    (Vector2){px, py},           // Position
                    originY,                     // Origin Y
                    &map->layerTexture           // Luon dung Texture Layer
                };
            }
        }
    }
    fclose(f);
    printf(">> [INFO] Loaded Data: %s (W: %d, P: %d)\n", fileName, map->wallCount, map->propCount);
}

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
             map->layerTexture = LoadTexture("resources/game_map/map1/layer.png");
             LoadMapDataFromFile(map, "resources/map_data/map_1.txt");
            break;
        case MAP_NHA_VO:
             map->texture = LoadTexture("resources/game_map/map2/nhavo.png");
             map->layerTexture = LoadTexture("resources/game_map/map2/layer.png");
             LoadMapDataFromFile(map, "resources/map_data/map_2.txt");
            break;
        case MAP_THU_VIEN:
            map->texture = LoadTexture("resources/game_map/map3/thuvien.png");
             map->layerTexture = LoadTexture("resources/game_map/map3/layer.png");
            LoadMapDataFromFile(map, "resources/map_data/map_3.txt");
            break;
        case MAP_NHA_AN:
            map->texture = LoadTexture("resources/game_map/map4/cangtin.png");
             map->layerTexture = LoadTexture("resources/game_map/map4/layer.png");
            LoadMapDataFromFile(map, "resources/map_data/map_4.txt");
            break;
         case MAP_BETA:
            map->texture = LoadTexture("resources/game_map/map5/beta.png");
             map->layerTexture = LoadTexture("resources/game_map/map5/layer.png");
            LoadMapDataFromFile(map, "resources/map_data/map_5.txt");
            break;
         case MAP_LAB:
            map->texture = LoadTexture("resources/game_map/map6/phonglab.png");
             map->layerTexture = LoadTexture("resources/game_map/map6/layer.png");
            LoadMapDataFromFile(map, "resources/map_data/map_6.txt");
            break;
        //testMap không sử dụng trong bản chính
        case MAP_DEN:
            map->texture = LoadTexture("resources/game_map/test/wibu.png");
            break;

        case MAP_TRANG:
            map->texture = LoadTexture("resources/game_map/test/wibu2.png");
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