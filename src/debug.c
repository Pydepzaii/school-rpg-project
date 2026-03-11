// FILE: src/debug.c
#include "raylib.h"
#include "debug.h"
#include "map.h"
#include "settings.h"
#include "ui_style.h"
#include "camera.h"
#include "inventory.h"
#include <stdio.h>
#include "player.h"
#include "interact.h"
#include "menu_system.h"
#include "dialog_system.h"
#include "raygui.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// --- [NEW] DANH SÁCH HÀNH ĐỘNG HỢP LỆ CHO TỪNG MENU ---

// Giúp giới hạn chức năng để không chọn nhầm
//bảng chọn cho titile
static int actionsForTitle[] = { 
    ACT_START_GAME, ACT_CONTINUE, ACT_OPEN_INFO,ACT_OPEN_SETTINGS, ACT_EXIT_GAME 
};// Bảng chọn cho Menu Info
static int actionsForInfo[] = {
    ACT_TAB_DEVLOG, ACT_TAB_CREDIT, ACT_CLOSE_INFO,ACT_INFO_NEXT_PAGE, ACT_INFO_PREV_PAGE, ACT_INFO_BACK_INDEX 
};
//bảng chọn cho pouse
static int actionsForPause[] = { 
    ACT_RESUME, ACT_SAVE_GAME, ACT_OPEN_INVENTORY, ACT_OPEN_SETTINGS, ACT_OPEN_INFO, ACT_QUIT_TO_TITLE 
};
//bảng chọn cho inventory
static int actionsForInventory[] = { 
    ACT_INV_PREV_PAGE, ACT_INV_NEXT_PAGE, ACT_INV_USE, ACT_INV_UNSELECT, ACT_INV_CLOSE 
};

// [MOI] Bảng chọn cho Settings
static int actionsForSettings[] = { 
    ACT_SET_MASTER_VOL, ACT_SET_MUSIC_VOL, ACT_SET_SFX_VOL, 
    ACT_TOGGLE_MUTE, ACT_TOGGLE_MUSIC_MUTE, ACT_TOGGLE_SFX_MUTE, // [MODIFIED] Thêm 2 nút Mute
    ACT_TOGGLE_FULLSCREEN, ACT_INV_CLOSE // Dùng INV_CLOSE làm nút Back
};

static int actionsForCharSelect[] = { 
    ACT_SEL_CLASS_1, ACT_SEL_CLASS_2, ACT_SEL_CLASS_3, ACT_SEL_CLASS_4, 
    ACT_CONFIRM_CHAR, ACT_QUIT_TO_TITLE 
};

//Bảng chọn chức năng cho màn hình Profile
static int actionsForProfile[] = {
    ACT_PROFILE_START_GAME, ACT_PROFILE_BACK
};

// Biến quản lý Popup chọn chức năng
static bool showActionPopup = false;
static Vector2 popupPos = {0};
static int* currentValidActions = NULL;
static int currentValidActionCount = 0;

// --- [DIALOG TOOL DATA] ---
static bool showDialogDebug = false;
static Npc* currentDialogNpc = NULL;
static DialogEvent* selectedEvent = NULL;

static int activeTextBoxIndex = -1; 
static float dialogScrollY = 0.0f;
static bool showDialogDeleteConfirm = false;
static int lineToDelete = -1;

// Hàm lấy danh sách hành động dựa trên Menu hiện tại
//nhớ nhé nếu cập nhật thêm nút chức năng ở phần biến static thì phải sửa số lượng phần tử ở đây nhé
void UpdateValidActionsList() {
    if (currentMenu == MENU_TITLE) {
        currentValidActions = actionsForTitle;
        currentValidActionCount = 5;
    } else if (currentMenu == MENU_PAUSE) {
        currentValidActions = actionsForPause;
        currentValidActionCount = 6;
    } else if (currentMenu == MENU_INVENTORY) {
        currentValidActions = actionsForInventory;
        currentValidActionCount = 5;
    } 
    // [MOI] Thêm case cho Settings
    else if (currentMenu == MENU_SETTINGS) {
        currentValidActions = actionsForSettings;
        currentValidActionCount = 8;
    }

    else if (currentMenu == MENU_CHARACTER_SELECT) {
        currentValidActions = actionsForCharSelect;
        currentValidActionCount = 6;
    }

    else if (currentMenu == MENU_CHARACTER_PROFILE) {
        currentValidActions = actionsForProfile;
        currentValidActionCount = 2; // Có 2 nút: Next và Back
    }

    else if (currentMenu == MENU_INFO) {
        currentValidActions = actionsForInfo;
        currentValidActionCount = 6;
    }

    else {
        currentValidActions = NULL;
        currentValidActionCount = 0;
    }
    
}

// --- STATE QUẢN LÝ RIÊNG BIỆT ---
static bool showMapDebug = false;
static bool showMenuDebug = false;
static bool showDebugUI = true;
static bool showDrawFrame = false;

// Biến toàn cục cho Tool Prop
static bool showPropTool = false;
static bool isPropDragging = false;
static Vector2 propStartPos = {0};
static Rectangle tempProps[50];
static int tempPropCount = 0;

//chức năng support xóa thêm tường in map
static bool isDeleteMode = false;
static bool showDeleteConfirm = false;
static Rectangle targetDeleteRect;
static int targetDeleteType = 0; // 0: Wall, 1: Prop

// [MENU TOOL DATA - UPDATED]
// Đã chuyển sang dùng trực tiếp currentButtons của menu_system
static bool isMenuDragging = false;
static Vector2 menuStartPos = {0};
static int selectedBtnIndex = -1; // Index của nút đang chọn để sửa
static bool isDragMove = false;   // Đang di chuyển nút?

//hàm hỗ trợ file map_date.txt để đọc ghi debug realtime
const char* GetMapDataFileName(int mapID) {
    switch(mapID) {
        case MAP_TOA_ALPHA: return "resources/map_data/map_1.txt";
        case MAP_NHA_VO:    return "resources/map_data/map_2.txt";
        case MAP_THU_VIEN:  return "resources/map_data/map_3.txt";
        case MAP_NHA_AN:    return "resources/map_data/map_4.txt"; // Bổ sung Căng tin
        case MAP_BETA:      return "resources/map_data/map_5.txt"; // Bổ sung Beta
        case MAP_LAB:       return "resources/map_data/map_6.txt"; // Bổ sung Lab
        default: return NULL;
    }
}

