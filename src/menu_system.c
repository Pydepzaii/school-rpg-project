// FILE: src/menu_system.c
#include "menu_system.h"
#include "audio_manager.h"
#include "ui_style.h" 
#include "debug.h" 
#include "settings.h" 
#include "inventory.h" 
#include "player.h"
#include <stdio.h>
#include <string.h> 
#include "transition.h"
#include "gameplay.h"
#include "save_system.h"
#include "intro.h"
#include "info.h"
#include <math.h> 

// --- GLOBAL DATA CHO MENU ĐỘNG (Title, Pause, Inventory) ---
MenuButton currentButtons[MAX_MENU_BUTTONS];
int currentBtnCount = 0;
static bool isDebugOverlay = false;

// --- GLOBAL DATA CHO CHARACTER SELECT (Dữ liệu cũ cần giữ lại) ---
static const char* lastHoveredBtn = NULL;
MenuType currentMenu = MENU_TITLE; 
static MenuType previousMenu = MENU_TITLE;
static bool shouldCloseGame = false;

static Texture2D texTitleBG;
static Texture2D texChoiceBG; 
static Texture2D texProfileBGs[4];      
static int selectedCharIndex = -1;  // -1: Chưa chọn
static Texture2D texAura;           // Ảnh vòng tròn phép thuật
Texture2D texUIBook;         // Spritesheet UI
static Texture2D texIconPlay;
static Texture2D texIconBack;
// [NEW] Các biến chứa ảnh Icon 4-frame mới
static Texture2D texIconSettings;
static Texture2D texIconResume;
static Texture2D texIconSave;
static Texture2D texIconHome;
static Texture2D texIconInventory;
static Texture2D texIconInfo; // [THÊM MỚI] Icon 4-frame cho Info

static float auraRotation = 0.0f;
static bool isGameStarting = false;
static float auraScale = 1.0f;
static Player previewChars[4];

// --- [NEW] Biến cho hiệu ứng chữ chạy Profile ---
static char displayedText[1024] = { 0 }; // Chuỗi lưu nội dung đang hiển thị
static int framesCounterText = 0;         // Đếm frame để hiện chữ
static int letterCount = 0;               // Số ký tự đã hiện
static bool isTextFinished = false;       // Đã chạy xong chữ chưa


// --- [NEW] BIẾN QUẢN LÝ ANIMATION MENU INFO ---
typedef enum {
    BOOK_IDLE = 0,      // Sách đóng
    BOOK_DROPPING,          // [MỚI] Sách đang rơi từ trên trời xuống
    BOOK_PAUSE_BEFORE_OPEN, // [MỚI] Khựng lại 1 nhịp sau khi chạm bàn
    BOOK_OPEN_RIGHT,    // GĐ 1: Bìa phải đang lật về gáy
    BOOK_HALF_OPEN,     // GĐ 1 Xong: Lộ trang phải, chờ bấm Tab
    BOOK_OPEN_LEFT,     // GĐ 2: Bìa trái đang mở ra
    BOOK_FULLY_OPEN,    // GĐ 2 Xong: Mở toang, hiện chữ
    BOOK_TURN_NEXT,     // [THÊM MỚI] Lật sang trang sau
    BOOK_TURN_PREV,     // [THÊM MỚI] Lật lùi trang trước
    BOOK_CLOSE_LEFT,    // GĐ 3: Bìa trái đang gập lại
    BOOK_CLOSE_RIGHT,    // GĐ 4: Bìa phải đang gập lại (Đóng hoàn toàn)
    BOOK_EXITING        // [MỚI] Sách gập xong và đang rớt xuống
} BookAnimState;

static BookAnimState bookState = BOOK_IDLE;
static float bookProgress = 0.0f; // Chạy từ 0.0 đến 1.0
static float bookAnimSpeed = 2.0f; // Tốc độ lật sách (Tăng số để lật nhanh hơn)

// --- Biến cho hiệu ứng Rơi ---
static float bookDropYOffset = 0.0f;   // Tọa độ lệch khi rơi
static float bookDropVelocity = 0.0f;  // Vận tốc rơi (gia tốc trọng trường)
static float bookPauseTimer = 0.0f;    // Đếm thời gian khựng

// --- [NEW] HỆ THỐNG ANIMATION CHO TIÊU ĐỀ MENU ---
static Texture2D texTitleFrame; // Biến chứa ảnh khung tiêu đề
static float titleYOffset = -600.0f; // Bắt đầu tít trên trời
static float titleVelocity = 0.0f;   // Vận tốc rơi/bay của tiêu đề

typedef enum {
    TITLE_HIDDEN = 0,   // Đang giấu
    TITLE_DROPPING,     // Đang rơi xuống và nảy
    TITLE_VISIBLE,      // Đứng im trên trang sách
    TITLE_FLYING_UP     // Bay vút lên khi đóng sách
} TitleAnimState;

static TitleAnimState titleState = TITLE_HIDDEN;

static float buttonRevealProgress = 0.0f; // [MỚI] Tiến trình hiện nút (0.0 -> 1.0)
static bool isReadingInfo = false;        // [MỚI] Cờ đánh dấu đang đọc chữ hay ở trang Index

// [MỚI] Biến nhớ ID của thanh Slider đang được kéo
static int activeSliderIndex = -1;

// [NEW] Hàm tính toán màu sắc đổ bóng biến thiên theo tiến trình lật
Color GetPageFlipTint(float progress) {
    unsigned char brightness;
    if (progress < 0.5f) {
        // Nửa đầu: Tối dần từ 255 xuống 160 khi dựng đứng lên
        float p = progress * 2.0f;
        brightness = (unsigned char)(255 - (p * 95));
    } else {
        // Nửa sau: Sáng dần từ 160 lên 255 khi nằm phẳng xuống
        float p = (progress - 0.5f) * 2.0f;
        brightness = (unsigned char)(160 + (p * 95));
    }
    return (Color){ brightness, brightness, brightness, 255 };
}

// Hàm vẽ Icon Animation Gốc (Tái sử dụng cho mọi loại nút 4 frame)
void DrawAnimatedIconBase(Texture2D iconTexture, Rectangle destRec, bool isHover) {
    // Nếu chưa load ảnh thì không vẽ để tránh crash
    if (iconTexture.id == 0) return; 

    int totalFrames = 4;
    int currentFrame = 0;

    // Tốc độ nhấp nháy: Số càng to chạy càng nhanh (ví dụ: 8 frame / 1 giây)
    float animationSpeed = 8.0f; 

    // Nếu lướt chuột qua thì chạy animation
    if (isHover) {
        currentFrame = (int)(GetTime() * animationSpeed) % totalFrames;
    } 
    // Nếu rút chuột ra thì trả về khung hình đầu tiên (đứng im)
    else {
        currentFrame = 0;
    }

    // Tính toán kích thước của 1 frame (chia đều chiều ngang ảnh cho 4)
    float frameWidth = (float)iconTexture.width / totalFrames;
    float frameHeight = (float)iconTexture.height;

    // Khung cắt ảnh trên Spritesheet
    Rectangle sourceRec = { currentFrame * frameWidth, 0.0f, frameWidth, frameHeight };

    // Vẽ và ép kích thước (Scale) vừa khít với hitbox (destRec) mà bạn quét bằng Tool
    DrawTexturePro(iconTexture, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);
}

// [HÀM HỖ TRỢ ĐỘC LẬP]: Vẽ 1 khung ảnh bất kỳ từ một Spritesheet
// Ví dụ: spritesheet có 4 khung (Normal, Hover, Pressed, Disabled)

void DrawCustomUIButton(MenuButton *b, bool isHover) {
    // 1. TỌA ĐỘ CẮT ẢNH TỪ SPRITESHEET 
    // Tọa độ cắt Khung Frame bình thường (X, Y, Rộng, Cao)
    Rectangle sourceNormal = { 161, 305, 62, 14 }; 
    // Tọa độ cắt Khung Frame khi lướt chuột qua (Sáng hơn)
    Rectangle sourceHover = { 161, 321, 62, 14 };  

    Rectangle activeSource = isHover ? sourceHover : sourceNormal;

    // 2. KÉO GIÃN KHUNG BẰNG 9-SLICE (Giữ viền không bị méo)
    NPatchInfo nPatch = {
        .source = activeSource,
        .left = 4, .top = 4, .right = 4, .bottom = 4, // Độ dày của mép viền (Đo xem viền nút pixel dày mấy px thì điền vào)
        .layout = NPATCH_NINE_PATCH
    };
    
    // Vẽ nền nút
    DrawTextureNPatch(texUIBook, nPatch, b->rect, (Vector2){0,0}, 0.0f, WHITE);

    // 3. HIỆU ỨNG 4 GÓC CHỈ VÀO NÚT (ANIMATION PHÌNH RA THU VÀO SÁT MÉP)
    if (isHover) {
        // Toán học: sinf chạy từ -1 đến 1. Biến đổi để offset dao động từ 1.0px đến 5.0px ra ngoài viền
        float offset = 1.0f + (sinf(GetTime() * 8.0f) + 1.0f) * 2.0f;

        // Tọa độ cắt 4 cái góc vuông nhỏ xíu ở mục "Select" trên ảnh (Bạn tự đo và điền)
        Rectangle srcTL = { 33, 369, 6, 6 }; // Góc Trái Trên (Top-Left)
        Rectangle srcTR = { 57, 369, 6, 6 }; // Góc Phải Trên (Top-Right)
        Rectangle srcBL = { 33, 393, 6, 6 }; // Góc Trái Dưới (Bottom-Left)
        Rectangle srcBR = { 57, 393, 6, 6 }; // Góc Phải Dưới (Bottom-Right)

        float cw = 6.0f; // Chiều rộng của 1 góc (tương ứng lúc cắt)
        float ch = 6.0f; // Chiều cao của 1 góc

        // Vẽ 4 góc ôm lấy hitbox của nút + khoảng cách offset đang đập (breathing)
        DrawTexturePro(texUIBook, srcTL, (Rectangle){b->rect.x - offset, b->rect.y - offset, cw, ch}, (Vector2){0,0}, 0.0f, WHITE);
        DrawTexturePro(texUIBook, srcTR, (Rectangle){b->rect.x + b->rect.width + offset - cw, b->rect.y - offset, cw, ch}, (Vector2){0,0}, 0.0f, WHITE);
        DrawTexturePro(texUIBook, srcBL, (Rectangle){b->rect.x - offset, b->rect.y + b->rect.height + offset - ch, cw, ch}, (Vector2){0,0}, 0.0f, WHITE);
        DrawTexturePro(texUIBook, srcBR, (Rectangle){b->rect.x + b->rect.width + offset - cw, b->rect.y + b->rect.height + offset - ch, cw, ch}, (Vector2){0,0}, 0.0f, WHITE);
    }

    // 4. VẼ CHỮ TIẾNG VIỆT LÊN TRÊN CÙNG
   if (b->label[0] != '\0') {
        Vector2 textSize = MeasureTextEx(globalFont, b->label, 20, 1);
        Vector2 textPos = { 
            b->rect.x + (b->rect.width - textSize.x) / 2.0f, 
            b->rect.y + (b->rect.height - textSize.y) / 2.0f 
        };
        
        // [TỰ ĐỊNH NGHĨA MÀU RGB LẤY TỪ SPRITESHEET]
        // Cú pháp: (Color){ Red, Green, Blue, Alpha(Độ đục) }
        Color normalTextColor = (Color){ 246, 227, 180, 255 }; // Màu Vàng be nhạt (Khớp với nền nút lúc Hover)
        Color hoverTextColor  = (Color){ 94,  65,  56,  255 }; // Màu Nâu đậm (Khớp với nền nút lúc Bình thường)
        
        Color activeTextColor = isHover ? hoverTextColor : normalTextColor;

        DrawTextEx(globalFont, b->label, textPos, 20, 1, activeTextColor);
    }
}

