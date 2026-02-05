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
#include "gameplay.h"
#include "transition.h"
#include "inventory.h"

int main() {
    // 1. INIT - Khởi tạo theo thứ tự nghiêm ngặt (Window -> Audio -> Assets)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); 
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    
    // Khởi tạo màn hình ảo ngay sau khi tạo cửa sổ
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
    Dialog_Init("resources/font_dialog/dialogs.txt"); 
   // 2. SETUP GAMEPLAY
   //Các biến khởi tạo để hết gameplay.h và gameplay.c nha ae
    Gameplay_Init();
    Inventory_Init(); // Khởi tạo túi đồ sau Gameplay

    // 3. INTRO
    bool showIntro = true;
    InitIntro("resources/intro/intro.mpg");

    // 4. MAIN GAME LOOP
    while (!WindowShouldClose() && !Menu_ShouldCloseGame()) {
        
        // [QUAN TRỌNG]: Luôn update audio stream
        Audio_Update();

        // Phase Intro - Chạy video mở đầu
        if (showIntro) {
            if (UpdateIntro()) {
                UnloadIntro(); 
                showIntro = false;
                Transition_IntroDone();
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawIntro();
            EndDrawing();
            continue; // Bỏ qua các logic game bên dưới khi đang chiếu intro
        }

        // Phase Update Logic
        Menu_Update();

        if (currentMenu == MENU_NONE || Transition_IsActive()) {
            Gameplay_Update();
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
                   Gameplay_Draw(); // Vẽ game
                    
                    Menu_Draw();     // Vẽ menu đè lên (nếu đang Pause)

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
    Gameplay_Shutdown();
    UnloadScaling();
    Menu_Shutdown();
    Audio_Shutdown();
    CloseUIStyle(); 
    CloseAudioDevice();
    CloseWindow();

    return 0;
}