void SaveMapDataToFile(GameMap *map) {
    const char* fileName = GetMapDataFileName(map->currentMapID);
    if (!fileName) return;
    FILE *f = fopen(fileName, "w");
    if (!f) { printf("ERR: Khong mo duoc file %s\n", fileName); return; }

    for (int i = 0; i < map->wallCount; i++) {
        Rectangle r = map->walls[i];
        fprintf(f, "WALL %.0f %.0f %.0f %.0f\n", r.x, r.y, r.width, r.height);
    }
    for (int i = 0; i < map->propCount; i++) {
        GameProp p = map->props[i];
        fprintf(f, "PROP %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n", 
                p.sourceRec.x, p.sourceRec.y, p.sourceRec.width, p.sourceRec.height,
                p.position.x, p.position.y, p.originY);
    }
    fclose(f);
    printf(">> SAVED: %s\n", fileName);
}

void DeleteWallFromMemory(GameMap *map, int index) {
    if (index < 0 || index >= map->wallCount) return;
    map->walls[index] = map->walls[map->wallCount - 1];
    map->wallCount--;
}

void DeletePropFromMemory(GameMap *map, int index) {
    if (index < 0 || index >= map->propCount) return;
    map->props[index] = map->props[map->propCount - 1];
    map->propCount--;
}

bool IsFloatEqual(float a, float b) { return fabsf(a - b) < 1.0f; }

// --- HÀM UI RIÊNG CHO DEBUG ---
bool DrawButton(const char* text, Rectangle rec) {
    Vector2 mousePoint = GetVirtualMousePos();
    bool isHover = CheckCollisionPointRec(mousePoint, rec);
    bool isClicked = false;

    // Vẽ nền nút
    if (isHover) {
        DrawRectangleRec(rec, Fade(BLUE, 0.8f)); // Sáng hơn khi di chuột
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isClicked = true;
        }
    } else {
        DrawRectangleRec(rec, Fade(DARKBLUE, 0.6f)); // Tối hơn khi bình thường
    }
    
    // Vẽ viền và chữ
    DrawRectangleLinesEx(rec, 1, WHITE);
    Vector2 size = MeasureTextEx(globalFont, text, 10, 1);
    float textWidth = size.x;
    DrawText(text, (int)(rec.x + (rec.width - textWidth)/2), (int)(rec.y + (rec.height - 10)/2), 10, WHITE);

    return isClicked;
}

// [MAP TOOL DATA]
#define MAX_TEMP_WALLS 100
static bool isMapDragging = false;
static Vector2 mapStartPos = {0};
static Rectangle tempMapWalls[MAX_TEMP_WALLS]; 
static int tempMapWallCount = 0;

bool IsMenuDebugActive() { return showMenuDebug; }

void Debug_ForceCloseMenuTool() {
    showMenuDebug = false;
    Menu_SetDebugMode(false); // [NEW] Tắt overlay
}

void DrawDebugInfoBox(const char* title, const char* line1, const char* line2) {
    if (!showDebugUI) return; 

    float scrW = (float)SCREEN_WIDTH; 
    Rectangle box = { scrW - 220, 10, 210, 100 };
    
    DrawRectangleRec(box, Fade(BLACK, 0.7f)); 
    DrawRectangleLinesEx(box, 2, WHITE);      
    
    DrawText(title, box.x + 10, box.y + 10, 20, YELLOW);
    DrawText(line1, box.x + 10, box.y + 40, 10, WHITE); 
    DrawText(line2, box.x + 10, box.y + 60, 10, WHITE);
    DrawText("[V] An/Hien Huong Dan", box.x + 10, box.y + 80, 10, GRAY);
}