// --- [RESTORE] HÀM VẼ CŨ (Dùng riêng cho Character Select) ---
// Chúng ta cần hàm này vì màn hình chọn tướng dùng logic vẽ đặc biệt
static bool DrawCharSelectButton(const char* debugName, Rectangle rec) {
    bool clicked = false;
    Vector2 mousePoint = GetVirtualMousePos(); 
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    if (isHover) {
        // Hiệu ứng nhấp nháy khi hover
        float time = (float)GetTime();
        DrawRectangleLinesEx(rec, 3.0f, Fade(GOLD, 0.5f + (sinf(time * 6.0f) + 1.0f) * 0.25f)); 
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) clicked = true;
    }
    
    // Debug (Chỉ hiện khi bật tool)
    if (IsMenuDebugActive()) {
        DrawRectangleLinesEx(rec, 1.0f, RED);
        DrawText(debugName, (int)rec.x, (int)rec.y - 15, 10, RED);
    }
    return clicked;
}

static bool DrawOldButton(const char* text, Rectangle rec) {
    bool clicked = false;
    Vector2 mousePoint = GetVirtualMousePos(); 
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    // Vẽ nền
    if (isHover) {
        DrawRectangleRec(rec, Fade(BLUE, 0.8f));
        DrawRectangleLinesEx(rec, 2.0f, YELLOW);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) clicked = true;
    } else {
        DrawRectangleLinesEx(rec, 2.0f, WHITE);
    }
    
    int textW = MeasureText(text, 20);
    DrawText(text, (int)(rec.x + (rec.width - textW)/2), (int)(rec.y + (rec.height - 20)/2), 20, isHover ? YELLOW : WHITE);
    
    return clicked;
}

// --- HỆ THỐNG MENU ĐỘNG (FILE LOADING) ---
const char* GetMenuSectionName(MenuType type) {
    switch(type) {
        case MENU_TITLE: return "[TITLE]";
        case MENU_PAUSE: return "[PAUSE]";
        case MENU_INVENTORY: return "[INVENTORY]";
        // [MOI] Thêm Settings để hệ thống biết tên section trong file
        case MENU_SETTINGS: return "[SETTINGS]";
        case MENU_CHARACTER_SELECT: return "[CHAR_SELECT]";
        case MENU_CHARACTER_PROFILE: return "[CHAR_PROFILE]";
        case MENU_INFO: return "[INFO]";
        default: return NULL;
    }
}

const char* GetActionName(int actionID) {
    switch(actionID) {
        case ACT_START_GAME: return "START_GAME";
        case ACT_CONTINUE: return "CONTINUE";
        case ACT_OPEN_SETTINGS: return "OPEN_SETTINGS";
        case ACT_EXIT_GAME: return "EXIT_GAME";
        case ACT_RESUME: return "RESUME";
        case ACT_SAVE_GAME: return "SAVE_GAME";
        case ACT_QUIT_TO_TITLE: return "QUIT_TO_TITLE";
        case ACT_OPEN_INVENTORY: return "OPEN_INVENTORY";
        case ACT_INV_PREV_PAGE: return "INV_PREV";
        case ACT_INV_NEXT_PAGE: return "INV_NEXT";
        case ACT_INV_USE: return "INV_USE";
        case ACT_INV_UNSELECT: return "INV_CANCEL";
        case ACT_INV_CLOSE: return "INV_CLOSE";
        case ACT_OPEN_INFO: return "OPEN_INFO";
        case ACT_TAB_DEVLOG: return "TAB_DEVLOG";
        case ACT_TAB_CREDIT: return "TAB_CREDIT";
        case ACT_CLOSE_INFO: return "CLOSE_INFO";

        case ACT_INFO_NEXT_PAGE: return "INFO_NEXT";
        case ACT_INFO_PREV_PAGE: return "INFO_PREV";
        case ACT_INFO_BACK_INDEX: return "INFO_BACK";

        // [MOI] Tên hành động cho Settings (để hiện debug)
        case ACT_SET_MASTER_VOL: return "SET_MASTER_VOL";
        case ACT_SET_MUSIC_VOL:  return "SET_MUSIC_VOL";
        case ACT_SET_SFX_VOL:    return "SET_SFX_VOL";
        case ACT_TOGGLE_MUTE:    return "TOGGLE_MUTE";
        case ACT_TOGGLE_MUSIC_MUTE: return "TOGGLE_MUSIC_MUTE"; // [NEW]
        case ACT_TOGGLE_SFX_MUTE:   return "TOGGLE_SFX_MUTE";   // [NEW]
        case ACT_TOGGLE_FULLSCREEN: return "TOGGLE_FULLSCREEN";
        case ACT_SEL_CLASS_1:  return "SEL_CLASS_1";
        case ACT_SEL_CLASS_2:  return "SEL_CLASS_2";
        case ACT_SEL_CLASS_3:  return "SEL_CLASS_3";
        case ACT_SEL_CLASS_4:  return "SEL_CLASS_4";
        case ACT_CONFIRM_CHAR: return "CONFIRM_CHAR";
        case ACT_PROFILE_START_GAME: return "PROF_START_GAME";
        case ACT_PROFILE_BACK: return "PROF_BACK";             
        default: return "UNKNOWN";
    }
}

void Menu_LoadLayout(MenuType type) {
    currentBtnCount = 0;
    const char* section = GetMenuSectionName(type);
    if (!section) return;
    FILE *f = fopen("resources/data/menus.txt", "r");
    if (!f) { printf("ERR: Khong thay file menus.txt\n"); return; }

    char line[256];
    bool readingSection = false;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '[') {
            if (strstr(line, section)) readingSection = true;
            else readingSection = false;
            continue;
        }

        if (readingSection && strncmp(line, "BUTTON", 6) == 0) {
            if (currentBtnCount >= MAX_MENU_BUTTONS) break;
            MenuButton *b = &currentButtons[currentBtnCount];
            int invisiTmp, typeTmp; // [MOI] Biến tạm để đọc type
            
            // [MOI] Cập nhật sscanf để đọc thêm type và value
            sscanf(line, "BUTTON %d %f %f %f %f %d %d %f %[^\r\n]", 
                &b->actionID, &b->rect.x, &b->rect.y, &b->rect.width, &b->rect.height, 
                &invisiTmp, &typeTmp, &b->value, b->label);
            
            b->isInvisible = (bool)invisiTmp;
            b->type = (ControlType)typeTmp; // [MOI] Gán loại control
            currentBtnCount++;
        }
    }
    fclose(f);
}

void Menu_SaveLayout() {
    FILE *fRead = fopen("resources/data/menus.txt", "r");
    if (!fRead) return;

    char fileContent[10000] = {0}; 
    char line[256];
    bool skipOldSection = false;
    const char* currentSectionName = GetMenuSectionName(currentMenu);
    
    // Nếu menu hiện tại không hỗ trợ lưu (vd Character Select), thoát
    if (!currentSectionName) {
        printf(">> Cannot save layout for this menu type.\n");
        fclose(fRead);
        return;
    }

    while (fgets(line, sizeof(line), fRead)) {
        if (line[0] == '[') {
            if (strstr(line, currentSectionName)) {
                skipOldSection = true; 
            } else {
                skipOldSection = false;
            }
        }

        if (!skipOldSection || strncmp(line, "BUTTON", 6) != 0) {
            if (skipOldSection && line[0] == '[') {
                strcat(fileContent, line); 
                for (int i = 0; i < currentBtnCount; i++) {
                    MenuButton *b = &currentButtons[i];
                    char buffer[256];
                    // [MOI] Cập nhật sprintf để ghi thêm type và value
                    sprintf(buffer, "BUTTON %d %.0f %.0f %.0f %.0f %d %d %.2f %s\n", 
                        b->actionID, b->rect.x, b->rect.y, b->rect.width, b->rect.height, 
                        (int)b->isInvisible, (int)b->type, b->value, b->label);
                    strcat(fileContent, buffer);
                }
            } else {
                strcat(fileContent, line); 
            }
        }
    }
    fclose(fRead);

    FILE *fWrite = fopen("resources/data/menus.txt", "w");
    if (fWrite) {
        fputs(fileContent, fWrite);
        fclose(fWrite);
        printf(">> SAVED LAYOUT TO FILE!\n");
    }
}

void Menu_SetDebugMode(bool enabled) { isDebugOverlay = enabled; }

