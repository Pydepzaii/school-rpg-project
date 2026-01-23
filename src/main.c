// FILE: src/main.c
#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include "debug.h" 
#include "renderer.h" 
#include "interact.h"
#include <stdio.h> 

int main() {
    // 1. SETUP CỬA SỔ
    SetConfigFlags(FLAG_WINDOW_HIGHDPI); 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(FPS); 

    // 2. KHỞI TẠO DỮ LIỆU (INIT)
    InitRenderer(); 

    // --- Tạo Player ---
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 

    // --- Tạo Map ---
    GameMap currentMap;
    currentMap.texture.id = 0; 
    LoadMap(&currentMap, MAP_THU_VIEN); 

    // --- Tạo NPC ---
    Npc npcList[MAX_NPCS];
    int npcCount = 0;

    // NPC 01: Cô Đầu Bếp
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){206, 250}, "Co Dau Bep");
    npcCount++;

    // --- Vật Cản Tĩnh ---
    GameProp cotNha;
    cotNha.texture = LoadTexture("resources/cot_nha.png"); 
    cotNha.position = (Vector2){ 400, 200 }; 
    cotNha.sourceRec = (Rectangle){0, 0, (float)cotNha.texture.width, (float)cotNha.texture.height};
    cotNha.originY = (float)cotNha.texture.height - 10; 

    // 3. VÒNG LẶP GAME (GAME LOOP)
    while (!WindowShouldClose()) {
        // --- PHẦN LOGIC (UPDATE) ---
        if (IsKeyPressed(KEY_F1)) LoadMap(&currentMap, MAP_THU_VIEN);
        if (IsKeyPressed(KEY_F2)) LoadMap(&currentMap, MAP_NHA_AN);

        UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == currentMap.currentMapID) {
                UpdateNpc(&npcList[i]);
            }
        }
      // [MỚI] 2. Gọi hàm xử lý logic tương tác (Check phím E)
        Interact_Update(&mainCharacter, npcList, npcCount);
        // --- PHẦN VẼ (DRAW) ---
        BeginDrawing();
            ClearBackground(RAYWHITE); 
            
            // LỚP 1: Nền đất
            DrawMap(&currentMap);
            
            // LỚP 2: CÁC VẬT THỂ NỔI
            Render_Clear(); 
            Render_AddPlayer(&mainCharacter);
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) {
                    Render_AddNpc(&npcList[i]);
                }
            }
            if (currentMap.currentMapID == MAP_THU_VIEN) {
                Render_AddProp(&cotNha);
            }
            Render_DrawAll();
  // [MỚI] 3. Gọi hàm vẽ giao diện hội thoại & nút nhắc nhở [E]
            Interact_DrawUI(&mainCharacter, npcList, npcCount);
            // LỚP 3: Debug & UI (Luôn nằm trên cùng)
            // [CẬP NHẬT] Truyền thêm mainCharacter và npcList vào đây
            Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount); 
            
            DrawText("F1: Thu Vien | F2: Nha An", 10, 10, 20, BLACK);
            DrawText(TextFormat("HP: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp), 10, 40, 20, RED);

        EndDrawing();
    }

    // 4. DỌN DẸP (UNLOAD)
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    for (int i = 0; i < npcCount; i++) UnloadNpc(&npcList[i]);
    UnloadTexture(cotNha.texture);

    CloseWindow();
    return 0;
}