// ---------------------------------------------
// TOOL 1: MAP DEBUG (PHÍM 0)
// ---------------------------------------------
void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount) {
    if (Inventory_IsActive() || Menu_IsActive()) return;
    if (IsKeyPressed(KEY_ZERO)) {
        showMapDebug = !showMapDebug;
        if (showMapDebug) {
            showMenuDebug = false; 
            Menu_SetDebugMode(false); // [NEW] Đồng bộ tắt Menu debug
            showPropTool = false;
            printf(">> [DEBUG] MAP TOOL: ON (Camera Sync Active)\n");
        } 
    }
    
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMapDebug) return;

    //// bật tắt chế độ xóa tường realtine
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_K)) {
        isDeleteMode = !isDeleteMode;
        showDeleteConfirm = false;
    }

    // --- [TÍNH NĂNG MỚI] PHÍM X: HIỆN KHUNG VẼ (DRAW RECT) ---
    if (IsKeyPressed(KEY_X)) {
        showDrawFrame = !showDrawFrame; // Đảo trạng thái (Bật -> Tắt, Tắt -> Bật)
    }
    if (showDrawFrame) {
        Rectangle drawRect = {
            player->position.x,
            player->position.y,
            player->drawWidth,   // Lấy từ biến bạn đã cài
            player->drawHeight
        };
        
        DrawRectangleLinesEx(drawRect, 1.0f, GREEN);
        // Vẽ thêm chấm tròn đánh dấu gốc tọa độ
        DrawCircle((int)player->position.x, (int)player->position.y, 2.0f, GREEN);
    }

    // 1. Tạm thời tắt Camera để vẽ UI
    EndMode2D();
    
    // UI INFO BOX & SAVE BUTTON
    if (isDeleteMode) {
        DrawDebugInfoBox("DELETE MODE", "Click vao tuong de XOA", "Bam DEL/K de thoat");
    } else {
        DrawDebugInfoBox("MAP TOOL (0)", "Keo: Blue | Tha: Green", "SAVE WALLS de Luu File");
    }

    // UI: BẢNG XÁC NHẬN XÓA (YES/NO)
    if (showDeleteConfirm && targetDeleteType == 0) {
        Rectangle box = { (float)SCREEN_WIDTH/2 - 150, (float)SCREEN_HEIGHT/2 - 50, 300, 100 };
        DrawRectangleRec(box, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx(box, 2, WHITE);
        DrawText("XOA TUONG NAY?", (int)box.x + 80, (int)box.y + 20, 20, WHITE);
        
        // Nut YES
        if (DrawButton("YES", (Rectangle){box.x + 30, box.y + 60, 80, 30})) {
            // Xoa trong RAM
            for(int i=0; i<map->wallCount; i++) {
                if (IsFloatEqual(map->walls[i].x, targetDeleteRect.x) && 
                    IsFloatEqual(map->walls[i].y, targetDeleteRect.y)) {
                    DeleteWallFromMemory(map, i);
                    break;
                }
            }
            SaveMapDataToFile(map); // Luu File ngay
            showDeleteConfirm = false;
        }
        // Nut NO
        if (DrawButton("NO", (Rectangle){box.x + 190, box.y + 60, 80, 30})) {
            showDeleteConfirm = false;
        }
    }

    // [FIX 1] Biến chặn click xuyên thấu (Khai báo trước UI)
    bool isHoveringSaveBtn = false; 

    // UI: NÚT SAVE WALLS
    if (tempMapWallCount > 0 && !isDeleteMode) {
        Rectangle btnSave = { 10, 150, 200, 40 };
        DrawRectangleRec(btnSave, BLUE);
        DrawRectangleLinesEx(btnSave, 2, WHITE);
        DrawText("SAVE WALLS", (int)btnSave.x + 40, (int)btnSave.y + 10, 20, WHITE);
        
        Vector2 mouseUI = GetVirtualMousePos();
        
        // Kiểm tra chuột có đang chạm nút không
        if (CheckCollisionPointRec(mouseUI, btnSave)) {
            isHoveringSaveBtn = true; // [FIX 1] Đánh dấu đang chạm nút
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // Chuyen tuong nhap vao tuong that
                for (int i = 0; i < tempMapWallCount; i++) {
                    if (map->wallCount < MAX_MAP_WALLS) {
                        map->walls[map->wallCount++] = tempMapWalls[i];
                    }
                }
                tempMapWallCount = 0;   // Reset bộ đếm -> Nút sẽ tự ẩn
                SaveMapDataToFile(map); // Ghi file
            }
        }
    }
    
    // 2. Bật lại Camera để vẽ tường
    BeginMode2D(gameCamera);

    // sử lí xóa khi click (LOGIC CẦN NẰM TRONG CAMERA)
    if (isDeleteMode && !showDeleteConfirm) {
        // Duyệt qua tất cả tường để xem chuột có đang trỏ vào cái nào không
        for (int i = 0; i < map->wallCount; i++) {
            // Lấy tọa độ chuột trong thế giới game (đã tính camera ở trên)
            Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);
            
            if (CheckCollisionPointRec(mouseWorld, map->walls[i])) {
                // Nếu chuột chạm tường -> Vẽ viền đỏ
                DrawRectangleLinesEx(map->walls[i], 3.0f, RED);
                
                // Nếu bấm chuột trái -> Xác nhận muốn xóa
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    targetDeleteRect = map->walls[i];
                    targetDeleteType = 0; // 0 là Wall
                    showDeleteConfirm = true; // Hiện bảng Yes/No
                }
            }
        }
    }

    // Vẽ Tường & Debug Map
    DrawMapDebug(map); 
    DrawRectangleLinesEx(player->frameRec, 1.0f, GREEN); 
    Interact_DrawDebugExits(map);

    // Vẽ Tường NHÁP
    for (int i = 0; i < tempMapWallCount; i++) {
        DrawRectangleLinesEx(tempMapWalls[i], 2.0f, GREEN);
        DrawRectangleRec(tempMapWalls[i], Fade(GREEN, 0.2f)); 
    }

    // --- A. VẼ HITBOX PLAYER (VÀNG) ---
    // Lấy trực tiếp kích thước vẽ từ Player
    float drawW = player->drawWidth; 
    float drawH = player->drawHeight;
    
    Rectangle playerHitbox = { 
        player->position.x + (drawW - player->hitWidth) / 2.0f,  
        player->position.y + drawH - player->hitHeight - 2.0f,    
        player->hitWidth,                        
        player->hitHeight                     
    };
    
    DrawRectangleRec(playerHitbox, Fade(YELLOW, 0.5f));
    DrawRectangleLinesEx(playerHitbox, 1.0f, YELLOW);

    // --- B. VẼ HITBOX NPC (TÍM) ---
    for (int i = 0; i < npcCount; i++) {
       if (npcList[i].mapID == map->currentMapID) {
            // 1. TÍNH HITBOX VẬT LÝ (Hộp Tím)
            // Lấy kích thước 1 frame ảnh
            float npcFrameW = (float)npcList[i].texture.width / npcList[i].frameCount;
            float npcFrameH = (float)npcList[i].texture.height;

            // Tính toán offset để hitbox nằm giữa chân
            float offsetX = (npcFrameW - npcList[i].hitWidth) / 2.0f;
            float offsetY = npcFrameH - npcList[i].hitHeight - npcList[i].paddingBottom;

            // Tạo hình chữ nhật Hitbox
            Rectangle npcHitbox = { 
                npcList[i].position.x + offsetX,            
                npcList[i].position.y + offsetY,
                npcList[i].hitWidth, 
                npcList[i].hitHeight                       
            };

            // Vẽ Hitbox Tím (Vật lý)
            DrawRectangleRec(npcHitbox, Fade(PURPLE, 0.7f));
            DrawRectangleLinesEx(npcHitbox, 1.0f, PURPLE);

            // -------------------------------------------------------------
            // 2. VẼ VÙNG TƯƠNG TÁC (Vòng Tròn Vàng)
            // -------------------------------------------------------------
            
            // [QUAN TRỌNG] Tính tâm dựa trên cái Hộp Tím vừa tính ở trên
            Vector2 hitboxCenter = {
                npcHitbox.x + (npcHitbox.width / 2.0f),  // Tâm X = Cạnh trái + nửa chiều rộng
                npcHitbox.y + (npcHitbox.height / 2.0f)  // Tâm Y = Cạnh trên + nửa chiều cao
            };

            // Vẽ vòng tròn từ Tâm này
            DrawCircleLines((int)hitboxCenter.x, (int)hitboxCenter.y, INTERACT_DISTANCE, PURPLE);
            DrawCircleV(hitboxCenter, INTERACT_DISTANCE, Fade(PURPLE, 0.2f));
            
            // Vẽ chấm đỏ ngay tâm để kiểm chứng (Nó phải nằm giữa hộp tím)
            DrawCircle((int)hitboxCenter.x, (int)hitboxCenter.y, 2.0f, RED);
        }
    }

    // Logic Kéo Thả
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // [THÊM VÀO] In tọa độ ra Console khi bấm Chuột Phải
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        printf(">> [TOA DO]: X = %.0f, Y = %.0f\n", mouseWorldPos.x, mouseWorldPos.y);
    }

    // [FIX 1] Thêm điều kiện !isHoveringSaveBtn để chặn xuyên thấu
    if (!isDeleteMode && !showDeleteConfirm && !isHoveringSaveBtn) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isMapDragging = true;
            mapStartPos = mouseWorldPos; 
        }
        
        if (isMapDragging) {
            Vector2 currentPos = mouseWorldPos; 
            Rectangle rect = {
                (mapStartPos.x < currentPos.x) ? mapStartPos.x : currentPos.x,
                (mapStartPos.y < currentPos.y) ? mapStartPos.y : currentPos.y,
                (float)abs((int)(currentPos.x - mapStartPos.x)),
                (float)abs((int)(currentPos.y - mapStartPos.y))
            };
            
            DrawRectangleLinesEx(rect, 2.0f, BLUE); 

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                isMapDragging = false;
                
                // [FIX 2] Chỉ thêm tường nếu kích thước > 5 (Chống click nhầm tạo rác)
                if (rect.width > 2 && rect.height > 2) {
                    if (tempMapWallCount < MAX_TEMP_WALLS) {
                        tempMapWalls[tempMapWallCount++] = rect;
                    }
                } else {
                    printf(">> [DEBUG] Ignored tiny wall (Click noise)\n");
                }
            }
        }
    }

    if (IsKeyPressed(KEY_C) && tempMapWallCount > 0) {
        tempMapWallCount--;
        printf(">> [DEBUG] Undo Last Temp Wall.\n");
    }
}