// --- LOGIC XỬ LÝ HÀNH ĐỘNG NÚT ĐỘNG ---
void ProcessButtonAction(int actionID) {
    switch (actionID) {
        case ACT_START_GAME:
            Debug_ForceCloseMenuTool();
            StoryCutscene_Start();
            //Menu_SwitchTo(MENU_CHARACTER_SELECT);
            //selectedCharIndex = -1;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_CONTINUE:
            if (Game_HasSaveFile()) {
                Audio_PlaySoundEffect(SFX_UI_CLICK);
                printf(">> [MENU] Loading Game...\n");
                
                Gameplay_LoadGame(); // Tự động nạp Class, vị trí, cốt truyện...
                
                Menu_SwitchTo(MENU_NONE); // Tắt menu để vào game
            }
            break;
        case ACT_OPEN_INVENTORY: // [THÊM MỚI] Logic lật sách sang trái để mở túi đồ
            previousMenu = currentMenu; 
            Menu_SwitchTo(MENU_INVENTORY);
            if (previousMenu == MENU_PAUSE) {
                bookState = BOOK_OPEN_LEFT; // Đang ở nửa trang (Pause), lật tiếp sang trái
                bookProgress = 0.0f;
            }
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_OPEN_SETTINGS:
            previousMenu = currentMenu; 
            Menu_SwitchTo(MENU_SETTINGS);
            // Nếu mở từ Title: Sách rơi từ trên xuống
            if (previousMenu == MENU_TITLE) {
                bookState = BOOK_DROPPING; 
                bookDropYOffset = -((float)SCREEN_HEIGHT + 200.0f);
                bookDropVelocity = 0.0f;
                bookProgress = 0.0f;
            } 
            // Nếu mở từ Pause: Sách đang mở 1 nửa, lật tiếp sang trái
            else if (previousMenu == MENU_PAUSE) {
                bookState = BOOK_OPEN_LEFT; 
                bookProgress = 0.0f;
            }
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_EXIT_GAME:
            Transition_StartExit();
            break;
        case ACT_RESUME:
            // Bấm Resume -> Gập sách lại rồi thoát
            if (bookState == BOOK_HALF_OPEN) {
                bookState = BOOK_CLOSE_RIGHT;
                bookProgress = 0.0f;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        case ACT_SAVE_GAME:
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            
            Gameplay_SaveGame(); // Tự động lưu tất cả mọi thứ hiện tại
            break;
        case ACT_QUIT_TO_TITLE:
            Transition_StartToTitle();
            break;
        case ACT_INV_PREV_PAGE:
            Inventory_PrevPage();
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_INV_NEXT_PAGE:
            Inventory_NextPage();
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_INV_USE:
            Inventory_UseSelected();
            break;
        case ACT_INV_UNSELECT:
            Inventory_Unselect();
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        case ACT_INV_CLOSE:
            if (bookState == BOOK_FULLY_OPEN) {
                bookState = BOOK_CLOSE_LEFT; // Gập bìa trái lại về trang Pause
                bookProgress = 0.0f;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        case ACT_SEL_CLASS_1: selectedCharIndex = 0; Audio_PlaySoundEffect(SFX_UI_CLICK); break;
        case ACT_SEL_CLASS_2: selectedCharIndex = 1; Audio_PlaySoundEffect(SFX_UI_CLICK); break;
        case ACT_SEL_CLASS_3: selectedCharIndex = 2; Audio_PlaySoundEffect(SFX_UI_CLICK); break;
        case ACT_SEL_CLASS_4: selectedCharIndex = 3; Audio_PlaySoundEffect(SFX_UI_CLICK); break;
        case ACT_CONFIRM_CHAR:
            if (selectedCharIndex != -1 && !isGameStarting) {
                // Chuyển sang màn hình Profile thay vì vào game ngay
                Menu_SwitchTo(MENU_CHARACTER_PROFILE);
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        // [NEW] Xử lý nút của màn hình Profile
        case ACT_PROFILE_START_GAME:
            if (!isGameStarting) {
                isGameStarting = true; // Bắt đầu hiệu ứng Zoom Aura
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        case ACT_PROFILE_BACK:
            if (!isGameStarting) {
                Menu_SwitchTo(MENU_CHARACTER_SELECT); // Lùi lại
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        case ACT_OPEN_INFO:
            previousMenu = currentMenu; // Lưu lại menu trước đó (Title hoặc Pause)
            Menu_SwitchTo(MENU_INFO);

            if (previousMenu == MENU_TITLE) {
                // Kích hoạt Rơi từ trên không
                bookState = BOOK_DROPPING; 
                bookDropYOffset = -((float)SCREEN_HEIGHT + 200.0f); 
                bookDropVelocity = 0.0f; 
                bookProgress = 0.0f;
            } else if (previousMenu == MENU_PAUSE) {
                // Mở từ Pause -> Sách đang mở sẵn nửa trang rồi, nhảy luôn vào việc
                bookState = BOOK_HALF_OPEN;
                bookProgress = 0.0f;
            }

            bookProgress = 0.0f;
            buttonRevealProgress = 0.0f; // Reset tiến trình hiện nút
            isReadingInfo = false;       // Bắt đầu ở trang Index
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            break;
        // Logic cho 2 nút Tab (Tạm thời chơi âm thanh, nội dung chữ ta sẽ xử lý hiện theo Action này sau)
        case ACT_TAB_DEVLOG:
            Info_SetTab(TAB_DEVLOG);
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            if (bookState == BOOK_HALF_OPEN) {
                bookState = BOOK_OPEN_LEFT; // Bắt đầu lật bìa trái
                bookProgress = 0.0f;
                Info_StartReveal();   // <-- [THÊM] Kích hoạt viết chữ
                isReadingInfo = true; // Chuyển sang chế độ đọc chữ
            }
            break;
        case ACT_TAB_CREDIT:
            Info_SetTab(TAB_CREDIT);
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            if (bookState == BOOK_HALF_OPEN) {
                bookState = BOOK_OPEN_LEFT; // Bắt đầu lật bìa trái
                bookProgress = 0.0f;
                Info_StartReveal();   // <-- [THÊM] Kích hoạt viết chữ
                isReadingInfo = true; // Chuyển sang chế độ đọc chữ
            }
            break;
        case ACT_INFO_BACK_INDEX:
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            if (bookState == BOOK_FULLY_OPEN) {
                bookState = BOOK_CLOSE_LEFT; // Gập trang trái lại
                bookProgress = 0.0f;
                isReadingInfo = false; // Trở về chế độ Index
                buttonRevealProgress = 0.0f; // Chuẩn bị vẽ lại nút từ đầu
            }
            break;
       case ACT_CLOSE_INFO:
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            if (bookState == BOOK_FULLY_OPEN) {
                bookState = BOOK_CLOSE_LEFT; 
                bookProgress = 0.0f;
            } else if (bookState == BOOK_HALF_OPEN) {
                if (previousMenu == MENU_PAUSE) {
                    Menu_SwitchTo(MENU_PAUSE); // Từ Pause vào thì khi đóng chỉ lùi về Pause (sách giữ nguyên mở nửa)
                } else {
                    bookState = BOOK_CLOSE_RIGHT; // Từ Title vào thì phải gập sách hoàn toàn
                    bookProgress = 0.0f;
                }
            } else {
                Menu_SwitchTo(previousMenu); // Fallback an toàn
            }
            break;
        case ACT_INFO_NEXT_PAGE:
           if (bookState == BOOK_FULLY_OPEN) {
                bookState = BOOK_TURN_NEXT; // Kích hoạt Animation lật trang
                bookProgress = 0.0f;
                Audio_PlaySoundEffect(SFX_UI_CLICK); 
            }
            break;
        case ACT_INFO_PREV_PAGE:
           if (bookState == BOOK_FULLY_OPEN) {
                bookState = BOOK_TURN_PREV; // Kích hoạt Animation lùi trang
                bookProgress = 0.0f;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
        // Các Action của Settings sẽ được xử lý realtime bên dưới (trong phần Slider/Toggle logic)
    }
}

// --- UPDATE ---
void Menu_Update() {
    if (IsKeyPressed(KEY_F11)) ToggleGameFullscreen();
    extern bool Gameplay_IsEnding();
   if (currentMenu == MENU_NONE) {
        if (Inventory_IsActive()) return;
        // Bấm ESC trong game -> Mở Menu Pause, Sách rơi xuống
        if (IsKeyPressed(KEY_ESCAPE) && !Gameplay_IsEnding()) {
            Menu_SwitchTo(MENU_PAUSE);
            bookState = BOOK_DROPPING; 
            bookDropYOffset = -((float)SCREEN_HEIGHT + 200.0f);
            bookDropVelocity = 0.0f;
            bookProgress = 0.0f;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }
        return;
    }

    // Bấm ESC ở Menu Pause -> Gập bìa phải lại rồi thoát
    if (currentMenu == MENU_PAUSE && IsKeyPressed(KEY_ESCAPE)) {
        if (bookState == BOOK_HALF_OPEN) {
            bookState = BOOK_CLOSE_RIGHT;
            bookProgress = 0.0f;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }
    }
    // Bấm ESC ở Settings -> Chỉ gập trang trái lại
    if (currentMenu == MENU_SETTINGS && IsKeyPressed(KEY_ESCAPE)) {
        if (bookState == BOOK_FULLY_OPEN) {
            bookState = BOOK_CLOSE_LEFT;
            bookProgress = 0.0f;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }
    }
    if (currentMenu == MENU_INVENTORY && IsKeyPressed(KEY_ESCAPE)) {
        if (bookState == BOOK_FULLY_OPEN) {
            bookState = BOOK_CLOSE_LEFT;
            bookProgress = 0.0f;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }
    }
    if (currentMenu == MENU_INVENTORY) {
        if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) Menu_SwitchTo(MENU_NONE);
    }

    // 1. LOGIC NÚT ĐỘNG (Title, Pause, Inventory, [MOI] SETTINGS)
    // [MOI] Thêm MENU_SETTINGS vào điều kiện
  if (currentMenu == MENU_TITLE || currentMenu == MENU_PAUSE || currentMenu == MENU_INVENTORY || currentMenu == MENU_SETTINGS || currentMenu == MENU_CHARACTER_SELECT
    || currentMenu == MENU_CHARACTER_PROFILE || currentMenu == MENU_INFO) {
        
        // [SỬA LỖI NGHIÊM TRỌNG]: Không dùng return; vì nó sẽ làm kẹt Animation!
        bool canInteract = true;
        if ((currentMenu == MENU_INFO || currentMenu == MENU_SETTINGS || currentMenu == MENU_PAUSE || currentMenu == MENU_INVENTORY) && 
            bookState != BOOK_HALF_OPEN && bookState != BOOK_FULLY_OPEN) {
            canInteract = false; 
        }

        if (canInteract) {
            Vector2 mouse = GetVirtualMousePos();
            bool anyHover = false;
            // [MỚI] Reset trạng thái khi người chơi thả chuột trái ra
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                activeSliderIndex = -1;
            }
            for (int i = 0; i < currentBtnCount; i++) {
            MenuButton *b = &currentButtons[i];
            
            if (currentMenu == MENU_INFO && !isDebugOverlay) {
                bool isTabButton = (b->actionID == ACT_TAB_DEVLOG || b->actionID == ACT_TAB_CREDIT);
                bool isReadButton = (b->actionID == ACT_INFO_NEXT_PAGE || b->actionID == ACT_INFO_PREV_PAGE || b->actionID == ACT_INFO_BACK_INDEX);

                if (!isReadingInfo && isReadButton) continue; // Ở trang Index -> Bỏ qua click chạm nút đọc chữ
                if (isReadingInfo && (isTabButton || b->actionID == ACT_CLOSE_INFO)) continue;   // [ĐÃ SỬA] Đang đọc chữ -> Bỏ qua Tab và Nút Đóng
            }

            if (!isDebugOverlay) {
                if (b->actionID == ACT_CONTINUE && !Game_HasSaveFile()) continue;
                if (b->actionID == ACT_INV_PREV_PAGE && Inventory_GetCurrentPage() <= 0) continue;
                if (b->actionID == ACT_INV_NEXT_PAGE && Inventory_GetCurrentPage() >= Inventory_GetMaxPages() - 1) continue;
                if ((b->actionID == ACT_INV_USE || b->actionID == ACT_INV_UNSELECT) && !Inventory_HasSelectedSlot()) continue;
            }

           // [SỬA LỖI] Ngăn các nút khác sáng lên nếu đang kéo Slider (activeSliderIndex != -1)
            if ((activeSliderIndex == -1 || activeSliderIndex == i) && CheckCollisionPointRec(mouse, b->rect)) {
                anyHover = true;
                if (lastHoveredBtn != b->label) {
                    Audio_PlaySoundEffect(SFX_UI_HOVER);
                    lastHoveredBtn = b->label;
                }
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

              // 1. BUTTON (Nút bấm thường và nút chính thức)
                if (b->type == CTRL_BUTTON || b->type == CTRL_CUSTOM_BTN || b->type == CTRL_ICON_BTN) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (!IsMenuDebugActive()) {
                            // [FIXED] Sửa lỗi click nút Back ở Settings không chịu gập sách
                            if (currentMenu == MENU_SETTINGS && b->actionID == ACT_INV_CLOSE) {
                                if (bookState == BOOK_FULLY_OPEN) {
                                    bookState = BOOK_CLOSE_LEFT; // Bắt đầu gập bìa trái
                                    bookProgress = 0.0f;
                                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                                }
                            } else {
                                ProcessButtonAction(b->actionID);
                            }
                        }
                    }
                }
                // 2. SLIDER (Chỉ Ghi nhận ID khi BẮT ĐẦU click chuột)
                else if (b->type == CTRL_SLIDER) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        activeSliderIndex = i; // Đánh dấu slider này đang được nắm giữ
                    }
                }
                // 3. TOGGLE (Công tắc - Bấm để đảo)
                else if (b->type == CTRL_TOGGLE) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                         b->value = (b->value > 0.5f) ? 0.0f : 1.0f;
                         Audio_PlaySoundEffect(SFX_UI_CLICK);
                         if (!IsMenuDebugActive()) {
                             if (b->actionID == ACT_TOGGLE_MUTE) Audio_ToggleMute();
                             if (b->actionID == ACT_TOGGLE_MUSIC_MUTE) Audio_ToggleMusicMute(); // [NEW] Tắt/bật Nhạc
                             if (b->actionID == ACT_TOGGLE_SFX_MUTE) Audio_ToggleSFXMute();     // [NEW] Tắt/bật Tiếng động
                             if (b->actionID == ACT_TOGGLE_FULLSCREEN) ToggleGameFullscreen();
                         }
                    }
                }
            } // <--- ĐÓNG NGOẶC CỦA LỆNH CheckCollisionPointRec

            // ==============================================================
            // [MỚI] XỬ LÝ KÉO SLIDER ĐỘC LẬP (NẰM NGOÀI KHỐI CHECK VA CHẠM)
            // Kể cả khi chuột văng ra ngoài hitbox, nó vẫn tính toán bình thường
            // ==============================================================
            if (b->type == CTRL_SLIDER && activeSliderIndex == i) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    anyHover = true; // Cưỡng chế giữ icon con trỏ chuột dạng bàn tay
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                    // Cập nhật % liên tục
                    float mouseX = mouse.x - b->rect.x;
                    b->value = mouseX / b->rect.width;
                    if (b->value < 0.0f) b->value = 0.0f;
                    if (b->value > 1.0f) b->value = 1.0f;
                    
                    // Gửi lệnh thay đổi âm thanh realtime
                    if (!IsMenuDebugActive()) {
                        if (b->actionID == ACT_SET_MASTER_VOL) Audio_SetMasterVolume(b->value);
                        if (b->actionID == ACT_SET_MUSIC_VOL) Audio_SetMusicVolume(b->value);
                        if (b->actionID == ACT_SET_SFX_VOL)    Audio_SetSFXVolume(b->value);
                    }
                }
            }
        }
        if (!anyHover) {
             lastHoveredBtn = NULL;
             SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }
}
    
    if (currentMenu == MENU_CHARACTER_PROFILE && selectedCharIndex != -1 && !isGameStarting) {
        const char *fullProfileText = "";
        //Hiển thị câu mô ta cho từng class của menu_profileb
        if (selectedCharIndex == 0) fullProfileText = "BỘ PC1\n\n366363636363636363\n KẺ HỦY DIỆT OSG\nKỹ năng: ẢO MA CANADA";
        else if (selectedCharIndex == 1) fullProfileText = "BỘ PC2\n\nSức mạnh nghệ tuyệt đối.\nVũ khí: Củ nghệ.\nLối chơi: không biết";
        else if (selectedCharIndex == 2) fullProfileText = "BỘ PC3\n\nĐiều khiển tri thức cổ xưa.\nVũ khí: Gậy phép.\nLối chơi: Sát thương diện rộng.";
        else if (selectedCharIndex == 3) fullProfileText = "BỘ PC4\n\nNhãn quan của loài đại bàng.\nVũ khí: Cung tên.\nLối chơi: Tiêu diệt từ xa.";

        if (!isTextFinished) {
            framesCounterText++;
            if (framesCounterText >= 2) { 
                framesCounterText = 0;
                if (letterCount < (int)strlen(fullProfileText)) {
                    int bytes = 1;
                    unsigned char c = (unsigned char)fullProfileText[letterCount];
                    if (c >= 0 && c <= 127) bytes = 1;
                    else if ((c & 0xE0) == 0xC0) bytes = 2;
                    else if ((c & 0xF0) == 0xE0) bytes = 3;
                    else if ((c & 0xF8) == 0xF0) bytes = 4;
                    
                    for (int i = 0; i < bytes; i++) {
                        displayedText[letterCount] = fullProfileText[letterCount];
                        letterCount++;
                    }
                    displayedText[letterCount] = '\0';
                    if (letterCount % 3 == 0) Audio_PlaySoundEffect(SFX_UI_HOVER);
                } else isTextFinished = true;
            }
        }
    }

    // 2. [RESTORE] LOGIC UPDATE RIÊNG CHO CHARACTER SELECT
    // Đây là phần bạn bị thiếu khiến nút ESC không hoạt động
    if (currentMenu == MENU_CHARACTER_SELECT || currentMenu == MENU_CHARACTER_PROFILE) {
       if (IsKeyPressed(KEY_ESCAPE)) {
            if (currentMenu == MENU_CHARACTER_PROFILE && !isGameStarting) {
                Menu_SwitchTo(MENU_CHARACTER_SELECT); // ESC ở Profile -> lùi về Select
            }
            else if (selectedCharIndex != -1) {
                selectedCharIndex = -1;
            } else {
                Menu_SwitchTo(MENU_TITLE);
            }
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }

        for(int i = 0; i < 4; i++) {
            previewChars[i].framesCounter++;
            if (previewChars[i].framesCounter >= previewChars[i].framesSpeed) {
                previewChars[i].framesCounter = 0;
                previewChars[i].currentFrame++;
                if (previewChars[i].currentFrame >= previewChars[i].maxFrames) {
                    previewChars[i].currentFrame = 0;
                }
            }
            // Chỉ định cắt khung ảnh tĩnh (Idle - Hướng xuống)
            previewChars[i].frameRec.x = (float)(previewChars[i].currentFrame * previewChars[i].spriteWidth); 
            previewChars[i].frameRec.y = 0.0f; // Luôn quay mặt xuống
            previewChars[i].frameRec.width = (float)previewChars[i].spriteWidth;
            previewChars[i].frameRec.height = (float)previewChars[i].spriteHeight;
        }
    }
   // [SỬA] Cho phép INFO, SETTINGS và PAUSE dùng chung hệ thống Animation
    if (currentMenu == MENU_INFO || currentMenu == MENU_SETTINGS || currentMenu == MENU_PAUSE || currentMenu == MENU_INVENTORY) {
        
        // Cập nhật thanh cuộn chuột (Chỉ dành cho Info)
        if (currentMenu == MENU_INFO && bookState == BOOK_FULLY_OPEN) {
            Info_Update(); 
        }
        if (bookState == BOOK_HALF_OPEN && buttonRevealProgress < 1.0f) {
            buttonRevealProgress += 2.0f * GetFrameTime(); // Tốc độ lộ nút (tăng số để nhanh hơn)
            if (buttonRevealProgress > 1.0f) buttonRevealProgress = 1.0f;
        }

        // --- 1. LOGIC RƠI VÀ NẢY (TRỌNG LỰC) ---
        if (bookState == BOOK_DROPPING) {
            bookDropVelocity += 4000.0f * GetFrameTime(); // Trọng lực hút xuống (Chỉnh số này để rơi nhanh/chậm)
            bookDropYOffset += bookDropVelocity * GetFrameTime();
            
            // Nếu chạm đích (Offset >= 0 là chạm mặt bàn tĩnh)
            if (bookDropYOffset >= 0.0f) {
                bookDropYOffset = 0.0f;
                bookDropVelocity *= -0.3f; // Nảy ngược lại (0.3 = Nảy cao 30% lực)
                
                // Nếu lực nảy còn quá yếu (gần như đứng im) thì dừng hẳn
                if (fabs(bookDropVelocity) < 100.0f) {
                    bookState = BOOK_PAUSE_BEFORE_OPEN;
                    bookPauseTimer = 0.0f;
                    // (Tùy chọn) Thêm tiếng "Cộp" chạm bàn ở đây
                }
            }
        }
        // --- 2. LOGIC KHỰNG LẠI ---
        else if (bookState == BOOK_PAUSE_BEFORE_OPEN) {
            bookPauseTimer += GetFrameTime();
            if (bookPauseTimer > 0.4f) { // THỜI GIAN SÁCH LƠ LỬNG
                bookState = BOOK_OPEN_RIGHT; // Hết giờ thì tự động mở lật bìa
                bookProgress = 0.0f;
            }
        }

        // Chạy Animation Progress
        else if (bookState == BOOK_OPEN_RIGHT || bookState == BOOK_OPEN_LEFT || 
            bookState == BOOK_CLOSE_LEFT || bookState == BOOK_CLOSE_RIGHT ||
            bookState == BOOK_TURN_NEXT || bookState == BOOK_TURN_PREV) {
            
            bookProgress += bookAnimSpeed * GetFrameTime();
            
            if (bookProgress >= 1.0f) {
                bookProgress = 1.0f; // Khóa max
                
              // Chuyển State khi chạy xong 1 Phase
                if (bookState == BOOK_OPEN_RIGHT) {
                    // Nếu mở Settings từ Title -> Tự lật thẳng sang trái
                    if (currentMenu == MENU_SETTINGS && previousMenu == MENU_TITLE) {
                        bookState = BOOK_OPEN_LEFT; 
                        bookProgress = 0.0f;
                    } else {
                        // Pause hoặc Info -> Dừng chờ ở 1 trang (Mở nửa)
                        bookState = BOOK_HALF_OPEN; 
                    }
                }
                else if (bookState == BOOK_OPEN_LEFT) bookState = BOOK_FULLY_OPEN;
              else if (bookState == BOOK_CLOSE_LEFT) {
                    if (currentMenu == MENU_SETTINGS) {
                        if (previousMenu == MENU_PAUSE) {
                            bookState = BOOK_HALF_OPEN;
                            Menu_SwitchTo(MENU_PAUSE);
                        } else {
                            bookState = BOOK_CLOSE_RIGHT; 
                            bookProgress = 0.0f;
                        }
                    }
                    // --- [THÊM MỚI] KHI GẬP TÚI ĐỒ XONG THÌ LÙI VỀ PAUSE ---
                    else if (currentMenu == MENU_INVENTORY) {
                        bookState = BOOK_HALF_OPEN;
                        Menu_SwitchTo(MENU_PAUSE);
                    }
                    else if (currentMenu == MENU_INFO) {
                        if (!isReadingInfo) {
                            bookState = BOOK_HALF_OPEN;   // Bấm Back Index -> Lùi về trang nửa
                        } else {
                            // Bấm Close khi đang đọc (Mở toang)
                            if (previousMenu == MENU_PAUSE) {
                                bookState = BOOK_HALF_OPEN; // Về Pause thì nằm chờ ở mở nửa
                                Menu_SwitchTo(MENU_PAUSE);
                            } else {
                                bookState = BOOK_CLOSE_RIGHT; // Về Title thì gập sách lại
                                bookProgress = 0.0f;
                            }
                        }
                    }
                }
                else if (bookState == BOOK_CLOSE_RIGHT) {
                    bookState = BOOK_EXITING; 
                    bookDropVelocity = 0.0f; 
                }
                else if (bookState == BOOK_TURN_NEXT) {
                    Info_NextPage();         // Chuyển dữ liệu sang trang mới
                    Info_StartReveal();      // Reset biến để chữ bắt đầu hiện ra
                    bookState = BOOK_FULLY_OPEN; // Mở sách ra bình thường
                }
                else if (bookState == BOOK_TURN_PREV) {
                    Info_PrevPage();         // Lùi dữ liệu về trang cũ
                    Info_StartReveal();      
                    bookState = BOOK_FULLY_OPEN;
                }
            }
        }
      else if (bookState == BOOK_EXITING) {
            bookDropVelocity += 4000.0f * GetFrameTime(); 
            bookDropYOffset += bookDropVelocity * GetFrameTime();
            
            if (bookDropYOffset > (float)SCREEN_HEIGHT + 200.0f) {
                bookState = BOOK_IDLE;
                if (currentMenu == MENU_PAUSE) {
                    Menu_SwitchTo(MENU_NONE); // Trở về game
                } else {
                    Menu_SwitchTo(previousMenu); // Về Title
                }
            }
        }
    }
    // --- [NEW] LOGIC VẬT LÝ CHO TIÊU ĐỀ ---
        // 1. Kích hoạt Rơi khi sách đã mở xong (Nửa hoặc Toàn bộ)
        bool isBookOpen = (bookState == BOOK_HALF_OPEN || bookState == BOOK_FULLY_OPEN);
        if (isBookOpen && titleState == TITLE_HIDDEN) {
            titleState = TITLE_DROPPING;
            titleYOffset = -600.0f; // Kéo lên trời
            titleVelocity = 0.0f;
        }
        // 2. Kích hoạt Bay Lên khi sách bắt đầu đóng lại
        else if (!isBookOpen && (titleState == TITLE_VISIBLE || titleState == TITLE_DROPPING)) {
            titleState = TITLE_FLYING_UP;
            titleVelocity = -500.0f; // Tạo một lực đẩy vút lên ban đầu
        }

        // 3. Xử lý tính toán vật lý Realtime
        if (titleState == TITLE_DROPPING) {
            titleVelocity += 3500.0f * GetFrameTime(); // Trọng lực kéo xuống
            titleYOffset += titleVelocity * GetFrameTime();
            
            if (titleYOffset >= 0.0f) { // Chạm mốc tọa độ đích
                titleYOffset = 0.0f;
                titleVelocity *= -0.35f; // Nảy lại 35% lực
                
                // Nếu lực nảy còn quá yếu -> Đứng im
                if (fabs(titleVelocity) < 60.0f) titleState = TITLE_VISIBLE;
            }
        } 
        else if (titleState == TITLE_FLYING_UP) {
            titleVelocity -= 4500.0f * GetFrameTime(); // Gia tốc bay ngược lên vũ trụ
            titleYOffset += titleVelocity * GetFrameTime();
            
            if (titleYOffset < -600.0f) { // Bay khuất màn hình thì ẩn đi
                titleState = TITLE_HIDDEN;
            }
        }
}


