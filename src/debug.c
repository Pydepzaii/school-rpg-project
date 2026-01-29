// FILE: src/debug.c
#include "debug.h"
#include "settings.h" // Để dùng GetVirtualMousePos()
#include "camera.h"   // [QUAN TRỌNG] Để lấy thông số gameCamera
#include <stdio.h> 
#include <stdlib.h> 

// --- STATE QUẢN LÝ RIÊNG BIỆT ---
static bool showMapDebug = false;  
static bool showMenuDebug = false; 
static bool showDebugUI = true;    

// [MAP TOOL DATA]
// [GIẢI THÍCH]: Mảng tạm thời lưu các bức tường vừa vẽ. Nó sẽ mất khi tắt game nếu không copy ra code.
#define MAX_TEMP_WALLS 100
static bool isMapDragging = false;
static Vector2 mapStartPos = {0};
static Rectangle tempMapWalls[MAX_TEMP_WALLS]; 
static int tempMapWallCount = 0;

// [MENU TOOL DATA]
#define MAX_DEBUG_BUTTONS 100
static bool isMenuDragging = false;
static Vector2 menuStartPos = {0};
static Rectangle tempButtons[MAX_DEBUG_BUTTONS]; 
static int tempBtnCount = 0;

bool IsMenuDebugActive() { return showMenuDebug; }

void Debug_ForceCloseMenuTool() {
    showMenuDebug = false;
}

void DrawDebugInfoBox(const char* title, const char* line1, const char* line2) {
    if (!showDebugUI) return; 

    // Vẽ UI luôn dùng toạ độ màn hình ảo (không bị camera tác động)
    float scrW = (float)SCREEN_WIDTH; // Dùng kích thước chuẩn 800
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
// Tool này vẽ BÊN TRONG thế giới game (bị Camera zoom)
// ---------------------------------------------
void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount) {
    if (IsKeyPressed(KEY_ZERO)) {
        showMapDebug = !showMapDebug;
        if (showMapDebug) {
            showMenuDebug = false; 
            printf(">> [DEBUG] MAP TOOL: ON (Camera Sync Active)\n");
        } 
        // [FIX MAP TOOL] Đã xóa dòng 'else { tempMapWallCount = 0; }'
        // Giờ tắt Debug Map (phím 0) thì tường xanh lá vẫn còn lưu.
    }
    
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMapDebug) return;

    // [QUAN TRỌNG] Lấy tọa độ chuột ĐÃ ĐƯỢC CHUYỂN ĐỔI sang thế giới game
    // [GIẢI THÍCH]: Vì Camera zoom và di chuyển, nên tọa độ chuột trên màn hình (Screen) 
    // KHÁC tọa độ trong game (World). Hàm này giúp chuyển đổi để vẽ tường đúng chỗ.
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // --- [SỬA LỖI UI BỊ TRÔI] ---
    // 1. Tạm thời tắt Camera để vẽ UI lên mặt kính màn hình (Screen Space)
    EndMode2D();
    
    DrawDebugInfoBox("MAP TOOL (0)", 
                     "Keo: Blue | Tha: Green", 
                     "Copy Code -> Map.c -> Red");

    // 2. Bật lại Camera để tiếp tục vẽ tường vào trong thế giới game (World Space)
    BeginMode2D(gameCamera);
    // ---------------------------

    // 2. Vẽ Tường THẬT
    DrawMapDebug(map); 
    DrawRectangleLinesEx(player->frameRec, 1.0f, GREEN); 
    
    // 3. Vẽ Tường NHÁP
    for (int i = 0; i < tempMapWallCount; i++) {
        DrawRectangleLinesEx(tempMapWalls[i], 2.0f, GREEN);
        DrawRectangleRec(tempMapWalls[i], Fade(GREEN, 0.2f)); 
    }

    // 4. Logic Kéo Thả (Dùng mouseWorldPos thay vì GetMousePosition)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isMapDragging = true;
        mapStartPos = mouseWorldPos; // Lưu vị trí bắt đầu trong map
    }
    
    if (isMapDragging) {
        // [GIẢI THÍCH]: Logic tính toán hình chữ nhật khi kéo chuột (tính width/height dương).
        Vector2 currentPos = mouseWorldPos; // Vị trí hiện tại trong map
        Rectangle rect = {
            (mapStartPos.x < currentPos.x) ? mapStartPos.x : currentPos.x,
            (mapStartPos.y < currentPos.y) ? mapStartPos.y : currentPos.y,
            (float)abs((int)(currentPos.x - mapStartPos.x)),
            (float)abs((int)(currentPos.y - mapStartPos.y))
        };
        
        DrawRectangleLinesEx(rect, 2.0f, BLUE); 

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isMapDragging = false;
            if (tempMapWallCount < MAX_TEMP_WALLS) {
                tempMapWalls[tempMapWallCount++] = rect;
            }
            // [OUTPUT]: In ra console để Dev copy vào code.
            printf("map->walls[map->wallCount++] = (Rectangle){ %.0f, %.0f, %.0f, %.0f };\n", 
                   rect.x, rect.y, rect.width, rect.height);
        }
    }

    if (IsKeyPressed(KEY_C) && tempMapWallCount > 0) {
        tempMapWallCount--;
        printf(">> [DEBUG] Undo Last Temp Wall.\n");
    }
}

