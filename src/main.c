// FILE: src/main.c
#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include "debug.h" 
#include "renderer.h" 
#include "interact.h"
#include "ui_style.h"
#include "intro.h"
#include "audio_manager.h" // [THÊM] Import Audio Manager
#include <stdio.h> 
#include <string.h> 

typedef enum {
    STATE_INTRO,
    STATE_GAMEPLAY
} GameState;

int main() {
    // 1. SETUP
    SetConfigFlags(FLAG_WINDOW_HIGHDPI); 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    InitAudioDevice(); 
    SetTargetFPS(FPS); 
    InitUIStyle(); 
    InitRenderer(); 

    // [THÊM] Khởi tạo hệ thống âm thanh
    Audio_Init();

    // --- SETUP GAMEPLAY (Giữ nguyên) ---
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 
    GameMap currentMap;
    currentMap.texture.id = 0; 
    LoadMap(&currentMap, MAP_THU_VIEN); 
    Npc npcList[MAX_NPCS];
    int npcCount = 0;
    
    // NPC 1
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){206, 250}, "Co Dau Bep");
    strcpy(npcList[0].dialog, "Xin chào! Hôm nay em muốn ăn món gì?");
    npcCount++;

    GameProp cotNha;
    cotNha.texture = LoadTexture("resources/cot_nha.png");
    cotNha.position = (Vector2){ 400, 200 };
    cotNha.originY = cotNha.texture.height - 10; 

    // SETUP INTRO
    GameState currentState = STATE_INTRO; 
    InitIntro("resources/intro.mpg");     

    // 3. GAME LOOP
    while (!WindowShouldClose()) {
        
        // [THÊM] Cập nhật âm thanh liên tục (QUAN TRỌNG)
        Audio_Update();

        if (currentState == STATE_INTRO) {
            if (UpdateIntro()) {
                UnloadIntro(); 
                currentState = STATE_GAMEPLAY; 
                // [GỢI Ý] Khi vào game có thể bật nhạc nền thư viện luôn ở đây:
                // Audio_PlayMusic(MUSIC_THU_VIEN);
            }
            
            BeginDrawing();
                ClearBackground(BLACK);
                DrawIntro();
            EndDrawing();

        } else {
            // --- LOGIC GAMEPLAY (Giữ nguyên) ---
            
            UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) UpdateNpc(&npcList[i]);
            }
            Interact_Update(&mainCharacter, npcList, npcCount);
            
            // Ví dụ: Bấm F1 đổi map thì đổi nhạc luôn (Demo)
            if (IsKeyPressed(KEY_F1)) {
                 LoadMap(&currentMap, MAP_THU_VIEN);
                 // Audio_PlayMusic(MUSIC_THU_VIEN); // Bỏ comment để dùng
            }
            if (IsKeyPressed(KEY_F2)) {
                LoadMap(&currentMap, MAP_NHA_AN);
                // Audio_PlayMusic(MUSIC_NHA_AN); // Bỏ comment để dùng
            }
            if (IsKeyPressed(KEY_F3)) LoadMap(&currentMap, MAP_SAN_TRUONG);
            
            // DRAW
            BeginDrawing();
                ClearBackground(RAYWHITE); 
                DrawMap(&currentMap);
                Render_Clear(); 
                Render_AddPlayer(&mainCharacter);
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].mapID == currentMap.currentMapID) Render_AddNpc(&npcList[i]);
                }
                if (currentMap.currentMapID == MAP_THU_VIEN) Render_AddProp(&cotNha);
                Render_DrawAll(); 

                Interact_DrawUI(&mainCharacter, npcList, npcCount);
                Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount); 
                
                DrawTextEx(globalFont, "F1: Thư Viện | F2: Nhà Ăn", (Vector2){10, 10}, 24, 1, DARKGRAY);
                const char* hpText = TextFormat("Sức Khỏe: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp);
                DrawTextEx(globalFont, hpText, (Vector2){10, 40}, 24, 1, RED);
            EndDrawing();
        }
    }

    // UNLOAD
    if (currentState == STATE_INTRO) UnloadIntro(); 
    UnloadMap(&currentMap);
    
    // [THÊM] Dọn dẹp âm thanh
    Audio_Shutdown();

    CloseUIStyle(); 
    CloseAudioDevice();
    CloseWindow();

    return 0;
}