// Hàm vẽ Nền Quyển Sách mở (Dùng 9-Slice cho cả 3 lớp)

// [NEW] Hàm Vẽ Tiêu Đề Động (Có Khung Ảnh + Căn Giữa Text + Nảy 3D)
void DrawDynamicMenuTitle(const char* text, Rectangle srcRec, float targetX, float targetY) {
    if (titleState == TITLE_HIDDEN) return; // Nếu đang giấu thì không vẽ

    // Tính toán tọa độ Y thực tế (đã cộng thêm hiệu ứng rơi/bay)
    float currentY = targetY + titleYOffset;

    // 1. Vẽ Khung ảnh (cắt theo tọa độ srcRec bạn cung cấp)
    Rectangle destRec = { targetX, currentY, srcRec.width, srcRec.height };
    DrawTexturePro(texTitleFrame, srcRec, destRec, (Vector2){0,0}, 0.0f, WHITE);

    // 2. Tính toán và Vẽ Chữ CĂN GIỮA vào đúng giữa Khung ảnh
    Vector2 textSize = MeasureTextEx(globalFont, text, 24, 1);
    Vector2 textPos = {
        targetX + (srcRec.width - textSize.x) / 2.0f,
        currentY + (srcRec.height - textSize.y) / 2.0f
    };
    
    // Đổi màu chữ ở đây nếu cần (Hiện tại đang để màu Kem Sáng)
    Color textColor = (Color){ 94, 65, 56, 255 }; 
    DrawTextEx(globalFont, text, textPos, 24, 1, textColor);
}

