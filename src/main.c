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
#include "info.h"
#include "transition.h"
#include "inventory.h"
#include "combat.h" 
#include "combatbychatting.h" // [THÊM MỚI] Khai báo Hỏi Đáp

int main() {
    // 1. INIT - Khởi tạo theo thứ tự nghiêm ngặt (Window -> Audio -> Assets)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); 
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    
    // Khởi tạo màn hình ảo
    InitScaling(); 
    
    InitAudioDevice();   
    SetWindowMinSize(320, 240); 
    SetTargetFPS(FPS);   
    InitUIStyle();       
    InitRenderer();      
    Audio_Init();
    Menu_Init(); 
    Info_Init();
    Camera_Init();
    Dialog_Init("resources/font_dialog/dialogs.txt"); 

    // 2. SETUP GAMEPLAY
    
    Inventory_Init(); 
    Gameplay_Init();

    // Khởi tạo các hệ thống Combat
    Combat_Init();
    CBC_Init(); // [THÊM MỚI] Khởi tạo hệ thống Hỏi Đáp

    // 3. INTRO
    bool showIntro = true;
    InitIntro("resources/intro/intro.mpg");

    // 4. MAIN GAME LOOP
    while (!WindowShouldClose() && !Menu_ShouldCloseGame()) {
        
        // Luôn update audio stream
        Audio_Update();

        // --- PHASE INTRO ---
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
            continue; 
        }

        // --- PHASE UPDATE LOGIC ---
        // Ưu tiên 1: Nếu đang Combat vật lý -> Chỉ Update Combat
        if (Combat_IsActive()) {
            Combat_Update();
        } 
        // [THÊM MỚI] Ưu tiên 2: Nếu đang Hỏi Đáp -> Chỉ Update Hỏi Đáp (khóa nhân vật)
        else if (CBC_IsActive()) {
            CBC_Update();
        }
        else if (StoryCutscene_IsActive()) {
            if (StoryCutscene_Update()) {
                // Cutscene trả về true -> Xem xong -> Chuyển sang màn Chọn Class
                Menu_SwitchTo(MENU_CHARACTER_SELECT);
                
            }
        }
        else {
            // Ưu tiên 3: Update Menu hoặc Gameplay
            Menu_Update();
            
            // Chỉ update Gameplay khi không ở Menu chính HOẶC đang có hiệu ứng chuyển cảnh
            if (currentMenu == MENU_NONE || Transition_IsActive()) {
                Gameplay_Update();
            }
        }

        // --- PHASE DRAWING ---
        BeginDrawing();
            ClearBackground(BLACK); 
            
            BeginScaling(); 
                ClearBackground(RAYWHITE); 
                
                // KIỂM TRA: NẾU ĐANG ĐÁNH NHAU VẬT LÝ -> VẼ COMBAT
                if (Combat_IsActive()) {
                    Combat_Draw();
                }
                // [THÊM MỚI] Nếu Story Cutscene đang bật thì chỉ vẽ nó
                else if (StoryCutscene_IsActive()) {
                    StoryCutscene_Draw();
                }
                else {
                    // NẾU KHÔNG -> VẼ GAME BÌNH THƯỜNG
                    if (currentMenu == MENU_TITLE) {
                        Menu_Draw(); 
                        Debug_RunMenuTool();
                    }
                    else {
                        Gameplay_Draw(); // Vẽ Map, Player, NPC
                        
                        // [THÊM MỚI] NẾU ĐANG HỎI ĐÁP -> VẼ MENU HỎI ĐÁP CHỒNG LÊN MAP
                        if (CBC_IsActive()) {
                            CBC_Draw();
                        }

                        Menu_Draw();     // Vẽ Menu Pause (nếu có) đè lên

                        if (currentMenu != MENU_NONE) {
                            Debug_RunMenuTool();
                        }
                    }
                }
                Debug_RunDialogTool();
                // [MỚI] VẼ BẢN ĐỒ BÍ MẬT LÊN TRÊN CÙNG
                 Inventory_DrawSecretMap();

            EndScaling(); 
            
            
            
            // Vẽ màn đen chuyển cảnh PHỦ LÊN TẤT CẢ (Trên cùng)
            Transition_Draw();

        EndDrawing();
    }

    // 5. CLEANUP
    if (showIntro) UnloadIntro();
    
    Gameplay_Shutdown();
    Combat_Shutdown(); 
    CBC_Shutdown(); // [THÊM MỚI] Dọn dẹp Hỏi đáp
    
    UnloadScaling();
    Menu_Shutdown();
    Info_Shutdown();
    Audio_Shutdown();
    CloseUIStyle(); 
    CloseAudioDevice();
    Inventory_Unload();
    CloseWindow();

    return 0;
}