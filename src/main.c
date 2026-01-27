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
#include "audio_manager.h" 
#include "menu_system.h" 
#include <stdio.h> 
#include <string.h> 
#include "dialog_system.h"

int main() {
    // 1. INIT
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); 
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    
    // [MỚI 1] Khởi tạo màn hình ảo ngay sau khi tạo cửa sổ
    InitScaling(); 
    
    InitAudioDevice();   
    SetWindowMinSize(320, 240); 
    SetTargetFPS(FPS);   
    InitUIStyle();       
    InitRenderer();      
    Audio_Init();
    Menu_Init(); 
    Dialog_Init("resources/dialogs.txt"); // <--- Nạp dữ liệu thoại

    // 2. SETUP OBJECTS
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 
    
    GameMap currentMap;
    currentMap.texture.id = 0; 
    LoadMap(&currentMap, MAP_THU_VIEN); 
    
    Npc npcList[MAX_NPCS];
    int npcCount = 0;
    
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){206, 250}, "Co Dau Bep", 0);
    strcpy(npcList[0].dialogKey, "DEFAULT"); // Mặc định nói câu Default
    npcCount++;

    GameProp cotNha;
    cotNha.texture = LoadTexture("resources/cot_nha.png");
    cotNha.position = (Vector2){ 400, 200 };
    cotNha.originY = cotNha.texture.height - 10; 

    // 3. INTRO
    bool showIntro = true;
    InitIntro("resources/intro.mpg");

    // 4. MAIN GAME LOOP
    while (!WindowShouldClose() && !Menu_ShouldCloseGame()) {
        
        // [CLEANUP] Đã chuyển logic F11 sang Menu_Update()

        Audio_Update();

        // Phase Intro (Giữ nguyên, không scale intro vì video thường tự handle)
        if (showIntro) {
            if (UpdateIntro()) {
                UnloadIntro(); 
                showIntro = false;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawIntro();
            EndDrawing();
            continue; 
        }

        // Phase Update Logic
        // Hàm này sẽ kiểm tra F11 và cả các phím tắt Menu
        Menu_Update();

        if (currentMenu == MENU_NONE) {
            UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) UpdateNpc(&npcList[i]);
            }
            Interact_Update(&mainCharacter, npcList, npcCount, &currentMap);
            
            if (IsKeyPressed(KEY_F1)) LoadMap(&currentMap, MAP_THU_VIEN);
            if (IsKeyPressed(KEY_F2)) LoadMap(&currentMap, MAP_NHA_AN);
            if (IsKeyPressed(KEY_F3)) LoadMap(&currentMap, MAP_SAN_TRUONG);
        }

        // Phase Drawing
        BeginDrawing();
            ClearBackground(BLACK); // [MỚI] Xóa nền màn hình thật thành màu đen (tạo viền)
            
            // [MỚI 2] Bắt đầu chế độ vẽ vào màn hình ảo
            BeginScaling(); 
                ClearBackground(RAYWHITE); // [MỚI] Xóa nền bên trong game (background thực)
                
                if (currentMenu == MENU_TITLE) {
                    Menu_Draw(); 
                    Debug_RunMenuTool();
                }
                else {
                    DrawMap(&currentMap);
                    
                    Render_Clear(); 
                    Render_AddPlayer(&mainCharacter);
                    for (int i = 0; i < npcCount; i++) {
                        if (npcList[i].mapID == currentMap.currentMapID) Render_AddNpc(&npcList[i]);
                    }
                    if (currentMap.currentMapID == MAP_THU_VIEN) Render_AddProp(&cotNha);
                    Render_DrawAll(); 

                    Interact_DrawUI(&mainCharacter, npcList, npcCount, &currentMap);
                    Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount); 
                    
                    // Vẽ hướng dẫn
                    if (currentMenu == MENU_NONE) {
                       DrawText("F11: Toan Man Hinh", 10, GetScreenHeight() - 30, 20, LIGHTGRAY);
                    }
                    
                    DrawTextEx(globalFont, "F1: Thu Vien | F2: Nha An", (Vector2){10, 10}, 24, 1, DARKGRAY);
                    const char* hpText = TextFormat("Suc Khoe: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp);
                    DrawTextEx(globalFont, hpText, (Vector2){10, 40}, 24, 1, RED);

                    Menu_Draw();

                    if (currentMenu != MENU_NONE) {
                        Debug_RunMenuTool();
                    }
                }

            // [MỚI 3] Kết thúc vẽ ảo và phóng to ra màn hình thật
            EndScaling(); 

        EndDrawing();
    }

    // 5. CLEANUP
    if (showIntro) UnloadIntro();
    
    // [MỚI 4] Giải phóng bộ nhớ Scaling
    UnloadScaling();
    
    UnloadMap(&currentMap);
    Menu_Shutdown();
    Audio_Shutdown();
    CloseUIStyle(); 
    CloseAudioDevice();
    CloseWindow();

    return 0;
}