// [NEW] HÀM VẼ SÁCH ANIMATION (BẢN HOÀN THIỆN VẬT LÝ 3D)
void DrawInfoBookAnimation() {
    float sw = (float)SCREEN_WIDTH;  
    float sh = (float)SCREEN_HEIGHT;

    // 1. KÍCH THƯỚC TĨNH CỦA SÁCH (Đích đến cuối cùng)
    float bookWidth = sw * 0.8f;      
    float bookHeight = sh * 0.75f;    
    float startX = (sw - bookWidth) / 2.0f;
    float startY = (sh - bookHeight) / 2.0f + 20.0f; 
    startY += bookDropYOffset;

    float spineX = startX + bookWidth / 2.0f; // Tọa độ X của Gáy Sách
    float halfWidth = bookWidth / 2.0f;       // Chiều rộng 1 nửa bìa sách

    float paddingX = 25.0f; 
    float paddingY = 20.0f; 
    float gap = -25.0f;     
    float pageWidth = (bookWidth - (paddingX * 2) - gap) / 2.0f;
    float pageHeight = bookHeight - (paddingY * 2);

    // 2. TỌA ĐỘ SPRITESHEET CỦA BẠN (GIỮ NGUYÊN GỐC)
    Rectangle srcCover     = { 32, 32, 224, 160 };  
    Rectangle srcPageLeft  = { 272, 32, 104, 147 }; 
    Rectangle srcPageRight = { 392, 32, 104, 147 }; 
    
    // Cấu hình NPatch và Khung tĩnh
    NPatchInfo patchCover = { .source = srcCover, .left = 14, .top = 14, .right = 14, .bottom = 14, .layout = NPATCH_NINE_PATCH };
    Rectangle destCover = { startX, startY, bookWidth, bookHeight };
    
    Rectangle destRight = { startX + paddingX + pageWidth + gap, startY + paddingY, pageWidth, pageHeight };
    Rectangle destLeft = { startX + paddingX, startY + paddingY, pageWidth, pageHeight };
    NPatchInfo patchRight = { .source = srcPageRight, .left = 0, .top = 16, .right = 8, .bottom = 16, .layout = NPATCH_NINE_PATCH };
    NPatchInfo patchLeft = { .source = srcPageLeft, .left = 8, .top = 16, .right = 0, .bottom = 16, .layout = NPATCH_NINE_PATCH };

    // ==========================================
   // ==========================================
    // PHASE 1: ĐÓNG / MỞ BÌA CỨNG (Hiệu ứng 3D siêu thực giống lật trang)
    // ==========================================
    if (bookState == BOOK_IDLE || bookState == BOOK_DROPPING || bookState == BOOK_PAUSE_BEFORE_OPEN || bookState == BOOK_OPEN_RIGHT || bookState == BOOK_CLOSE_RIGHT || bookState == BOOK_EXITING) {
        float progress = 0.0f;
        if (bookState == BOOK_OPEN_RIGHT) progress = bookProgress;
        if (bookState == BOOK_CLOSE_RIGHT) progress = 1.0f - bookProgress;
        progress = progress * progress * (3.0f - 2.0f * progress);

        // 1. CHỈ VẼ NỬA NỀN BÊN PHẢI tĩnh (Mặt trong bìa phải + Trang phải)
        // Vì bìa trái chưa lật tới nên ta phải giấu nửa nền trái đi bằng Scissor
        BeginScissorMode((int)spineX, (int)startY, (int)halfWidth, (int)bookHeight);
            DrawTextureNPatch(texUIBook, patchCover, destCover, (Vector2){0,0}, 0.0f, WHITE);
            DrawTextureNPatch(texUIBook, patchRight, destRight, (Vector2){0,0}, 0.0f, WHITE);
        EndScissorMode();

        // 2. Logic Lật Bìa (Từ Phải sang Trái giống hệt lật trang giấy)
        Rectangle movingCover;
        
        if (progress < 0.5f) {
            // NỬA ĐẦU (0.0 -> 0.5): Bìa ngoài nhấc từ bên Phải lên
            float p = progress * 2.0f; // Chạy 0.0 -> 1.0
            
            movingCover.width = halfWidth * (1.0f - p);
            movingCover.height = bookHeight;
            movingCover.y = startY;
            movingCover.x = spineX; // Neo tại gáy sách, mép phải co về gáy
            
            // Cắt 1 nửa bên phải của ảnh bìa đỏ làm bìa ngoài
            Rectangle srcHalfCover = { srcCover.x + srcCover.width/2.0f, srcCover.y, srcCover.width/2.0f, srcCover.height };
            DrawTexturePro(texUIBook, srcHalfCover, movingCover, (Vector2){0,0}, 0.0f, WHITE);

            // Vẽ Aura đè lên bìa đang lật (Chỉ hiện ở nửa đầu)
            float auraScaleCover = movingCover.width / halfWidth;
            float auraSize = 250.0f;
            Rectangle destAura = { spineX + movingCover.width/2.0f, startY + bookHeight/2.0f, auraSize * auraScaleCover, auraSize };
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texAura, (Rectangle){0,0,texAura.width,texAura.height}, destAura, (Vector2){destAura.width/2, destAura.height/2}, GetTime() * 20.0f, Fade(GOLD, 0.8f));
            EndBlendMode();
            
       } else {
            // NỬA SAU (0.5 -> 1.0): Mặt trong bìa hạ xuống bên Trái
            float p = (progress - 0.5f) * 2.0f; // Chạy 0.0 -> 1.0
            
            movingCover.width = halfWidth * p;
            movingCover.height = bookHeight;
            movingCover.y = startY;
            movingCover.x = spineX - movingCover.width; // Neo tại gáy sách, vươn dần ra mép trái
            
            // Cắt 1 nửa bên trái của ảnh bìa đỏ (Đóng vai trò là mặt trong bìa trái)
            Rectangle srcLeftCover = { srcCover.x, srcCover.y, srcCover.width/2.0f, srcCover.height };

            // [TỐI ƯU] Sử dụng hàm GetPageFlipTint để đổ bóng mượt mà thay vì LIGHTGRAY cố định
            DrawTexturePro(texUIBook, srcLeftCover, movingCover, (Vector2){0,0}, 0.0f, GetPageFlipTint(progress));
        }
    }

    // ==========================================
    // PHASE 2: LẬT TRANG GIẤY (Hiệu ứng 3D siêu thực)
    // ==========================================
    else if (bookState == BOOK_HALF_OPEN || bookState == BOOK_OPEN_LEFT || bookState == BOOK_FULLY_OPEN || bookState == BOOK_CLOSE_LEFT || bookState == BOOK_TURN_NEXT || bookState == BOOK_TURN_PREV) {
        
        float progress = 0.0f;

        if (bookState == BOOK_OPEN_LEFT || bookState == BOOK_TURN_NEXT) progress = bookProgress;
        if (bookState == BOOK_CLOSE_LEFT || bookState == BOOK_TURN_PREV) progress = 1.0f - bookProgress;
        if (bookState == BOOK_FULLY_OPEN) progress = 1.0f;
        progress = progress * progress * (3.0f - 2.0f * progress);
        // 1. Vẽ Full Nền Đỏ + Trang Phải tĩnh (Vì bìa cứng đã mở xong 100%)
        DrawTextureNPatch(texUIBook, patchCover, destCover, (Vector2){0,0}, 0.0f, WHITE);
        DrawTextureNPatch(texUIBook, patchRight, destRight, (Vector2){0,0}, 0.0f, WHITE);
        if (bookState == BOOK_FULLY_OPEN || bookState == BOOK_TURN_NEXT || bookState == BOOK_TURN_PREV) {
            DrawTextureNPatch(texUIBook, patchLeft, destLeft, (Vector2){0,0}, 0.0f, WHITE);
        }
        // 2. Logic Lật Trang (Từ Phải sang Trái)
        if (progress > 0.0f) {
            // [MỚI] Nếu đang gập sách lại (Bấm Back/Close), ta vẽ 4 lớp giấy để tạo xấp dày. Nếu lật thường thì vẽ 1 lớp.
            int pagesToDraw = (bookState == BOOK_CLOSE_LEFT) ? 4 : 1;

            // Vẽ từ lớp dưới cùng (lag nhất) lên lớp trên cùng
            for (int i = pagesToDraw - 1; i >= 0; i--) { 
                
                float currentProg = progress;
                // Tạo độ trễ (lag) cho các trang bên dưới để xòe ra như cây quạt
                if (bookState == BOOK_CLOSE_LEFT) {
                    currentProg = progress + (i * 0.06f); // Mỗi trang lùi lại 0.06
                    if (currentProg > 1.0f) currentProg = 1.0f;
                }

                if (currentProg <= 0.0f) continue; // Nếu trễ quá chưa tới góc lật thì bỏ qua

                Rectangle movingPage;
                Color pageTint = GetPageFlipTint(currentProg); // Bóng đổ riêng cho từng lá

                if (currentProg < 0.5f) {
                    // NỬA ĐẦU: Nhấc từ bên Phải lên
                    float p = currentProg * 2.0f;
                    movingPage.width = destRight.width * (1.0f - p);
                    movingPage.height = destRight.height;
                    movingPage.y = destRight.y;
                    movingPage.x = spineX + (destRight.x - spineX) * (1.0f - p);
                } else {
                    // NỬA SAU: Hạ xuống bên Trái
                    float p = (currentProg - 0.5f) * 2.0f;
                    movingPage.width = destLeft.width * p;
                    movingPage.height = destLeft.height;
                    movingPage.y = destLeft.y;
                    movingPage.x = spineX - (spineX - destLeft.x) * p;
                }

                // Làm tối nhẹ các trang nằm bên dưới để nổi bật trang trên cùng (Chiều sâu 3D)
                if (i > 0) {
                    pageTint.r = (unsigned char)(pageTint.r * 0.8f);
                    pageTint.g = (unsigned char)(pageTint.g * 0.8f);
                    pageTint.b = (unsigned char)(pageTint.b * 0.8f);
                }

                // Vẽ từng trang đè lên nhau
                DrawTexturePro(texUIBook, srcPageLeft, movingPage, (Vector2){0,0}, 0.0f, pageTint);
            }
        }
    }
}