// ---------------------------------------------
// TOOL 2: MENU DEBUG (PHÍM =) [NÂNG CẤP POPUP]
// ---------------------------------------------
void Debug_RunMenuTool() {
    // 1. Bật Tắt Tool
    if (IsKeyPressed(KEY_EQUAL)) {
        showMenuDebug = !showMenuDebug;
        Menu_SetDebugMode(showMenuDebug); 
        if (showMenuDebug) {
            showMapDebug = false; 
            showPropTool = false;
            selectedBtnIndex = -1; 
            showActionPopup = false; 
            printf(">> [DEBUG] MENU TOOL: ON\n");
        } 
    }

    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;
    if (!showMenuDebug) return;

    // Lấy tọa độ chuột ẢO (Đã qua xử lý Scale) để tính toán chính xác
    Vector2 mouseVirtualPos = GetVirtualMousePos();

    // UI Hướng dẫn (Góc trên phải)
    // Info Box cao 100px, nằm ở Y=10. Vậy đáy của nó là Y=110.
    if (!showActionPopup) {
        DrawDebugInfoBox("MENU TOOL (=)", "Keo: Tao Nut | Chuot Phai: MENU ACTION", "S: SAVE | DEL: Xoa | I: An/Hien");
    } else {
        // Khi đang mở Popup thì hiện text nhắc nhở
        DrawDebugInfoBox("SELECT ACTION", "Chon chuc nang ben duoi...", "Click ra ngoai de huy");
    }

    // --- LOGIC CHỌN / TẠO / DI CHUYỂN (Chỉ chạy khi KHÔNG mở Popup) ---
    if (!showActionPopup) {
        // 1. KIỂM TRA CLICK VÀO NÚT CŨ
        bool isHoveringAnyBtn = false;
        for (int i = 0; i < currentBtnCount; i++) {
            if (CheckCollisionPointRec(mouseVirtualPos, currentButtons[i].rect)) {
                isHoveringAnyBtn = true;
                break;
            }
        }

        // 2. TẠO NÚT MỚI
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isHoveringAnyBtn) {
            isMenuDragging = true;
            menuStartPos = mouseVirtualPos;
            selectedBtnIndex = -1; 
        }

        if (isMenuDragging) {
            Vector2 currentPos = mouseVirtualPos;
            Rectangle rect = {
                (menuStartPos.x < currentPos.x) ? menuStartPos.x : currentPos.x,
                (menuStartPos.y < currentPos.y) ? menuStartPos.y : currentPos.y,
                (float)abs((int)(currentPos.x - menuStartPos.x)),
                (float)abs((int)(currentPos.y - menuStartPos.y))
            };
            DrawRectangleLinesEx(rect, 2.0f, GREEN);

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                isMenuDragging = false;
                if (rect.width > 20 && rect.height > 20 && currentBtnCount < MAX_MENU_BUTTONS) {
                    MenuButton* newBtn = &currentButtons[currentBtnCount];
                    newBtn->rect = rect;
                    newBtn->actionID = -1; 
                    newBtn->isInvisible = false; 
                    newBtn->type = CTRL_BUTTON; // [MOI] Mặc định là Button
                    newBtn->value = 0.5f;       // [MOI] Mặc định value giữa
                    strcpy(newBtn->label, "NEW_BTN");
                    selectedBtnIndex = currentBtnCount++;
                }
            }
        }

        // 3. DI CHUYỂN NÚT CŨ
        if (!isMenuDragging && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHoveringAnyBtn) {
            for (int i = currentBtnCount - 1; i >= 0; i--) { 
                if (CheckCollisionPointRec(mouseVirtualPos, currentButtons[i].rect)) {
                    selectedBtnIndex = i;
                    isDragMove = true;
                    menuStartPos = mouseVirtualPos; 
                    break;
                }
            }
        }

        if (isDragMove && selectedBtnIndex != -1) {
            MenuButton* btn = &currentButtons[selectedBtnIndex];
            Vector2 delta = { mouseVirtualPos.x - menuStartPos.x, mouseVirtualPos.y - menuStartPos.y };
            btn->rect.x += delta.x;
            btn->rect.y += delta.y;
            menuStartPos = mouseVirtualPos; 
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) isDragMove = false;
        }
    }

    // --- HIỂN THỊ NÚT ĐANG CHỌN ---
    if (selectedBtnIndex != -1) {
        MenuButton* btn = &currentButtons[selectedBtnIndex];
        DrawRectangleLinesEx(btn->rect, 3.0f, YELLOW);
        
        // Vẽ thông tin nhỏ cạnh nút
        char info[100];
        sprintf(info, "ID: %d\n%s", selectedBtnIndex, GetActionName(btn->actionID));
        DrawText(info, (int)btn->rect.x, (int)btn->rect.y - 30, 10, YELLOW);

        if (!showActionPopup) {
            if (IsKeyPressed(KEY_DELETE)) {
                for (int i = selectedBtnIndex; i < currentBtnCount - 1; i++) currentButtons[i] = currentButtons[i+1];
                currentBtnCount--;
                selectedBtnIndex = -1;
            }
            if (IsKeyPressed(KEY_I)) btn->isInvisible = !btn->isInvisible;
            
            // [NEW] MỞ POPUP (Bấm Chuột Phải hoặc Space)
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_SPACE)) {
                showActionPopup = true;
                UpdateValidActionsList(); // Load danh sách phù hợp
            }
        }
    }

    // --- [NEW] VẼ POPUP MENU (CỐ ĐỊNH GÓC TRÊN PHẢI) ---
    if (showActionPopup && selectedBtnIndex != -1) {
        // Cấu hình vị trí cố định (Dưới Info Box)
        float popupWidth = 210; 
        float popupX = (float)SCREEN_WIDTH - 220; // Căn thẳng hàng với Info Box
        float popupY = 120; // InfoBox Y=10 + Height=100 + Padding=10 = 120
        
        // [MOI] Vẽ nút đổi TYPE (Button/Slider/Toggle) ở trên cùng
        Rectangle typeChangeRect = { popupX, popupY, popupWidth, 30 };
        DrawRectangleRec(typeChangeRect, DARKPURPLE);
        DrawRectangleLinesEx(typeChangeRect, 1, WHITE);
        
        const char* typeName = "TYPE: BUTTON";
        if (currentButtons[selectedBtnIndex].type == CTRL_SLIDER) typeName = "TYPE: SLIDER";
        if (currentButtons[selectedBtnIndex].type == CTRL_TOGGLE) typeName = "TYPE: TOGGLE";
        if (currentButtons[selectedBtnIndex].type == CTRL_CUSTOM_BTN) typeName = "TYPE: CUSTOM";
        if (currentButtons[selectedBtnIndex].type == CTRL_ICON_BTN) typeName = "TYPE: ICON_ANIM";
        DrawText(typeName, (int)(typeChangeRect.x + 10), (int)(typeChangeRect.y + 8), 10, WHITE);

        if (CheckCollisionPointRec(mouseVirtualPos, typeChangeRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
             // Đổi vòng tròn: BUTTON -> SLIDER -> TOGGLE -> BUTTON
             currentButtons[selectedBtnIndex].type++;
             if (currentButtons[selectedBtnIndex].type >  CTRL_ICON_BTN ) 
                 currentButtons[selectedBtnIndex].type = CTRL_BUTTON;
        }

        // Vẽ danh sách Actions bên dưới nút Type
        float listStartY = popupY + 35; // Cách nút Type một chút
        float itemHeight = 30;
        float popupHeight = currentValidActionCount * itemHeight + 10;
        
        Rectangle popupRect = { popupX, listStartY, popupWidth, popupHeight };
        
        // Vẽ nền Popup danh sách
        DrawRectangleRec(popupRect, Fade(BLACK, 0.95f)); 
        DrawRectangleLinesEx(popupRect, 2, YELLOW);     

        // Vẽ từng dòng Action
        for (int i = 0; i < currentValidActionCount; i++) {
            int actionID = currentValidActions[i];
            Rectangle itemRect = { popupX, listStartY + 5 + i * itemHeight, popupWidth, itemHeight };
            
            // Check va chạm bằng chuột ẢO (Virtual Mouse) -> Chuẩn xác 100%
            bool isItemHover = CheckCollisionPointRec(mouseVirtualPos, itemRect);
            
            if (isItemHover) {
                DrawRectangleRec(itemRect, DARKBLUE); // Highlight
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // --> CHỌN ACTION <--
                    currentButtons[selectedBtnIndex].actionID = actionID;
                    strcpy(currentButtons[selectedBtnIndex].label, GetActionName(actionID));
                    
                    showActionPopup = false; 
                    printf(">> [DEBUG] Assigned Action: %s\n", GetActionName(actionID));
                }
            }
            
            // Vẽ tên Action ra giữa nút
            int textW = MeasureText(GetActionName(actionID), 10);
            DrawText(GetActionName(actionID), (int)(itemRect.x + (itemRect.width - textW)/2), (int)itemRect.y + 10, 10, WHITE);
        }

        // Click ra ngoài vùng Popup thì đóng lại
        // (Kiểm tra cả vùng TypeRect và PopupRect)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
            !CheckCollisionPointRec(mouseVirtualPos, popupRect) &&
            !CheckCollisionPointRec(mouseVirtualPos, typeChangeRect)) {
             showActionPopup = false;
        }
    }

    // [S] SAVE
    if (IsKeyPressed(KEY_S)) Menu_SaveLayout();
}

