// FILE: src/main.c
#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include "debug.h" // <--- Đã thêm file debug vào đây
#include <stdio.h> 

int main() {
    // 1. SETUP CỬA SỔ
    SetConfigFlags(FLAG_WINDOW_HIGHDPI); // Hỗ trợ màn hình độ phân giải cao
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(FPS); // Khóa FPS lại (thường là 60)

    // 2. KHỞI TẠO DỮ LIỆU (INIT)
    // - Tạo Player
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 

    // - Tạo Map
    GameMap currentMap;
    currentMap.texture.id = 0; // Đánh dấu ID = 0 (chưa load gì)
    LoadMap(&currentMap, MAP_THU_VIEN); // Load map đầu tiên

    // - Tạo NPC (Dùng mảng để quản lý nhiều NPC)
    Npc npcList[MAX_NPCS];
    int npcCount = 0;

    // NPC 01: Cô Đầu Bếp
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){500, 300}, "Co Dau Bep");
    npcCount++;

    // 3. VÒNG LẶP GAME (GAME LOOP)
    while (!WindowShouldClose()) {
        // --- PHẦN LOGIC (UPDATE) ---
        // (Tính toán mọi thay đổi trước khi vẽ)
        
        // Debug: Phím tắt chuyển map nhanh
        if (IsKeyPressed(KEY_F1)) LoadMap(&currentMap, MAP_THU_VIEN);
        if (IsKeyPressed(KEY_F2)) LoadMap(&currentMap, MAP_NHA_AN);

        // Upda te Người chơi (Truyền map và npc vào để check va chạm)
        UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);

        // Update NPC: Chỉ update những NPC đang ở cùng map với người chơi
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == currentMap.currentMapID) {
                UpdateNpc(&npcList[i]);
            }
        }

        // --- PHẦN VẼ (DRAW) ---
        BeginDrawing();
            ClearBackground(RAYWHITE); // Xóa màn hình cũ
            
            // Lớp 1: Vẽ nền Map
            DrawMap(&currentMap);
            // (Phần vẽ tường Debug cũ đã được chuyển sang hàm Debug_UpdateAndDraw bên dưới)
            
            // Lớp 2: Vẽ NPC
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) {
                    DrawNpc(&npcList[i]);
                }
            }

            // Lớp 3: Vẽ Player (Vẽ sau cùng để đè lên trên nền)
            DrawPlayer(&mainCharacter);

            // --- [TOOL DEBUG] ---
            // Gọi hàm này để: 
            // 1. Ấn phím 0 thì hiện tường đỏ
            // 2. Kéo chuột thì hiện tường xanh để lấy tọa độ
            Debug_UpdateAndDraw(&currentMap); 

            // Lớp 4: UI (Giao diện người dùng)
            DrawText("F1: Thu Vien | F2: Nha An (Trong)", 10, 10, 20, BLACK);
            DrawText(TextFormat("HP: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp), 10, 40, 20, RED);

        EndDrawing();
    }

    // 4. DỌN DẸP (UNLOAD)
    // Giải phóng RAM trước khi tắt
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    for (int i = 0; i < npcCount; i++) UnloadNpc(&npcList[i]);
    
    CloseWindow();
    return 0;
}