// --- DRAW menu

void Menu_Draw() {
    if (currentMenu == MENU_NONE) return;

    float sw = (float)SCREEN_WIDTH;  
    float sh = (float)SCREEN_HEIGHT; 

    // ==========================================
    // PHẦN 1: VẼ NỀN VÀ NHÂN VẬT (NẰM DƯỚI CÙNG)
    // ==========================================
    if (currentMenu == MENU_TITLE) {
        DrawTexturePro(texTitleBG, (Rectangle){0,0,texTitleBG.width,texTitleBG.height}, (Rectangle){0,0,sw,sh}, (Vector2){0,0}, 0.0f, WHITE);
        if (!Game_HasSaveFile()) DrawText("NO SAVE DATA", 340, 200, 20, Fade(GRAY, 0.5f));
    } 

    // --- GỘP CHUNG VẼ NỀN SÁCH CHO CẢ PAUSE, SETTINGS VÀ INFO ---
    else if (currentMenu == MENU_PAUSE || currentMenu == MENU_SETTINGS || currentMenu == MENU_INFO || currentMenu == MENU_INVENTORY) {
        
        // Nếu mở từ Title thì vẽ mờ Title phía sau
        if (previousMenu == MENU_TITLE) {
            DrawTexturePro(texTitleBG, (Rectangle){0,0,texTitleBG.width,texTitleBG.height}, (Rectangle){0,0,sw,sh}, (Vector2){0,0}, 0.0f, WHITE);
        }
        
        // Phủ mờ màn hình đen
        DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.8f));
        
        // Gọi chung 1 hàm vẽ Sách Animation 3D
        DrawInfoBookAnimation();

        // Các tính toán thông số chung để vẽ text
        float bookWidth = sw * 0.8f;      
        float bookHeight = sh * 0.75f;    
        float startX = (sw - bookWidth) / 2.0f;
        float startY = (sh - bookHeight) / 2.0f + 20.0f; 