// ---------------------------------------------
// TOOL 3: PROP CUTTER TOOL (PHÍM P)
// ---------------------------------------------


void Debug_RunPropTool(GameMap *map) {
    if (Inventory_IsActive() || Menu_IsActive()) return;
    if (IsKeyPressed(KEY_P)) {
        showPropTool = !showPropTool;
        if (showPropTool) {
            showMapDebug = false; // Tắt tool Map đi cho đỡ rối
            showMenuDebug = false; // tắt cái render debug cho không lỗi
            Menu_SetDebugMode(false); // [NEW] Tắt overlay menu
            printf(">> [DEBUG] PROP TOOL: ON (Keo chuot de cat vat the)\n");
        }
    }

    if (!showPropTool) return;

    //bật tắt xóa đồ vật render
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_K)) {
        isDeleteMode = !isDeleteMode;
        showDeleteConfirm = false;
    }

    // Vẽ hướng dẫn
    EndMode2D(); 
    if (isDeleteMode) {
        DrawDebugInfoBox("PROP DELETE", "Click Prop de XOA", "An toan tuyet doi!");
    } else {
        DrawDebugInfoBox("PROP TOOL (P)", "Keo chuot trai: Cat Anh", "Save de luu File");
    }
    
    BeginMode2D(gameCamera);

    Vector2 mouseWorld = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // ---- [CHÈN] LOGIC CLICK ĐỂ XÓA PROP (TRONG CAMERA) ----
    if (isDeleteMode && !showDeleteConfirm) {
        for (int i = 0; i < map->propCount; i++) {
            // Tạo hitbox bao quanh Prop để click
            Rectangle propHitbox = {
                map->props[i].position.x, map->props[i].position.y,
                map->props[i].sourceRec.width * map->scale, // Nhân scale vì vẽ ra màn hình nó to hơn
                map->props[i].sourceRec.height * map->scale
            };

            if (CheckCollisionPointRec(mouseWorld, propHitbox)) {
                DrawRectangleLinesEx(propHitbox, 2.0f, RED); // Highlight đỏ
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    targetDeleteRect.x = map->props[i].position.x; // Dùng vị trí làm ID định danh
                    targetDeleteRect.y = map->props[i].position.y;
                    targetDeleteType = 1; // 1 là Prop
                    showDeleteConfirm = true;
                }
            }
        }
    }

    // Lấy tọa độ chuột TRONG GAME (đã tính Camera)
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // [FIX 1] Biến chặn xuyên thấu cho Tool Prop
    bool isHoveringSaveProp = false;

    // --- UI XÁC NHẬN XÓA PROP & NÚT SAVE ---
    EndMode2D(); // Tat camera de ve UI
    
    if (showDeleteConfirm && targetDeleteType == 1) {
        Rectangle box = { (float)SCREEN_WIDTH/2 - 150, (float)SCREEN_HEIGHT/2 - 50, 300, 100 };
        DrawRectangleRec(box, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx(box, 2, WHITE);
        DrawText("XOA VAT THE?", (int)box.x + 80, (int)box.y + 20, 20, WHITE);
        
        if (DrawButton("YES", (Rectangle){box.x + 30, box.y + 60, 80, 30})) {
            for (int i = 0; i < map->propCount; i++) {
                if (IsFloatEqual(map->props[i].position.x, targetDeleteRect.x) &&
                    IsFloatEqual(map->props[i].position.y, targetDeleteRect.y)) {
                    DeletePropFromMemory(map, i);
                    break;
                }
            }
            SaveMapDataToFile(map); // Luu file
            showDeleteConfirm = false;
        }
        if (DrawButton("NO", (Rectangle){box.x + 190, box.y + 60, 80, 30})) {
            showDeleteConfirm = false;
        }
    }

    // NÚT SAVE PROPS
    if (tempPropCount > 0 && !isDeleteMode) {
        Rectangle btnSave = { 10, 200, 200, 40 };
        DrawRectangleRec(btnSave, PURPLE); 
        DrawRectangleLinesEx(btnSave, 2, WHITE);
        DrawText("SAVE PROPS", (int)btnSave.x + 40, (int)btnSave.y + 10, 20, WHITE);

        Vector2 mouseUI = GetVirtualMousePos();
        
        // Kiểm tra va chạm chuột
        if (CheckCollisionPointRec(mouseUI, btnSave)) {
            isHoveringSaveProp = true; // [FIX 1] Đang chạm nút
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // Chuyen tu nhap sang that
                for (int i = 0; i < tempPropCount; i++) {
                    Rectangle r = tempProps[i];
                    float s = map->scale; 
                    if (map->propCount < MAX_MAP_PROPS) {
                        map->props[map->propCount++] = (GameProp){ 
                            (Rectangle){r.x/s, r.y/s, r.width/s, r.height/s}, 
                            (Vector2){r.x, r.y}, 
                            r.height, 
                            &map->layerTexture 
                        };
                    }
                }
                tempPropCount = 0;      // Reset -> Nút ẩn đi
                SaveMapDataToFile(map); // Ghi file
            }
        }
    }
    
    BeginMode2D(gameCamera); // Bat lai neu co gi phia sau

    // 1. Logic Kéo Thả
    // [FIX 1] Thêm điều kiện !isHoveringSaveProp
    if (!isDeleteMode && !showDeleteConfirm && !isHoveringSaveProp) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isPropDragging = true;
            propStartPos = mouseWorldPos; 
        }
        
        if (isPropDragging) {
            Vector2 currentPos = mouseWorldPos;
            Rectangle rect = {
                (propStartPos.x < currentPos.x) ? propStartPos.x : currentPos.x,
                (propStartPos.y < currentPos.y) ? propStartPos.y : currentPos.y,
                (float)abs((int)(currentPos.x - propStartPos.x)),
                (float)abs((int)(currentPos.y - propStartPos.y))
            };
            
            // Vẽ khung nháp màu SKYBLUE (Xanh da trời)
            DrawRectangleLinesEx(rect, 2.0f, SKYBLUE);

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                isPropDragging = false;
                
                // [FIX 2] Chỉ thêm vật thể nếu kích thước > 5 (Chống click nhầm)
                if (rect.width > 2 && rect.height > 2) {
                    if (tempPropCount < 50) tempProps[tempPropCount++] = rect;
                } else {
                     printf(">> [DEBUG] Ignored tiny prop (Click noise)\n");
                }
            }
        } 
    }

    // 2. Vẽ lại các vùng đã cắt tạm thời
    for (int i = 0; i < tempPropCount; i++) {
        DrawRectangleLinesEx(tempProps[i], 1.0f, SKYBLUE);
        DrawLine(tempProps[i].x, tempProps[i].y + tempProps[i].height, 
                 tempProps[i].x + tempProps[i].width, tempProps[i].y + tempProps[i].height, SKYBLUE); // Vẽ đáy
    }
    
    // 3. Vẽ các Prop ĐÃ CÓ trong map (Màu xanh dương)
    for (int i=0; i<map->propCount; i++) {
         DrawRectangleLinesEx(map->props[i].sourceRec, 1.0f, BLUE);
    }
    
    // Undo
    if (IsKeyPressed(KEY_C) && tempPropCount > 0) {
        tempPropCount--;
        printf(">> [DEBUG] Undo Last Prop.\n");
    }
}


