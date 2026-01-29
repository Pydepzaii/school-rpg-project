// FILE: src/menu_system.c
#include "menu_system.h"
#include "audio_manager.h"
#include "ui_style.h" 
#include "debug.h" 
#include "settings.h" 
#include <stdio.h>
#include "transition.h"
#include <math.h> 

// Biến này giúp game nhớ xem lúc nãy chuột đang ở đâu
static const char* lastHoveredBtn = NULL;
MenuType currentMenu = MENU_TITLE; 
static MenuType previousMenu = MENU_TITLE;
static bool shouldCloseGame = false;
static Texture2D texTitleBG;
static float clickCooldown = 0.0f; 

// Khai báo trước các hàm (Forward declaration)
static bool DrawButton(const char* text, Rectangle rec);
static bool DrawInvisibleButton(const char* debugName, Rectangle rec);

void Menu_RequestClose() { shouldCloseGame = true; }
bool Menu_ShouldCloseGame() { return shouldCloseGame; }

// --- Hàm Vẽ Thanh Volume ---
static float DrawVolumeControl(const char* label, Rectangle bounds, float value) {
    Vector2 mouse = GetVirtualMousePos();
    DrawText(label, (int)bounds.x, (int)bounds.y - 25, 20, YELLOW);
    
    // Nút giảm (-)
    if (DrawButton("-", (Rectangle){ bounds.x, bounds.y, 30, bounds.height })) {
        value -= 0.1f;
        Audio_PlaySoundEffect(SFX_UI_CLICK); 
    }
    
    // Thanh trượt (Slider)
    Rectangle sliderRec = { bounds.x + 40, bounds.y, bounds.width - 80, bounds.height };
    // Logic kéo thả thanh trượt
    if (CheckCollisionPointRec(mouse, sliderRec) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && clickCooldown <= 0.0f) {
        value = (mouse.x - sliderRec.x) / sliderRec.width;
    }
    // Kẹp giá trị từ 0 đến 1
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    DrawRectangleRec(sliderRec, DARKGRAY);          
    DrawRectangleLinesEx(sliderRec, 2, WHITE);      
    DrawRectangleRec((Rectangle){ sliderRec.x, sliderRec.y, sliderRec.width * value, sliderRec.height }, BLUE);
    DrawRectangle((int)(sliderRec.x + sliderRec.width * value) - 5, (int)sliderRec.y - 5, 10, sliderRec.height + 10, RED);

    // Nút tăng (+)
    if (DrawButton("+", (Rectangle){ bounds.x + bounds.width - 30, bounds.y, 30, bounds.height })) {
        value += 0.1f;
        Audio_PlaySoundEffect(SFX_UI_CLICK);
    }
    DrawText(TextFormat("%d%%", (int)(value * 100)), (int)(bounds.x + bounds.width + 10), (int)bounds.y, 20, WHITE);
    return value;
}

// --- HÀM NÚT TÀNG HÌNH (Dùng cho Title Screen) ---
// [GIẢI THÍCH]: Nút này không vẽ gì cả (trừ khi di chuột vào), dùng để đè lên các nút trong ảnh nền (background image).
static bool DrawInvisibleButton(const char* debugName, Rectangle rec) {
    bool clicked = false;
    Vector2 mousePoint = GetVirtualMousePos(); 
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    if (isHover) {
        if (lastHoveredBtn != debugName) {
            Audio_PlaySoundEffect(SFX_UI_HOVER); 
            lastHoveredBtn = debugName;          
        }
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        
        // Hiệu ứng nhấp nháy (Sine wave) khi hover
        float time = (float)GetTime();
        float alpha = (sinf(time * 6.0f) + 1.0f) / 2.0f; 
        float finalAlpha = 0.5f + (alpha * 0.5f);        
        
        DrawRectangleLinesEx(rec, 3.0f, Fade(GOLD, finalAlpha)); 

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Audio_PlaySoundEffect(SFX_UI_CLICK); 
            clicked = true;
        }
    } 
    else {
        if (lastHoveredBtn == debugName) lastHoveredBtn = NULL; 
    }

    // [FIX 1 & 2] Logic hiển thị Debug
    if (IsMenuDebugActive()) {
        // 1. Vẽ khung xanh lá (Chỉ hiện khi bật Debug)
        DrawRectangleLinesEx(rec, 3.0f, GREEN); 

        // 2. Màu đỏ nền đậm hơn (0.8f) để dễ nhìn
        DrawRectangleRec(rec, Fade(RED, 0.8f)); 
        DrawText(debugName, (int)rec.x, (int)rec.y - 20, 10, RED); 
    }

    return clicked;
}

// --- HÀM NÚT THƯỜNG ---
static bool DrawButton(const char* text, Rectangle rec) {
    bool clicked = false;
    Vector2 mousePoint = GetVirtualMousePos(); 
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    DrawRectangleLinesEx(rec, 2.0f, WHITE); 
    int textW = MeasureText(text, 20);
    DrawText(text, (int)(rec.x + (rec.width - textW)/2), (int)(rec.y + (rec.height - 20)/2), 20, WHITE);

    if (isHover) {
        if (lastHoveredBtn != text) {
            Audio_PlaySoundEffect(SFX_UI_HOVER); 
            lastHoveredBtn = text;          
        }
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        
        float time = (float)GetTime();
        float alpha = (sinf(time * 6.0f) + 1.0f) / 2.0f; 
        float finalAlpha = 0.5f + (alpha * 0.5f);        
        DrawRectangleLinesEx(rec, 3.0f, Fade(GOLD, finalAlpha)); 
        DrawRectangleRec(rec, Fade(GOLD, 0.1f)); 
        DrawText(text, (int)(rec.x + (rec.width - textW)/2), (int)(rec.y + (rec.height - 20)/2), 20, YELLOW);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Audio_PlaySoundEffect(SFX_UI_CLICK); 
            clicked = true;
        }
    } 
    else {
        if (lastHoveredBtn == text) lastHoveredBtn = NULL; 
    }

    // [FIX 2] Debug đậm hơn cho nút thường
    if (IsMenuDebugActive()) {
        DrawRectangleLinesEx(rec, 2.0f, RED); 
        DrawRectangleRec(rec, Fade(RED, 0.8f)); // Đậm hơn
        DrawText(text, (int)rec.x, (int)rec.y - 20, 10, RED); 
    }
    return clicked;
}