// --- [NEW] VẼ TIÊU ĐỀ BẰNG HỆ THỐNG ĐỘNG CÓ ANIMATION ---
        
        // Tọa độ cắt Khung ảnh trên file resources/menu/title_frames.png (Bạn tự chỉnh lại cho khớp thực tế)
        Rectangle srcTitleFrame = { 25, 27, 330, 47 }; // Cắt 1 khung rộng 200px, cao 50px
        
        // 0. Vẽ cho màn hình INVENTORY (Khi mở toang)
        if (bookState == BOOK_FULLY_OPEN && currentMenu == MENU_INVENTORY) {
            float leftPageCenterX = startX + (bookWidth / 4.0f) + 165.0f; 
            float targetX = leftPageCenterX - (srcTitleFrame.width / 2.0f);
            float targetY = startY - 43.0f;
            DrawDynamicMenuTitle("TÚI ĐỒ", srcTitleFrame, targetX, targetY);
            
            // CHỈ vẽ Grid đồ khi sách đã mở toang (Chống lỗi vẽ đè lúc sách đang lật)
            Inventory_Draw(); 
        }
        // 1. Vẽ cho màn hình SETTINGS (Khi mở toang)
        if (bookState == BOOK_FULLY_OPEN && currentMenu == MENU_SETTINGS) {
            float leftPageCenterX = startX + (bookWidth / 4.0f) + 165.0f; 
            float targetX = leftPageCenterX - (srcTitleFrame.width / 2.0f); // Tự động căn giữa khung vào trang
            float targetY = startY - 43.0f;
            
            DrawDynamicMenuTitle("CÀI ĐẶT", srcTitleFrame, targetX, targetY);
        }
        
        // 2. Vẽ cho màn hình PAUSE (Khi mở nửa trang)
        else if (bookState == BOOK_HALF_OPEN && currentMenu == MENU_PAUSE) {
            float rightPageCenterX = startX + (bookWidth * 0.75f) - 150.0f;
            float targetX = rightPageCenterX - (srcTitleFrame.width / 2.0f); 
            float targetY = startY -43.0f;
            
            DrawDynamicMenuTitle("TẠM DỪNG", srcTitleFrame, targetX, targetY);
        }
        // 3. Vẽ cho màn hình INFO (Khi ở trang Index chọn Tab - Sách mở nửa)
        else if (currentMenu == MENU_INFO && !isReadingInfo) {
            float rightPageCenterX = startX + (bookWidth * 0.75f) - 150.0f;
            float targetX = rightPageCenterX - (srcTitleFrame.width / 2.0f); 
            float targetY = startY - 43.0f; // Căn Y giống Menu Pause
            
            DrawDynamicMenuTitle("THÔNG TIN", srcTitleFrame, targetX, targetY);
        }
        
      // 4. Vẽ cho màn hình INFO (Khi đang đọc nội dung - Sách mở toang)
        else if (currentMenu == MENU_INFO && isReadingInfo) {
            float leftPageCenterX = startX + (bookWidth / 4.0f) + 165.0f; 
            float targetX = leftPageCenterX - (srcTitleFrame.width / 2.0f); 
            float targetY = startY -43.0f; 

            // Dùng hàm Getter vừa tạo để lấy tên Tab hiện tại
            const char* titleText = (Info_GetCurrentTab() == TAB_DEVLOG) ? "DEVLOG" : "CREDITS";
            DrawDynamicMenuTitle(titleText, srcTitleFrame, targetX, targetY);
            
            // [ĐÃ SỬA] Đưa Info_Draw() vào chung khối này để nó không bị bỏ qua
            if (bookState == BOOK_FULLY_OPEN) {
                Info_Draw();
            }
        }
    }   

    else if (currentMenu == MENU_CHARACTER_SELECT) {
        DrawTexturePro(texChoiceBG, (Rectangle){0,0,texChoiceBG.width,texChoiceBG.height}, (Rectangle){0,0,sw,sh}, (Vector2){0,0}, 0.0f, WHITE);
        
        DrawTextEx(globalFont, "CHỌN NHÂN VẬT CỦA BẠN", (Vector2){ 50, 30 }, 30, 1, WHITE);
        if (selectedCharIndex == -1) {
            DrawTextEx(globalFont, "Hãy click vào một nhân vật để chọn...", (Vector2){ 50, 70 }, 20, 1, RED);
        } else {
            DrawTextEx(globalFont, "Đã chọn! Bấm VÀO GAME để bắt đầu hành trình.", (Vector2){ 50, 70 }, 20, 1, GREEN);
        }

        float centerXs[4] = { 100.0f, 300.0f, 500.0f, 700.0f };
        float charY = 90.0f; 
       
        // --- CHÈN MỚI: Vẽ Aura cho nhân vật đang được chọn ---
        if (selectedCharIndex != -1) {
            Vector2 charCenter = { centerXs[selectedCharIndex], charY + previewChars[selectedCharIndex].drawHeight / 2.0f };
            
            // Cập nhật hiệu ứng xoay
            auraRotation += 60.0f * GetFrameTime();
            auraScale = 1.0f + (sinf((float)GetTime() * 5.0f) * 0.05f); 

            BeginBlendMode(BLEND_ADDITIVE);
                float drawSize = 250.0f * auraScale; 
                Rectangle destAura = { charCenter.x, charCenter.y, drawSize, drawSize };
                Vector2 originAura = { drawSize / 2.0f, drawSize / 2.0f };
                
                // Lớp 1: Màu Vàng Kim (GOLD)
                DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, auraRotation, Fade(GOLD, 0.7f));
                // Lớp 2: Màu Trắng (WHITE) lấp lánh xoay ngược
                DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, -auraRotation * 1.2f, Fade(WHITE, 0.3f));
                // Vẽ thêm lớp phụ xoay ngược
                DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, -auraRotation * 1.2f, Fade(WHITE, 0.3f));
            EndBlendMode();
        }
        for (int i = 0; i < 4; i++) {
            Rectangle dest = { centerXs[i] - previewChars[i].drawWidth / 2.0f, charY, previewChars[i].drawWidth, previewChars[i].drawHeight };
            Color tint = WHITE;
            if (selectedCharIndex != -1 && selectedCharIndex != i) tint = DARKGRAY; 
            DrawTexturePro(*previewChars[i].currentTexture, previewChars[i].frameRec, dest, (Vector2){0,0}, 0.0f, tint);
        }
    }
   
    else if (currentMenu == MENU_CHARACTER_PROFILE && selectedCharIndex != -1) {
        // Vẽ nền quyển sách
        DrawTexturePro(texProfileBGs[selectedCharIndex], (Rectangle){0, 0, texProfileBGs[selectedCharIndex].width, texProfileBGs[selectedCharIndex].height}, (Rectangle){0, 0, sw, sh}, (Vector2){0,0}, 0.0f, WHITE);

        Vector2 charPos = { 200.0f, 250.0f }; 
        
        // Cập nhật Aura Rotation & Scale
        if (!isGameStarting) {
            auraRotation += 60.0f * GetFrameTime(); 
            auraScale = 1.0f + (sinf((float)GetTime() * 5.0f) * 0.05f); 
        } else {
            auraRotation += 300.0f * GetFrameTime(); 
            auraScale += 10.0f * GetFrameTime();      // Phóng to cực nhanh khi bấm Start
        }
        
        // Vẽ Aura (Phải có cái này mới mượt)
        BeginBlendMode(BLEND_ADDITIVE); 
            float drawSize = 350.0f * auraScale; 
            Rectangle destAura = { charPos.x, charPos.y, drawSize, drawSize };
            Vector2 originAura = { drawSize / 2.0f, drawSize / 2.0f }; 
            // Vẽ 2 lớp Aura cho sáng rực
           // Lớp 1: Màu Vàng Kim rực rỡ
             DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, auraRotation, Fade(GOLD, 0.8f));
            // Lớp 2: Lớp trắng xoay ngược tạo hiệu ứng phép thuật
            DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, -auraRotation * 1.5f, Fade(WHITE, 0.4f));
            DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, originAura, -auraRotation * 1.5f, Fade(WHITE, 0.4f));
        EndBlendMode();

        // Vẽ Nhân vật
        Rectangle destChar = { charPos.x - previewChars[selectedCharIndex].drawWidth / 2.0f, charPos.y - previewChars[selectedCharIndex].drawHeight / 2.0f, previewChars[selectedCharIndex].drawWidth, previewChars[selectedCharIndex].drawHeight };
        DrawTexturePro(*previewChars[selectedCharIndex].currentTexture, previewChars[selectedCharIndex].frameRec, destChar, (Vector2){0,0}, 0.0f, WHITE);

        // Tọa độ này căn theo ảnh book_2.png (sw là độ rộng màn hình, sh là chiều cao)
        float textStartX = sw * 0.54f; // Bắt đầu ở trang bên phải
        float textStartY = sh * 0.22f; // Độ cao từ trên xuống

        // Vẽ chuỗi hiển thị (displayedText) bằng font Tiếng Việt
        DrawTextEx(globalFont, displayedText, (Vector2){ textStartX, textStartY }, 22, 2, DARKGRAY);

        // ĐÂY LÀ CHỖ CHUYỂN MAP MƯỢT MÀ:
        if (isGameStarting && auraScale > 20.0f) {
            Gameplay_SetPlayerClass(selectedCharIndex);
            Transition_StartToMap(MAP_TOA_ALPHA, (Vector2){400, 300}); 
            Audio_PlayMusic(MUSIC_TOA_ALPHA);
            Menu_SwitchTo(MENU_NONE);
            isGameStarting = false;
            auraScale = 1.0f; // Reset cho lần sau
        }
    }

    // ==========================================
    // PHẦN 2: VẼ NÚT BẤM (ĐÈ LÊN TRÊN CÙNG)
    // ==========================================
    if (currentMenu == MENU_TITLE || currentMenu == MENU_PAUSE || currentMenu == MENU_INVENTORY || currentMenu == MENU_SETTINGS || currentMenu == MENU_CHARACTER_SELECT || currentMenu == MENU_CHARACTER_PROFILE || currentMenu == MENU_INFO) {
        
        // --- [NEW] CHẶN VẼ NÚT NẾU SÁCH CHƯA SẴN SÀNG ---
        if (currentMenu == MENU_INFO) {
            // Nút Tab Devlog/Credit chỉ được vẽ khi sách ở trạng thái NỬA MỞ hoặc MỞ TOANG
            if (bookState != BOOK_HALF_OPEN && bookState != BOOK_FULLY_OPEN) return; 
            
            
        }

        Vector2 mouse = GetVirtualMousePos();
        for (int i = 0; i < currentBtnCount; i++) {
            MenuButton *b = &currentButtons[i];
// Chặn vẽ nút nếu Sách (của PAUSE/SETTINGS/INFO) đang bay hoặc đang gập/mở
            if ((currentMenu == MENU_INFO || currentMenu == MENU_SETTINGS || currentMenu == MENU_PAUSE || currentMenu == MENU_INVENTORY) && 
                bookState != BOOK_HALF_OPEN && bookState != BOOK_FULLY_OPEN) {
                continue; 
            }
            if (currentMenu == MENU_INFO) {
                bool isTabButton = (b->actionID == ACT_TAB_DEVLOG || b->actionID == ACT_TAB_CREDIT);
                bool isReadButton = (b->actionID == ACT_INFO_NEXT_PAGE || b->actionID == ACT_INFO_PREV_PAGE || b->actionID == ACT_INFO_BACK_INDEX);

                if (!isReadingInfo && isReadButton) continue; // Ở trang Index -> Ẩn nút đọc chữ
                if (isReadingInfo && (isTabButton || b->actionID == ACT_CLOSE_INFO)) continue;   // [ĐÃ SỬA] Đang đọc chữ -> Ẩn Tab và Nút Đóng

                // Bật khung cắt (Scissor) để tạo hiệu ứng viết từ phải sang trái
                if (!isReadingInfo && bookState == BOOK_HALF_OPEN) {
                    float currentWidth = b->rect.width * buttonRevealProgress;
                    float startX = b->rect.x + b->rect.width - currentWidth;
                    BeginScissorMode((int)startX, (int)b->rect.y - 10, (int)currentWidth, (int)b->rect.height + 20);
                }
            }

            if (!isDebugOverlay) {
                if (b->actionID == ACT_CONTINUE && !Game_HasSaveFile()) continue;
                if (b->actionID == ACT_INV_PREV_PAGE && Inventory_GetCurrentPage() <= 0) continue;
                if (b->actionID == ACT_INV_NEXT_PAGE && Inventory_GetCurrentPage() >= Inventory_GetMaxPages() - 1) continue;
                if ((b->actionID == ACT_INV_USE || b->actionID == ACT_INV_UNSELECT) && !Inventory_HasSelectedSlot()) continue;
            }

            bool isHover = CheckCollisionPointRec(mouse, b->rect);
            
            if (b->isInvisible) {
                if (!isDebugOverlay && b->type == CTRL_SLIDER) {
                    float fillWidth = b->rect.width * b->value;
                    DrawRectangle((int)b->rect.x, (int)b->rect.y, (int)fillWidth, (int)b->rect.height, Fade(GREEN, 0.3f));
                }
                 if (isDebugOverlay && b->type == CTRL_SLIDER) {
                     float fillWidth = b->rect.width * b->value;
                     DrawRectangle((int)b->rect.x, (int)b->rect.y, (int)fillWidth, (int)b->rect.height, Fade(GREEN, 0.3f));
                 }
                if (isDebugOverlay) {
                    DrawRectangleRec(b->rect, Fade(RED, 0.3f)); 
                    DrawRectangleLinesEx(b->rect, 1, RED);
                    DrawText(b->label, b->rect.x, b->rect.y - 15, 10, RED);
                }
            } 
            else {
                Color baseColor = isHover ? DARKBLUE : BLACK;
                
                if (b->type == CTRL_CUSTOM_BTN) {
                    DrawCustomUIButton(b, isHover);
                }

              // --- [NEW] NHÁNH VẼ NÚT ICON ---
                else if (b->type == CTRL_ICON_BTN) {
                    Texture2D iconToDraw = texIconPlay; // Mặc định là nút Play

                    // Phân loại Icon dựa trên ID hành động của nút
                    if (b->actionID == ACT_PROFILE_BACK || b->actionID == ACT_INV_PREV_PAGE || b->actionID == ACT_INV_CLOSE) {
                        iconToDraw = texIconBack;
                    } 
                    else if (b->actionID == ACT_CONFIRM_CHAR || b->actionID == ACT_PROFILE_START_GAME || b->actionID == ACT_INV_NEXT_PAGE) {
                        iconToDraw = texIconPlay;
                    }
                    else if (b->actionID == ACT_OPEN_SETTINGS) {
                        iconToDraw = texIconSettings;
                    }
                    else if (b->actionID == ACT_OPEN_INVENTORY) {
                        iconToDraw = texIconInventory; // [THÊM MỚI] Gắn icon túi đồ
                    }
                    // [THÊM MỚI] Gắn icon cho nút Mở Info
                    else if (b->actionID == ACT_OPEN_INFO) {
                        iconToDraw = texIconInfo; 
                    }
                    else if (b->actionID == ACT_RESUME || b->actionID == ACT_START_GAME || b->actionID == ACT_CONTINUE) {
                        iconToDraw = texIconResume;
                    }
                    else if (b->actionID == ACT_SAVE_GAME) {
                        iconToDraw = texIconSave;
                    }
                    else if (b->actionID == ACT_QUIT_TO_TITLE || b->actionID == ACT_EXIT_GAME) {
                        iconToDraw = texIconHome;
                    }

                    // Gọi hàm Base vẽ 4 frame
                    DrawAnimatedIconBase(iconToDraw, b->rect, isHover);
                }

                else if (b->type == CTRL_BUTTON) {
                    DrawRectangleRec(b->rect, Fade(baseColor, 0.8f));
                    DrawRectangleLinesEx(b->rect, 2.0f, WHITE);
                    Vector2 textSize = MeasureTextEx(globalFont, b->label, 20, 1);
                    Vector2 textPos = { b->rect.x + (b->rect.width - textSize.x) / 2, b->rect.y + (b->rect.height - textSize.y) / 2 };
                    DrawTextEx(globalFont, b->label, textPos, 20, 1, WHITE);
                }
              else if (b->type == CTRL_SLIDER) {
                    // 1. TỌA ĐỘ BẠN ĐÃ ĐO (GIỮ NGUYÊN)
                    Rectangle srcIcon  = { 69, 788, 22, 24 }; 
                    Rectangle srcTrack = { 33, 454, 62, 4 }; 
                    Rectangle srcFill  = { 33,471, 30, 2 }; 
                    Rectangle srcKnob  = { 101, 484, 7, 8 }; 
                    
                    // ==========================================
                    // BƯỚC 1: VẼ RÃNH NỀN TỐI (ĂN HẾT 100% HITBOX)
                    // ==========================================
                    Rectangle destTrack = b->rect; // Khung rãnh bằng đúng Hitbox bạn vẽ bằng chuột
                    DrawTexturePro(texUIBook, srcTrack, destTrack, (Vector2){0,0}, 0.0f, WHITE);

                    // ==========================================
                    // BƯỚC 2: TÍNH TOÁN ĐỘ THỤT (PADDING) CHO THANH VÀNG
                    // ==========================================
                    // Tính xem Rãnh đã bị kéo giãn bao nhiêu lần so với ảnh gốc
                    float scaleX = destTrack.width / srcTrack.width;
                    float scaleY = destTrack.height / srcTrack.height;

                    // Khai báo độ dày viền của rãnh trên ảnh gốc (Bạn có thể chỉnh số này nếu thấy thanh vàng vẫn bị tràn)
                    float padSrcX = 2.0f; // Thụt trái/phải 2 pixel
                    float padSrcY = 1.0f; // Thụt trên/dưới 1 pixel

                    // Quy đổi độ thụt ra kích thước thực tế trên màn hình
                    float padDestX = padSrcX * scaleX; 
                    float padDestY = padSrcY * scaleY;

                    // ==========================================
                    // BƯỚC 3: VẼ THANH VÀNG (FILL) LỌT VÀO GIỮA RÃNH
                    // ==========================================
                    float maxFillWidth = destTrack.width - (padDestX * 2.0f); // Chiều dài tối đa sau khi trừ 2 đầu
                    float currentFillWidth = maxFillWidth * b->value;         // Chiều dài hiện tại theo %
                    float fillHeight = destTrack.height - (padDestY * 2.0f);  // Chiều cao lọt lòng

                    Rectangle destFill = { 
                        destTrack.x + padDestX, // Đẩy sang phải một khoảng bằng viền
                        destTrack.y + padDestY, // Đẩy xuống dưới một khoảng bằng viền
                        currentFillWidth, 
                        fillHeight
                    };
                    DrawTexturePro(texUIBook, srcFill, destFill, (Vector2){0,0}, 0.0f, WHITE);

                    // ==========================================
                    // BƯỚC 4: VẼ CỤC GẠT (KNOB) CHUẨN TỶ LỆ GỐC
                    // ==========================================
                    // Muốn cục gạt cao gấp mấy lần rãnh thì chỉnh số 2.5f này
                    float knobHeight = destTrack.height * 0.9f; 
                    // Tính chiều rộng dựa theo chiều cao để GIỮ NGUYÊN TỶ LỆ GỐC (Không bị méo)
                    float knobWidth = srcKnob.width * (knobHeight / srcKnob.height); 
                    
                    Rectangle destKnob = {
                        destFill.x + currentFillWidth - (knobWidth / 2.0f), // Tâm cục gạt nằm ngay mép thanh vàng
                        destTrack.y + (destTrack.height - knobHeight) / 2.0f, // Căn giữa theo chiều dọc rãnh
                        knobWidth, 
                        knobHeight
                    };
                    DrawTexturePro(texUIBook, srcKnob, destKnob, (Vector2){0,0}, 0.0f, WHITE);

                    // ==========================================
                    // BƯỚC 5: VẼ ICON VÀ CHỮ
                    // ==========================================
                    // ==========================================
                    // BƯỚC 5: VẼ ICON VÀ CHỮ
                    Color brownColor = (Color){ 94, 65, 56, 255 };
                    // ==========================================
                    float iconScale = 1.2f; 
                    Rectangle destIcon = { 
                        b->rect.x - (srcIcon.width * iconScale) - 15, 
                        b->rect.y + (b->rect.height - (srcIcon.height * iconScale))/2, 
                        srcIcon.width * iconScale, 
                        srcIcon.height * iconScale 
                    };
                    DrawTexturePro(texUIBook, srcIcon, destIcon, (Vector2){0,0}, 0.0f, WHITE);

                    // Vẽ chữ nhãn bên trái trên
                    DrawTextEx(globalFont, b->label, (Vector2){ b->rect.x, b->rect.y - 25 }, 15, 1, brownColor);

                    // --- [SỬA ĐỔI] CHUYỂN PHẦN TRĂM LÊN TRÊN BÊN PHẢI ---
                    // 1. Tạo chuỗi phần trăm
                    const char* pctText = TextFormat("%d%%", (int)(b->value * 100.0f + 0.5f));
                    // 2. Đo kích thước chuỗi để căn lề phải
                    Vector2 pctSize = MeasureTextEx(globalFont, pctText, 20, 1);
                    // 3. Tọa độ X: Góc phải của thanh (b->rect.x + b->rect.width) trừ đi chiều dài của chữ
                    // 4. Tọa độ Y: Ngang hàng với chữ nhãn (b->rect.y - 25)
                    DrawTextEx(globalFont, pctText, (Vector2){ b->rect.x + b->rect.width - pctSize.x, b->rect.y - 25 }, 20, 1, brownColor);
                }
              else if (b->type == CTRL_TOGGLE) {
                    // 1. Tọa độ cắt ảnh trên sheet
                    Rectangle srcToggleOff = { 128, 533, 16,6 }; // Hình công tắc lúc TẮT
                    Rectangle srcToggleOn  = { 144, 533, 16, 6 }; // Hình công tắc lúc BẬT
                    
                    // 2. Chọn ảnh tự động dựa vào giá trị đang có
                    Rectangle activeSrc = (b->value > 0.5f) ? srcToggleOn : srcToggleOff;
                    
                    // 3. Vẽ đè lên hitbox của Debug Tool
                    DrawTexturePro(texUIBook, activeSrc, b->rect, (Vector2){0,0}, 0.0f, WHITE);
                    
                    
                }

                if (isDebugOverlay) {
                    DrawRectangleLinesEx(b->rect, 2.0f, RED); 
                    DrawText(GetActionName(b->actionID), b->rect.x, b->rect.y - 15, 10, RED);
                }
            }
            if (currentMenu == MENU_INFO && !isReadingInfo && bookState == BOOK_HALF_OPEN) {
                EndScissorMode();
            }
        }
    }
}

