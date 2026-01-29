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
#include "camera.h"
#include "transition.h" // [MỚI] Thêm thư viện chuyển cảnh

int main() {
    // 1. INIT - Khởi tạo theo thứ tự nghiêm ngặt (Window -> Audio -> Assets)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); 
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    
    // [MỚI 1] Khởi tạo màn hình ảo ngay sau khi tạo cửa sổ
    // [GIẢI THÍCH]: Giúp game giữ nguyên tỉ lệ pixel khi phóng to cửa sổ.
    InitScaling(); 
    
    InitAudioDevice();   
    SetWindowMinSize(320, 240); 
    SetTargetFPS(FPS);   
    InitUIStyle();       
    InitRenderer();      
    Audio_Init();
    Menu_Init(); 
    Camera_Init();
    Dialog_Init("resources/dialogs.txt"); 

    // 2. SETUP OBJECTS
    Player mainCharacter;
    InitPlayer(&mainCharacter, CLASS_STUDENT); 
    
    GameMap currentMap;
    currentMap.texture.id = 0; 
    LoadMap(&currentMap, MAP_THU_VIEN); 
    
    Npc npcList[MAX_NPCS];
    int npcCount = 0;
    
    InitNpc(&npcList[0], MAP_THU_VIEN, "resources/codaubep.png", (Vector2){206, 250}, "Co Dau Bep", 0);
    strcpy(npcList[0].dialogKey, "DEFAULT"); 
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
        
        // [QUAN TRỌNG]: Luôn update audio stream
        Audio_Update();

        // Phase Intro - Chạy video mở đầu
        if (showIntro) {
            if (UpdateIntro()) {
                UnloadIntro(); 
                showIntro = false;
                Audio_PlayMusic(MUSIC_TITLE);
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawIntro();
            EndDrawing();
            continue; // Bỏ qua các logic game bên dưới khi đang chiếu intro
        }

        // Phase Update Logic
        Menu_Update();

        // [MỚI] Luôn cập nhật trạng thái chuyển cảnh (Fade in/out)
        Transition_Update(&currentMap, &mainCharacter);

        if (currentMenu == MENU_NONE) {
            
            // [MỚI] Chỉ cho phép nhân vật di chuyển/tương tác khi KHÔNG đang chuyển cảnh
            // Giúp tránh lỗi nhân vật chạy lung tung trong lúc màn hình đen
            if (!Transition_IsActive()) {
                UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].mapID == currentMap.currentMapID) UpdateNpc(&npcList[i]);
                }
                Interact_Update(&mainCharacter, npcList, npcCount, &currentMap);
                
                // [SỬA] Thay LoadMap thô bằng hiệu ứng chuyển cảnh (Ví dụ mẫu)
                // Lưu ý: Tọa độ (Vector2){x, y} là điểm xuất hiện của nhân vật ở map mới
                // [CÓ THỂ THỪA]: Các phím F1, F2, F3 này dùng để debug nhảy map nhanh. 
                // Nên xóa khi phát hành game để tránh người chơi ăn gian.
                if (IsKeyPressed(KEY_F1)) Transition_StartToMap(MAP_THU_VIEN, (Vector2){200, 200});
                if (IsKeyPressed(KEY_F2)) Transition_StartToMap(MAP_NHA_AN, (Vector2){400, 300});
                if (IsKeyPressed(KEY_F3)) Transition_StartToMap(MAP_SAN_TRUONG, (Vector2){300, 300});
                
                Camera_Update(&mainCharacter, &currentMap);
            }
        }

        // Phase Drawing
        BeginDrawing();
            ClearBackground(BLACK); 
            
            // [GIẢI THÍCH]: BeginScaling giúp vẽ mọi thứ lên màn hình ảo kích thước thấp, sau đó phóng to ra.
            BeginScaling(); 
                ClearBackground(RAYWHITE); 
                
                if (currentMenu == MENU_TITLE) {
                    Menu_Draw(); 
                    Debug_RunMenuTool();
                }
                else {
                    // 1. VẼ THẾ GIỚI GAME (Có Camera)
                    BeginMode2D(gameCamera); 

                    DrawMap(&currentMap);
                    
                    // Renderer (Y-Sorting): Vẽ nhân vật và đồ vật theo thứ tự trên/dưới để tạo chiều sâu
                    Render_Clear(); 
                    Render_AddPlayer(&mainCharacter);
                    for (int i = 0; i < npcCount; i++) {
                        if (npcList[i].mapID == currentMap.currentMapID) Render_AddNpc(&npcList[i]);
                    }
                    if (currentMap.currentMapID == MAP_THU_VIEN) Render_AddProp(&cotNha);
                    Render_DrawAll(); 

                    Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount); 

                    EndMode2D(); 

                    // 2. VẼ UI (Không chịu ảnh hưởng Camera)
                    Interact_DrawUI(&mainCharacter, npcList, npcCount, &currentMap);
                    
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

            EndScaling(); 
            
            // [MỚI] Vẽ màn đen chuyển cảnh PHỦ LÊN TẤT CẢ (Trên cùng)
            Transition_Draw();

        EndDrawing();
    }

    // 5. CLEANUP
    if (showIntro) UnloadIntro();
    
    UnloadScaling();
    UnloadMap(&currentMap);
    Menu_Shutdown();
    Audio_Shutdown();
    CloseUIStyle(); 
    CloseAudioDevice();
    CloseWindow();

    return 0;
}