// ---------------------------------------------
// TOOL 2: MENU DEBUG (PHÍM =)
// Tool này vẽ BÊN NGOÀI Camera (UI tĩnh)
// ---------------------------------------------
void Debug_RunMenuTool() {
    if (IsKeyPressed(KEY_EQUAL)) {
        showMenuDebug = !showMenuDebug;
        if (showMenuDebug) {
            showMapDebug = false; 
            printf(">> [DEBUG] MENU TOOL: ON\n");
        } 
        // [FIX MENU TOOL] Đã xóa dòng 'else { tempBtnCount = 0; }'
    }

    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMenuDebug) return;

    DrawDebugInfoBox("MENU TOOL (=)", "Do Nut Menu UI", "Copy Code -> Menu.c");

    // [QUAN TRỌNG] Menu không bị Camera ảnh hưởng, nên chỉ cần lấy chuột ảo (Virtual)
    // Không dùng GetScreenToWorld2D ở đây!
    Vector2 mouseVirtualPos = GetVirtualMousePos();

    for (int i = 0; i < tempBtnCount; i++) {
        DrawRectangleRec(tempButtons[i], Fade(GREEN, 0.3f));
        DrawRectangleLinesEx(tempButtons[i], 2.0f, GREEN);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isMenuDragging = true;
        menuStartPos = mouseVirtualPos;
    }

    if (isMenuDragging) {
        Vector2 currentPos = mouseVirtualPos;
        Rectangle rect = {
            (menuStartPos.x < currentPos.x) ? menuStartPos.x : currentPos.x,
            (menuStartPos.y < currentPos.y) ? menuStartPos.y : currentPos.y,
            (float)abs((int)(currentPos.x - menuStartPos.x)),
            (float)abs((int)(currentPos.y - menuStartPos.y))
        };
        
        DrawRectangleLinesEx(rect, 2.0f, BLUE);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isMenuDragging = false;
            if (tempBtnCount < MAX_DEBUG_BUTTONS) tempButtons[tempBtnCount++] = rect;
            
            printf("if (DrawButton(\"NUT\", (Rectangle){ %.0f, %.0f, %.0f, %.0f })) { }\n", 
                   rect.x, rect.y, rect.width, rect.height);
        }
    }

    if (IsKeyPressed(KEY_C) && tempBtnCount > 0) {
        tempBtnCount--;
        printf(">> [DEBUG] Undo Last Menu Button.\n");
    }
}