void Menu_SwitchTo(MenuType type) {
    // [SỬA LỖI] Lưu lại đúng Menu trước đó để tránh lỗi vẽ nhầm nền Title đè lên màn hình Game
    if (currentMenu != type) {
        previousMenu = currentMenu;
    }
    currentMenu = type;
    
    // Chỉ Load Layout nếu là Menu Động (Title/Pause/Inv/[MOI]Settings)
    // Character Select không cần load file
    Menu_LoadLayout(type); 
    
    // [MOI] Cập nhật lại trạng thái các nút Settings dựa trên dữ liệu hiện tại
    if (type == MENU_SETTINGS) {
        for (int i=0; i<currentBtnCount; i++) {
            MenuButton *b = &currentButtons[i];
            if (b->actionID == ACT_SET_MASTER_VOL) b->value = Audio_GetMasterVolume();
            if (b->actionID == ACT_SET_MUSIC_VOL) b->value = Audio_GetMusicVolume();
            if (b->actionID == ACT_SET_SFX_VOL) b->value = Audio_GetSFXVolume();
           if (b->actionID == ACT_TOGGLE_MUTE) b->value = Audio_IsMuted() ? 1.0f : 0.0f;
            if (b->actionID == ACT_TOGGLE_MUSIC_MUTE) b->value = Audio_IsMusicMuted() ? 1.0f : 0.0f; // [NEW]
            if (b->actionID == ACT_TOGGLE_SFX_MUTE) b->value = Audio_IsSFXMuted() ? 1.0f : 0.0f;     // [NEW]
            if (b->actionID == ACT_TOGGLE_FULLSCREEN) b->value = IsWindowFullscreen() ? 1.0f : 0.0f;
        }
    }
    
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (type == MENU_CHARACTER_PROFILE) {
        framesCounterText = 0;
        letterCount = 0;
        displayedText[0] = '\0';
        isTextFinished = false;
    }
}

void Menu_Init() {
    texTitleBG = LoadTexture("resources/menu/titlescreen.png");
    texChoiceBG = LoadTexture("resources/menu/choice_main.png");
    texAura = LoadTexture("resources/menu/aura.png");

    texUIBook = LoadTexture("resources/menu/ui.png");
    SetTextureFilter(texUIBook, TEXTURE_FILTER_POINT); // Bắt buộc dùng POINT để pixel art không bị mờ

    // Load ảnh Icon nút 
    texIconPlay = LoadTexture("resources/menu/nextanibut.png");
    SetTextureFilter(texIconPlay, TEXTURE_FILTER_POINT);

    //nút back
    texIconBack = LoadTexture("resources/menu/backanibut.png");
    SetTextureFilter(texIconBack, TEXTURE_FILTER_POINT);
    // [NEW] Load ảnh icon mới
    texIconSettings = LoadTexture("resources/menu/setting_icon.png");
    SetTextureFilter(texIconSettings, TEXTURE_FILTER_POINT);
    
    texIconResume = LoadTexture("resources/menu/resume_icon.png");
    SetTextureFilter(texIconResume, TEXTURE_FILTER_POINT);
    
    texIconSave = LoadTexture("resources/menu/save_icon.png");
    SetTextureFilter(texIconSave, TEXTURE_FILTER_POINT);
    
    texIconHome = LoadTexture("resources/menu/home_icon.png");
    SetTextureFilter(texIconHome, TEXTURE_FILTER_POINT);

    texIconInventory = LoadTexture("resources/menu/inv_icon.png"); // [THÊM MỚI]
    SetTextureFilter(texIconInventory, TEXTURE_FILTER_POINT);
    texIconInfo = LoadTexture("resources/menu/info_icon.png"); // [THÊM MỚI]
    SetTextureFilter(texIconInfo, TEXTURE_FILTER_POINT);
    // [NEW] Load ảnh chứa các Khung Tiêu Đề
    texTitleFrame = LoadTexture("resources/menu/title_frames.png");
    SetTextureFilter(texTitleFrame, TEXTURE_FILTER_POINT);

    texProfileBGs[0] = LoadTexture("resources/menu/book_1.png");   // Mây xanh
    texProfileBGs[1] = LoadTexture("resources/menu/book_2.png");  // Hoàng hôn tím
    texProfileBGs[2] = LoadTexture("resources/menu/book_3.png"); // Trời đêm
    texProfileBGs[3] = LoadTexture("resources/menu/book_4.png");  // Mây hồng
    SetTextureFilter(texAura, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texTitleBG, TEXTURE_FILTER_POINT);
    SetExitKey(KEY_NULL);
    
    // Khởi tạo 4 diễn viên cho màn hình chọn tướng dựa theo enum PlayerClass
    InitPlayer(&previewChars[0], CLASS_STUDENT);
    InitPlayer(&previewChars[1], CLASS_WARRIOR);
    InitPlayer(&previewChars[2], CLASS_MAGE);
    InitPlayer(&previewChars[3], CLASS_ARCHER);
    
    for(int i = 0; i < 4; i++) {
        previewChars[i].drawWidth = 300.0f;  // Cho nhân vật to hơn xíu để nhìn cho rõ
        previewChars[i].drawHeight = 300.0f;
    }

    Menu_SwitchTo(MENU_TITLE);
}

bool Menu_ShouldCloseGame() { return shouldCloseGame; }
void Menu_RequestClose() { shouldCloseGame = true; }
bool Menu_IsActive() { return currentMenu != MENU_NONE; }
void Menu_Shutdown() {
    UnloadTexture(texTitleBG);
    UnloadTexture(texChoiceBG);
    UnloadTexture(texAura);
    UnloadTexture(texUIBook);
    UnloadTexture(texIconPlay);
    UnloadTexture(texIconBack);
    UnloadTexture(texIconSettings);
    UnloadTexture(texIconResume);
    UnloadTexture(texIconSave);
    UnloadTexture(texIconHome);
    UnloadTexture(texIconInventory); // [THÊM MỚI]
    UnloadTexture(texIconInventory); // [THÊM MỚI]
    UnloadTexture(texTitleFrame);

    for (int i=0; i<4; i++) UnloadTexture(texProfileBGs[i]);
}
