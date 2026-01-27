// FILE: src/menu_system.c
#include "menu_system.h"
#include "audio_manager.h"
#include "ui_style.h" 
#include "debug.h" 
#include "settings.h" // Để dùng SCREEN_WIDTH, SCREEN_HEIGHT, ToggleGameFullscreen
#include <stdio.h>
#include <math.h> 

MenuType currentMenu = MENU_TITLE; 

static bool shouldCloseGame = false;
static Texture2D texTitleBG;

bool Menu_ShouldCloseGame() {
    return shouldCloseGame;
}

static bool DrawButton(const char* debugName, Rectangle rec) {
    bool clicked = false;
    
    // [SỬA] Dùng chuột ảo thay vì chuột thật
    Vector2 mousePoint = GetVirtualMousePos(); 
    
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    if (isHover) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        float time = (float)GetTime();
        float alpha = (sinf(time * 6.0f) + 1.0f) / 2.0f; 
        float finalAlpha = 0.5f + (alpha * 0.5f);        
        DrawRectangleLinesEx(rec, 3.0f, Fade(GOLD, finalAlpha));
        DrawRectangleRec(rec, Fade(GOLD, 0.1f)); 

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            clicked = true;
        }
    }

    if (IsMenuDebugActive()) {
        DrawRectangleLinesEx(rec, 2.0f, RED); 
        DrawRectangleRec(rec, Fade(RED, 0.2f)); 
        DrawText(debugName, (int)rec.x, (int)rec.y - 20, 20, RED); 
    }

    return clicked;
}

void Menu_Init() {
    texTitleBG = LoadTexture("resources/titlescreen.png");
    SetTextureFilter(texTitleBG, TEXTURE_FILTER_POINT); // Pixel Art nên dùng POINT thay vì BILINEAR
}

void Menu_Update() {
    // [SỬA] Gọi hàm toggle xịn của settings.c
    if (IsKeyPressed(KEY_F11)) {
        ToggleGameFullscreen();
    }

    // Logic chuyển menu
    if (currentMenu == MENU_NONE) {
        if (IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_PAUSE);
        if (IsKeyPressed(KEY_I)) Menu_SwitchTo(MENU_UPGRADE);
        return;
    }

    switch (currentMenu) {
        case MENU_TITLE: break;
        case MENU_PAUSE: if (IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_NONE); break;
        case MENU_UPGRADE: if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_NONE); break;
        default: break;
    }
}

void Menu_Draw() {
    if (currentMenu == MENU_NONE) return;

    // [QUAN TRỌNG] Không cần check WindowMinimized hay tính scale nữa
    // Vì hàm này đã được gọi bên trong BeginScaling() -> Luôn vẽ ở chuẩn 800x450

    // Dùng hằng số chuẩn từ settings.h
    float sw = (float)SCREEN_WIDTH;  // 800
    float sh = (float)SCREEN_HEIGHT; // 450

    switch (currentMenu) {
        case MENU_TITLE:
            {
                Rectangle sourceRec = { 0.0f, 0.0f, (float)texTitleBG.width, (float)texTitleBG.height };
                // Vẽ full màn hình ảo
                Rectangle destRec = { 0.0f, 0.0f, sw, sh }; 
                DrawTexturePro(texTitleBG, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
            }
            
            // Tọa độ cứng theo thiết kế 800x450 (Không nhân scaleX nữa)
            if (DrawButton("START", (Rectangle){ 305, 94, 190, 49 })) {
                Menu_SwitchTo(MENU_NONE); 
                Debug_ForceCloseMenuTool(); 
            }
            if (DrawButton("EXIT", (Rectangle){ 304, 306, 190, 50 })) {
                shouldCloseGame = true; 
            }
            break;

        case MENU_PAUSE:
            DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.5f));
            {
                float boxW = 300; 
                float boxH = 300;
                Rectangle pauseBox = { (sw - boxW)/2, (sh - boxH)/2, boxW, boxH };
                
                DrawRectangleRec(pauseBox, DARKBLUE);
                DrawRectangleLinesEx(pauseBox, 3, WHITE); 
                
                int fontSize = 30;
                DrawText("PAUSE", (int)(pauseBox.x + 100), (int)(pauseBox.y + 20), fontSize, WHITE);

                if (DrawButton("RESUME", (Rectangle){pauseBox.x + 50, pauseBox.y + 80, 200, 40})) {
                    Menu_SwitchTo(MENU_NONE);
                }
                if (DrawButton("QUIT", (Rectangle){pauseBox.x + 50, pauseBox.y + 140, 200, 40})) {
                    Menu_SwitchTo(MENU_TITLE); 
                }
            }
            break;

        case MENU_UPGRADE:
            DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.7f));
            DrawText("UPGRADE", 50, 50, 40, YELLOW);
            
            if (DrawButton("BUY HP", (Rectangle){100, 150, 250, 50})) {
                printf(">> Buy HP logic...\n");
            }
            break;
            
        default: break;
    }
}

void Menu_SwitchTo(MenuType type) {
    currentMenu = type;
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
}

void Menu_Shutdown() {
    UnloadTexture(texTitleBG);
}