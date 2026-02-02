// FILE: src/menu_system.c
#include "menu_system.h"
#include "audio_manager.h"
#include "ui_style.h" 
#include "debug.h" 
#include "settings.h" 
#include <stdio.h>
#include "transition.h"
#include "gameplay.h"
#include <math.h> 

// Biến này giúp game nhớ xem lúc nãy chuột đang ở đâu
static const char* lastHoveredBtn = NULL;
MenuType currentMenu = MENU_TITLE; 
static MenuType previousMenu = MENU_TITLE;
static bool shouldCloseGame = false;
static Texture2D texTitleBG;
static float clickCooldown = 0.0f; 
//bIẾN CHO MÀN HÌNH CHỌN CLASS
static Texture2D texChoiceBG;       // Ảnh nền chọn nhân vật
static int selectedCharIndex = -1;  // -1: Chưa chọn, 0-3: Là các nhân vật
//làm tí hiệu ứng aura chọn nhân vật cho nó chất
static Texture2D texAura;       
static float auraRotation = 0.0f;
static bool isGameStarting = false;  // Đã bấm nút vào game chưa?
static float auraScale = 1.0f;       // Độ to của vòng tròn
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
    texTitleBG = LoadTexture("resources/menu/titlescreen.png");
    texChoiceBG = LoadTexture("resources/menu/choice_main.png");
    //load aura
    texAura = LoadTexture("resources/menu/aura.png"); 
    isGameStarting = false;
    auraScale = 1.0f;
    //Dùng Bilinear để ảnh mịn, không bị răng cưa khi xoay
    SetTextureFilter(texAura, TEXTURE_FILTER_BILINEAR);
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
                currentMenu = MENU_CHARACTER_SELECT; // Chuyển sang màn chọn
                selectedCharIndex = -1;              // Reset chưa chọn ai cả
                Audio_PlaySoundEffect(SFX_UI_CLICK);
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
        case MENU_CHARACTER_SELECT:
            {
                // 1. Vẽ nền (Ảnh 4 nhân vật)
                Rectangle sourceRec = { 0.0f, 0.0f, (float)texChoiceBG.width, (float)texChoiceBG.height };
                Rectangle destRec = { 0.0f, 0.0f, sw, sh }; 
                DrawTexturePro(texChoiceBG, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);

                // 2. Vẽ Tiêu Đề
                DrawText("CHON NHAN VAT", 50, 30, 30, WHITE);
                if (selectedCharIndex == -1) {
                    DrawText("Hay chon mot nhan vat...", 50, 70, 20, RED);
                } else {
                    DrawText("Da chon! Bam XAC NHAN de choi.", 50, 70, 20, GREEN);
                }

                // 3. Vẽ 4 nút chọn nhân vật
                
                // Class 1
                Rectangle btnC1 = {72, 100, 150, 220}; 
                if (DrawInvisibleButton("CLASS 1", btnC1)) {
                    selectedCharIndex = 0; // Ghim nhân vật 1
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }

                // Class 2
                Rectangle btnC2 = {240, 100, 150, 220};
                if (DrawInvisibleButton("CLASS 2", btnC2)) {
                    selectedCharIndex = 1; // Ghim nhân vật 2
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }

                // Class 3
                Rectangle btnC3 = { 420, 100, 150, 220};
                if (DrawInvisibleButton("CLASS 3", btnC3)) {
                    selectedCharIndex = 2; // Ghim nhân vật 3
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }

                // Class 4
                Rectangle btnC4 = { 600, 100, 150, 220};
                if (DrawInvisibleButton("CLASS 4", btnC4)) {
                    selectedCharIndex = 3; // Ghim nhân vật 4
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }

                // 4. HIỆU ỨNG "GHIM"
               Rectangle targetBox = {0};
                if (selectedCharIndex == 0) targetBox = btnC1;
                if (selectedCharIndex == 1) targetBox = btnC2;
                if (selectedCharIndex == 2) targetBox = btnC3;
                if (selectedCharIndex == 3) targetBox = btnC4;

                if (selectedCharIndex != -1) {
                    
                    // A. Tính toán tâm (VÀO GIỮA NGƯỜI)
                    Vector2 centerPos = {
                        targetBox.x + targetBox.width / 2.0f,
                        targetBox.y + targetBox.height / 2.0f 
                    };

                    // B. Xử lý logic: Đang chọn hay Đang vào game?
                    if (!isGameStarting) {
                        // --- TRẠNG THÁI 1: ĐANG CHỌN (Aura xoay nhẹ nhàng) ---
                        auraRotation += 60.0f * GetFrameTime(); // Xoay đều
                        auraScale = 1.0f + (sinf((float)GetTime() * 5.0f) * 0.05f); // Nhấp nháy nhẹ (1.0 -> 1.05)
                    } 
                    else {
                        // --- TRẠNG THÁI 2: ĐANG VÀO GAME (Phóng to cực đại) ---
                        auraRotation += 300.0f * GetFrameTime(); // Xoay tít thò lò
                        auraScale += 10.0f * GetFrameTime();      // Phóng to rất nhanh
                    }
                    if (auraRotation >= 360.0f) auraRotation -= 360.0f;

                    // C. Vẽ Aura
                    BeginBlendMode(BLEND_ADDITIVE); // Cộng màu cho sáng rực
                        
                        Rectangle source = {0, 0, (float)texAura.width, (float)texAura.height};
                        
                        // Kích thước vẽ = Kích thước gốc (150) * Tỉ lệ phóng to (auraScale)
                        float drawSize = 150.0f * auraScale; 
                        
                        Rectangle dest = { centerPos.x, centerPos.y, drawSize, drawSize };
                        Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f }; // Tâm xoay luôn ở giữa

                        // Vẽ lớp chính
                        DrawTexturePro(texAura, source, dest, origin, auraRotation, Fade(SKYBLUE, 0.8f));
                        // Vẽ lớp phụ xoay ngược (cho ảo)
                        DrawTexturePro(texAura, source, dest, origin, -auraRotation * 1.5f, Fade(WHITE, 0.5f));

                    EndBlendMode();

                    // D. Kiểm tra kết thúc hiệu ứng (Chuyển Map)
                    // Nếu vòng tròn to gấp 20 lần (che kín màn hình) -> Chuyển cảnh
                    if (isGameStarting && auraScale > 20.0f) {
                        Gameplay_SetPlayerClass(selectedCharIndex);
                        Transition_StartToMap(MAP_TOA_ALPHA, (Vector2){400, 300});
                        Audio_PlayMusic(MUSIC_TOA_ALPHA);
                        
                        // Reset lại để lần sau quay lại menu không bị lỗi
                        isGameStarting = false;
                        auraScale = 1.0f;
                    }

                    // E. Vẽ nút "VÀO GAME" 
                    // (CHỈ HIỆN KHI CHƯA BẤM BẮT ĐẦU - Để tránh bấm trùng)
                    if (!isGameStarting) {
                        Rectangle btnStartGame = { sw - 220, sh - 80, 200, 60 }; 
                        if (DrawButton("VAO GAME", btnStartGame)) {
                            // KHI BẤM NÚT: Không chuyển map ngay, mà kích hoạt hiệu ứng phóng to
                            isGameStarting = true;
                            Audio_PlaySoundEffect(SFX_UI_CLICK); // Hoặc âm thanh phép thuật nếu có
                        }
                    }
                }

                // 6. XỬ LÝ PHÍM ESC (Hủy chọn hoặc Quay lại)
                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (selectedCharIndex != -1) {
                        // Nếu đang chọn -> Hủy chọn
                        selectedCharIndex = -1;
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    } else {
                        // Nếu chưa chọn ai -> Quay về màn hình chính
                        currentMenu = MENU_TITLE;
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                }
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
                    Transition_StartToTitle();
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
    UnloadTexture(texChoiceBG);
    UnloadTexture(texAura);
}