void Menu_Init() {
    texTitleBG = LoadTexture("resources/titlescreen.png");
    SetTextureFilter(texTitleBG, TEXTURE_FILTER_POINT); 
    SetExitKey(KEY_NULL); 
}

void Menu_Update() {
    if (IsKeyPressed(KEY_F11)) ToggleGameFullscreen();

    if (currentMenu == MENU_NONE) {
        if (IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_PAUSE);
        if (IsKeyPressed(KEY_I)) Menu_SwitchTo(MENU_UPGRADE);
        return;
    }

    switch (currentMenu) {
        case MENU_TITLE: break;
        case MENU_PAUSE: 
            if (IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_NONE); 
            break;
        case MENU_UPGRADE: 
            if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_NONE); 
            break;
        case MENU_SETTINGS: 
            if (IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(previousMenu); 
            break;
        default: break;
    }
}

void Menu_Draw() {
    if (currentMenu == MENU_NONE) return;

    float sw = (float)SCREEN_WIDTH;  
    float sh = (float)SCREEN_HEIGHT; 

    switch (currentMenu) {
        case MENU_TITLE:
            {
                Rectangle sourceRec = { 0.0f, 0.0f, (float)texTitleBG.width, (float)texTitleBG.height };
                Rectangle destRec = { 0.0f, 0.0f, sw, sh }; 
                DrawTexturePro(texTitleBG, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
            }
            
            // --- [FIX 3] CHỈNH TỌA ĐỘ NÚT Ở ĐÂY ---
            // Nút START
            if (DrawInvisibleButton("START", (Rectangle){ 305, 94, 190, 49 })) {
                Debug_ForceCloseMenuTool(); 
                Transition_StartToMap(MAP_THU_VIEN, (Vector2){200, 250});
                Audio_PlayMusic(MUSIC_THU_VIEN);
            }

            // Nút SETTINGS: Sửa 4 số trong ngoặc nhọn này
            // { Tọa độ X, Tọa độ Y, Chiều Rộng, Chiều Cao }
            if (DrawInvisibleButton("SETTINGS", (Rectangle){  307, 236, 189, 50 })) {
                previousMenu = MENU_TITLE; 
                Menu_SwitchTo(MENU_SETTINGS);
            }

            // Nút EXIT
            if (DrawInvisibleButton("EXIT", (Rectangle){ 304, 306, 190, 50 })) {
                Transition_StartExit();
            }
            break;

        case MENU_PAUSE:
            DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.5f));
            {
                float boxW = 300; float boxH = 300;
                Rectangle pauseBox = { (sw - boxW)/2, (sh - boxH)/2, boxW, boxH };
                
                DrawRectangleRec(pauseBox, DARKBLUE);
                DrawRectangleLinesEx(pauseBox, 3, WHITE); 
                int fontSize = 30;
                DrawText("PAUSE", (int)(pauseBox.x + 100), (int)(pauseBox.y + 20), fontSize, WHITE);

                if (DrawButton("RESUME", (Rectangle){pauseBox.x + 50, pauseBox.y + 80, 200, 40})) {
                    Menu_SwitchTo(MENU_NONE);
                }
                if (DrawButton("SETTINGS", (Rectangle){pauseBox.x + 50, pauseBox.y + 140, 200, 40})) {
                    previousMenu = MENU_PAUSE; 
                    Menu_SwitchTo(MENU_SETTINGS);
                }
                if (DrawButton("QUIT", (Rectangle){pauseBox.x + 50, pauseBox.y + 200, 200, 40})) {
                    Menu_SwitchTo(MENU_TITLE); 
                    Audio_PlayMusic(MUSIC_TITLE);
                }
            }
            break;

        case MENU_UPGRADE:
            // [THỪA]: Menu này hiện chỉ in ra console, chưa có logic game thực tế.
            DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.7f));
            DrawText("UPGRADE", 50, 50, 40, YELLOW);
            if (DrawButton("BUY HP", (Rectangle){100, 150, 250, 50})) {
                printf(">> Buy HP logic...\n");
            }
            break;
        
        case MENU_SETTINGS:
            DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.9f));
            DrawText("SETTINGS", (int)(sw/2 - 80), 40, 40, YELLOW);

            float currentVol = Audio_GetMasterVolume();
            float newVol = DrawVolumeControl("Master Volume", (Rectangle){ 200, 130, 400, 30 }, currentVol);
            if (newVol != currentVol) {
                Audio_SetMasterVolume(newVol);
            }

            const char* modeText = IsWindowFullscreen() ? "Mode: FULLSCREEN" : "Mode: WINDOWED";
            if (DrawButton(modeText, (Rectangle){ (sw-300)/2, 250, 300, 40 })) {
                ToggleGameFullscreen();
            }

            if (DrawButton("BACK", (Rectangle){ (sw-200)/2, 350, 200, 40 })) {
                Menu_SwitchTo(previousMenu); 
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