// ---------------------------------------------
// TOOL 4: DIALOG EDITOR (IN-GAME)
// ---------------------------------------------
void Debug_OpenDialogTool(Npc* targetNpc) {
    showDialogDebug = true;
    currentDialogNpc = targetNpc;
    selectedEvent = NULL;
    activeTextBoxIndex = -1;
    dialogScrollY = 0.0f;
    showDialogDeleteConfirm = false;
    GuiSetFont(globalFont); // Đảm bảo dùng font tiếng Việt cho UI
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKBLUE));
    

    
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(BLUE));
    // 2. Đổi màu viền của các thành phần thành màu Trắng (để nổi bật trên nền xanh)
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(WHITE));
    
    // 3. Đổi màu chữ mặc định thành Trắng để dễ đọc
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
    
    // 4. (Tùy chọn) Đổi màu nền của Nút bấm (Button) thành một màu xanh nhạt hơn một chút
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(BLUE));

    // Ép màu viền (và chữ) của cái khung GroupBox thành màu Trắng
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(WHITE));

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);   // Ép Raygui dùng đúng kích thước font (thay 16 bằng cỡ gốc của bạn nếu khác)
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1); // Chỉnh lại khoảng cách giữa các ký tự cho gọn gàng
    printf(">> [DEBUG] DIALOG TOOL: Mở cho NPC %s (ID: %d)\n", targetNpc->name, targetNpc->id);
}

void Debug_CloseDialogTool() {
    showDialogDebug = false;
    currentDialogNpc = NULL;
    printf(">> [DEBUG] DIALOG TOOL: Đã đóng.\n");
}

