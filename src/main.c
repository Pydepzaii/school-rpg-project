#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include <stdio.h> // Debug

int main() {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(FPS);

    // --- 1. KHỞI TẠO PLAYER (CHỌN CLASS) ---
    // Sau này có thể làm Menu chọn, giờ ta chọn cứng CLASS_STUDENT
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 

    // --- 2. KHỞI TẠO MAP ---
    GameMap currentMap;
    currentMap.texture.id = 0; // Đánh dấu để biết chưa load
    LoadMap(&currentMap, MAP_THU_VIEN); // Bắt đầu ở thư viện

    // --- 3. KHỞI TẠO HỆ THỐNG NPC (MẢNG) ---
    Npc npcList[MAX_NPCS];
    int npcCount = 0;

    // NPC 1: Cô Đầu Bếp (Chỉ xuất hiện ở MAP_THU_VIEN - ví dụ demo)
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){500, 300}, "Co Dau Bep");
    npcCount++;

    // NPC 2: Ví dụ thêm một bạn học sinh khác (Thầy giáo...)
    // InitNpc(&npcList[1], MAP_SAN_TRUONG, "resources/thaygiao.png", (Vector2){200, 200}, "Thay Giao");
    // npcCount++;

    while (!WindowShouldClose()) {
        // --- XỬ LÝ LOGIC ---
        
        // (Cheat) Nhấn F1, F2 để test chuyển map
        if (IsKeyPressed(KEY_F1)) LoadMap(&currentMap, MAP_THU_VIEN);
        if (IsKeyPressed(KEY_F2)) LoadMap(&currentMap, MAP_NHA_AN);

        // Update Player (Truyền toàn bộ dữ liệu va chạm vào)
        UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);

        // Update NPC (Chỉ update NPC nào đang ở cùng map với người chơi)
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == currentMap.currentMapID) {
                UpdateNpc(&npcList[i]);
            }
        }

        // --- VẼ HÌNH ---
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // 1. Vẽ Map
            DrawMap(&currentMap);
            // DrawMapDebug(&currentMap); // Bỏ comment dòng này để xem tường va chạm

            // 2. Vẽ NPC (Chỉ vẽ ai đang ở map này)
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) {
                    DrawNpc(&npcList[i]);
                }
            }

            // 3. Vẽ Player
            DrawPlayer(&mainCharacter);

            // 4. UI Thông tin
            DrawText("F1: Thu Vien | F2: Nha An (Trong)", 10, 10, 20, BLACK);
            DrawText(TextFormat("HP: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp), 10, 40, 20, RED);

        EndDrawing();
    }

    // --- DỌN DẸP ---
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    for (int i = 0; i < npcCount; i++) UnloadNpc(&npcList[i]);
    
    CloseWindow();
    return 0;
}