bool IsDialogDebugActive() {
    return showDialogDebug;
}

// Hàm hỗ trợ chèn dòng nội bộ
static void InsertDialogLine(DialogEvent* ev, int index, int speaker) {
    if (ev->lineCount >= MAX_LINES_PER_EVENT) return;
    for (int i = ev->lineCount; i > index + 1; i--) {
        ev->lines[i] = ev->lines[i-1];
    }
    ev->lines[index + 1].speakerType = speaker;
    strcpy(ev->lines[index + 1].content, "");
    ev->lineCount++;
}

// ---------------------------------------------
// TOOL 4: DIALOG EDITOR (IN-GAME) - VIRTUAL MOUSE FIX
// ---------------------------------------------
void Debug_RunDialogTool() {
    if (!showDialogDebug || currentDialogNpc == NULL) return;

    // --- TUYỆT CHIÊU: ÉP CHUỘT CHO RAYGUI THEO TỶ LỆ MÀN HÌNH ẢO ---
    float scaleW = (float)GetScreenWidth() / SCREEN_WIDTH;
    float scaleH = (float)GetScreenHeight() / SCREEN_HEIGHT;
    float scale = (scaleW < scaleH) ? scaleW : scaleH;

    Vector2 offset = {
        (GetScreenWidth() - (SCREEN_WIDTH * scale)) * 0.5f,
        (GetScreenHeight() - (SCREEN_HEIGHT * scale)) * 0.5f
    };

    // Bắt Raygui hiểu tọa độ chuột theo khung chuẩn 800x450
    SetMouseOffset((int)-offset.x, (int)-offset.y);
    SetMouseScale(1.0f / scale, 1.0f / scale);
    // ----------------------------------------------------------------

    float sw = (float)SCREEN_WIDTH;  // Luôn cố định là 800
    float sh = (float)SCREEN_HEIGHT; // Luôn cố định là 450

    // 1. Phủ mờ màn hình game bên dưới
    DrawRectangle(0, 0, (int)sw, (int)sh, Fade(BLACK, 0.85f));

    // 2. Vẽ Panel Khung chính
    Rectangle mainPanel = { 30, 30, sw - 60, sh - 60 };
    
    // [SỬA Ở ĐÂY]: Bắt sự kiện click vào nút [X] của chính cái bảng
    bool isCloseClicked = GuiWindowBox(mainPanel, TextFormat("CHỈNH SỬA HỘI THOẠI - NPC: %s (ID: %d)", currentDialogNpc->name, currentDialogNpc->id));

    // Nút Đóng & Save
    // Nếu bấm nút [X] HOẶC bấm nút DONG (ESC) thì đều đóng Tool
    if (isCloseClicked || GuiButton((Rectangle){ mainPanel.x + mainPanel.width - 90, mainPanel.y + 30, 80, 25 }, "ĐÓNG")) {
        Debug_CloseDialogTool();
        
        // [CỰC KỲ QUAN TRỌNG]: Phải trả lại chuột ảo cho game trước khi thoát
        SetMouseOffset(0, 0);
        SetMouseScale(1.0f, 1.0f);
        
        return; // <--- CỨU TINH Ở ĐÂY! Thoát ngay lập tức để không chạy đoạn code bên dưới (tránh Crash).
    }

    if (GuiButton((Rectangle){ mainPanel.x + mainPanel.width - 180, mainPanel.y + 30, 80, 25 }, "LƯU")) {
        Dialog_SaveToFile("resources/font_dialog/dialogs.txt");
    }

    // --- CỘT TRÁI: DANH SÁCH SỰ KIỆN ---
    Rectangle leftPanel = { mainPanel.x + 10, mainPanel.y + 65, 160, mainPanel.height - 75 };
    GuiGroupBox(leftPanel, "Các Sự Kiện (Key)");

    int eventCount = Dialog_GetNpcEventCount(currentDialogNpc->id);
    for (int i = 0; i < eventCount; i++) {
        DialogEvent* ev = Dialog_GetNpcEventByIndex(currentDialogNpc->id, i);
        if (!ev) continue;

        Rectangle btnRec = { leftPanel.x + 10, leftPanel.y + 20 + i * 30, 140, 25 };
        
        if (selectedEvent == ev) DrawRectangleRec(btnRec, Fade(BLUE, 0.5f));

        if (GuiButton(btnRec, ev->key)) {
            selectedEvent = ev;
            activeTextBoxIndex = -1; // Tắt chế độ gõ
        }
    }

    // Nút tạo sự kiện mới
    if (GuiButton((Rectangle){ leftPanel.x + 10, leftPanel.y + 20 + eventCount * 30, 140, 25 }, "+ SỰ KIỆN MỚI")) {
        selectedEvent = Dialog_CreateEvent(currentDialogNpc->id, "NEW");
    }

    // --- CỘT PHẢI: CHI TIẾT CÂU THOẠI ---
    Rectangle rightPanel = { leftPanel.x + 170, mainPanel.y + 65, mainPanel.width - 180, mainPanel.height - 75 };
    GuiGroupBox(rightPanel, "Nội dung thoại");

    if (selectedEvent != NULL) {
        
        // --- [THÊM MỚI] Ô ĐỔI TÊN SỰ KIỆN (KEY) ---
        DrawTextEx(globalFont, "Tên sự kiện (Key):", (Vector2){ rightPanel.x + 10, rightPanel.y + 20 }, 16, 1, YELLOW);
        Rectangle keyRec = { rightPanel.x + 150, rightPanel.y + 15, rightPanel.width - 160, 25 };
        
        // Dùng index -2 để phân biệt với các TextBox nhập thoại (từ 0 trở đi)
        bool isEditingKey = (activeTextBoxIndex == -2);
        if (GuiTextBox(keyRec, selectedEvent->key, 32, isEditingKey)) {
            activeTextBoxIndex = isEditingKey ? -1 : -2; 
        }
        // ------------------------------------------

        dialogScrollY += GetMouseWheelMove() * 20.0f;
        if (dialogScrollY > 0) dialogScrollY = 0;

        // [SỬA] Đẩy startY xuống một chút (cộng thêm 60 thay vì 20) để nhường chỗ cho ô Đổi tên phía trên
        float startY = rightPanel.y + 60 + dialogScrollY;

        // Khởi tạo tọa độ Y bắt đầu
        float currentY = rightPanel.y + 60 + dialogScrollY;

        for (int i = 0; i < selectedEvent->lineCount; i++) {
            
            // --- 1. VẼ DÒNG THOẠI CHÍNH ---
            // Kiểm tra xem dòng này có nằm trong khung hiển thị không (tránh vẽ tràn ra ngoài khi cuộn)
            if (currentY > rightPanel.y + 30 && currentY < rightPanel.y + rightPanel.height - 40) {
                
                // Nút Đổi Người Nói
                Rectangle speakerBtn = { rightPanel.x + 10, currentY, 80, 25 };
                const char* speakerText = (selectedEvent->lines[i].speakerType == 0) ? "NPC Nói" : "Player Nói";
                if (GuiButton(speakerBtn, speakerText)) {
                    selectedEvent->lines[i].speakerType = (selectedEvent->lines[i].speakerType == 0) ? 1 : 0;
                }

                // Textbox nhập liệu thoại chính
                Rectangle textRec = { rightPanel.x + 100, currentY, rightPanel.width - 200, 25 };
                bool isEditing = (activeTextBoxIndex == i);
                if (GuiTextBox(textRec, selectedEvent->lines[i].content, MAX_DIALOG_LENGTH, isEditing)) {
                    activeTextBoxIndex = isEditing ? -1 : i; 
                }

                // Nút Xóa (X)
                if (GuiButton((Rectangle){ textRec.x + textRec.width + 10, currentY, 25, 25 }, "X")) {
                    showDialogDeleteConfirm = true;
                    lineToDelete = i;
                }

                // Nút Thêm/Bớt Lựa Chọn (+ / -)
                if (GuiButton((Rectangle){ textRec.x + textRec.width + 40, currentY, 25, 25 }, "+")) {
                    if (selectedEvent->lines[i].choiceCount < MAX_CHOICES) selectedEvent->lines[i].choiceCount++;
                }
                if (GuiButton((Rectangle){ textRec.x + textRec.width + 70, currentY, 25, 25 }, "-")) {
                    if (selectedEvent->lines[i].choiceCount > 0) selectedEvent->lines[i].choiceCount--;
                }
            }

            // Dịch Y xuống 30px để chuẩn bị vẽ vùng Lựa chọn
            currentY += 30;

            // --- 2. VẼ CÁC LỰA CHỌN (NẾU CÓ) ---
            for (int c = 0; c < selectedEvent->lines[i].choiceCount; c++) {
                if (currentY > rightPanel.y + 30 && currentY < rightPanel.y + rightPanel.height - 40) {
                    DrawTextEx(globalFont, TextFormat("Lựa chọn %d:", c+1), (Vector2){ rightPanel.x + 10, currentY + 5 }, 16, 1, GREEN);
                    
                    // Ô Text Lựa Chọn
                    Rectangle cTextRec = { rightPanel.x + 100, currentY, rightPanel.width - 260, 25 };
                    bool isEditingChoice = (activeTextBoxIndex == (i * 100 + c + 1000)); 
                    if (GuiTextBox(cTextRec, selectedEvent->lines[i].choices[c], 64, isEditingChoice)) {
                        activeTextBoxIndex = isEditingChoice ? -1 : (i * 100 + c + 1000);
                    }

                    // Ô Key (Nhánh)
                    Rectangle cKeyRec = { rightPanel.x + rightPanel.width - 150, currentY, 100, 25 };
                    bool isEditingKey = (activeTextBoxIndex == (i * 100 + c + 2000)); 
                    if (GuiTextBox(cKeyRec, selectedEvent->lines[i].nextKeys[c], 32, isEditingKey)) {
                        activeTextBoxIndex = isEditingKey ? -1 : (i * 100 + c + 2000);
                    }
                }
                currentY += 30; // Dịch Y xuống cho lựa chọn tiếp theo
            }

            // --- 3. VẼ NÚT THÊM CÂU THOẠI (+ NPC / + Player) ---
            if (currentY > rightPanel.y + 30 && currentY < rightPanel.y + rightPanel.height - 40) {
                if (GuiButton((Rectangle){ rightPanel.x + 100, currentY, 80, 20 }, "+ NPC")) {
                    InsertDialogLine(selectedEvent, i, 0);
                }
                if (GuiButton((Rectangle){ rightPanel.x + 190, currentY, 80, 20 }, "+ Player")) {
                    InsertDialogLine(selectedEvent, i, 1);
                }
            }
            
            // Dịch Y xuống thêm 35px để tạo khoảng cách (Padding) với Dòng thoại của người tiếp theo
            currentY += 35; 
        }

        if (selectedEvent->lineCount == 0) {
            if (GuiButton((Rectangle){ rightPanel.x + 10, startY, 150, 25 }, "TẠO CÂU THOẠI ĐẦU")) {
                selectedEvent->lines[0].speakerType = 0;
                strcpy(selectedEvent->lines[0].content, "...");
                selectedEvent->lineCount = 1;
            }
        }
    } else {
        DrawTextEx(globalFont, "Hãy chọn 1 Sự Kiện ở cột bên trái!", (Vector2){ rightPanel.x + 20, rightPanel.y + 30 }, 20, 1, YELLOW);
    }

    // --- POPUP XÓA ---
    if (showDialogDeleteConfirm) {
        Rectangle popup = { sw/2 - 120, sh/2 - 60, 240, 120 };
        DrawRectangleRec(popup, Fade(BLACK, 0.95f));
        DrawRectangleLinesEx(popup, 2, RED);
        DrawTextEx(globalFont, "BẠN CÓ CHẮC CHẮN XÓA?", (Vector2){ popup.x + 20, popup.y + 20 }, 20, 1, WHITE);
        
        if (GuiButton((Rectangle){ popup.x + 30, popup.y + 60, 80, 30 }, "XÓA")) {
            for (int k = lineToDelete; k < selectedEvent->lineCount - 1; k++) {
                selectedEvent->lines[k] = selectedEvent->lines[k+1];
            }
            selectedEvent->lineCount--;
            showDialogDeleteConfirm = false;
        }
        if (GuiButton((Rectangle){ popup.x + 130, popup.y + 60, 80, 30 }, "HỦY")) {
            showDialogDeleteConfirm = false;
        }
    }

    // --- TRẢ LẠI CHUỘT NHƯ CŨ (QUAN TRỌNG: Tránh làm loạn click của các Menu khác) ---
    SetMouseOffset(0, 0);
    SetMouseScale(1.0f, 1.0f);
    // --------------------------------------------------------------------------------

    // Đóng bằng phím ESC
    if (IsKeyPressed(KEY_ESCAPE) && activeTextBoxIndex == -1) {
        Debug_CloseDialogTool();
    }
}