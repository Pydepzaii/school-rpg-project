// FILE: src/debug.c
#include "debug.h"
#include <stdio.h> 
#include <stdlib.h> 

// --- STATE QUẢN LÝ RIÊNG BIỆT ---
static bool showMapDebug = false;  // Biến của Map Tool (Phím 0)
static bool showMenuDebug = false; // Biến của Menu Tool (Phím =)
static bool showDebugUI = true;    // Biến bật/tắt bảng hướng dẫn (Phím V)

// [MAP TOOL DATA]
#define MAX_TEMP_WALLS 100
static bool isMapDragging = false;
static Vector2 mapStartPos = {0};
static Rectangle tempMapWalls[MAX_TEMP_WALLS]; // Tường nháp (Xanh Lá)
static int tempMapWallCount = 0;

// [MENU TOOL DATA]
#define MAX_DEBUG_BUTTONS 100
static bool isMenuDragging = false;
static Vector2 menuStartPos = {0};
static Rectangle tempButtons[MAX_DEBUG_BUTTONS]; // Nút nháp (Xanh Lá)
static int tempBtnCount = 0;

// Hàm trả về trạng thái (cho menu_system dùng)
bool IsMenuDebugActive() { return showMenuDebug; }

// Hàm tắt cưỡng chế (Gọi khi vào game)
void Debug_ForceCloseMenuTool() {
    showMenuDebug = false;
    printf(">> [DEBUG] Force Closed Menu Tool.\n");
}

// Hàm vẽ bảng hướng dẫn màu đen (Helper)
void DrawDebugInfoBox(const char* title, const char* line1, const char* line2) {
    if (!showDebugUI) return; 

    float scrW = (float)GetScreenWidth();
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
    // 1. Logic Bật/Tắt
    if (IsKeyPressed(KEY_ZERO)) {
        showMapDebug = !showMapDebug;
        if (showMapDebug) {
            showMenuDebug = false; // Tắt Tool Menu
            printf(">> [DEBUG] MAP TOOL: ON\n");
        } else {
            tempMapWallCount = 0; // Xóa nháp
        }
    }
    
    // Toggle UI hướng dẫn
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMapDebug) return;

    // 2. Vẽ Bảng Hướng Dẫn
    DrawDebugInfoBox("MAP TOOL (0)", 
                     "Keo: Blue | Tha: Green", 
                     "Copy Code -> Map.c -> Red");

    // 3. Vẽ Tường THẬT (Màu ĐỎ)
    DrawMapDebug(map); 
    DrawRectangleLinesEx(player->frameRec, 1.0f, GREEN); 
    
    // 4. Vẽ Tường NHÁP (Màu XANH LÁ)
    for (int i = 0; i < tempMapWallCount; i++) {
        DrawRectangleLinesEx(tempMapWalls[i], 2.0f, GREEN);
        DrawRectangleRec(tempMapWalls[i], Fade(GREEN, 0.2f)); 
    }

    // 5. Logic Kéo Thả (Màu XANH DƯƠNG)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isMapDragging = true;
        mapStartPos = GetMousePosition();
    }
    
    if (isMapDragging) {
        Vector2 currentPos = GetMousePosition();
        Rectangle rect = {
            (mapStartPos.x < currentPos.x) ? mapStartPos.x : currentPos.x,
            (mapStartPos.y < currentPos.y) ? mapStartPos.y : currentPos.y,
            (float)abs((int)(currentPos.x - mapStartPos.x)),
            (float)abs((int)(currentPos.y - mapStartPos.y))
        };
        
        DrawRectangleLinesEx(rect, 2.0f, BLUE); 

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isMapDragging = false;
            // Chỉ lưu vào mảng NHÁP
            if (tempMapWallCount < MAX_TEMP_WALLS) {
                tempMapWalls[tempMapWallCount++] = rect;
            }
            // In code
            printf("map->walls[map->wallCount++] = (Rectangle){ %.0f, %.0f, %.0f, %.0f };\n", 
                   rect.x, rect.y, rect.width, rect.height);
        }
    }

    // 6. Undo (Phím C)
    if (IsKeyPressed(KEY_C) && tempMapWallCount > 0) {
        tempMapWallCount--;
        printf(">> [DEBUG] Undo Last Temp Wall.\n");
    }
}

// ---------------------------------------------
// TOOL 2: MENU DEBUG (PHÍM =)
// ---------------------------------------------
void Debug_RunMenuTool() {
    if (IsKeyPressed(KEY_EQUAL)) {
        showMenuDebug = !showMenuDebug;
        if (showMenuDebug) {
            showMapDebug = false; 
            printf(">> [DEBUG] MENU TOOL: ON\n");
        } else {
            tempBtnCount = 0; 
        }
    }

    // Toggle UI hướng dẫn
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMenuDebug) return;

    // Vẽ Bảng Hướng Dẫn
    DrawDebugInfoBox("MENU TOOL (=)", 
                     "Do Nut Menu UI", 
                     "Copy Code -> Menu.c -> Red");

    // Vẽ nháp (Xanh Lá)
    for (int i = 0; i < tempBtnCount; i++) {
        DrawRectangleRec(tempButtons[i], Fade(GREEN, 0.3f));
        DrawRectangleLinesEx(tempButtons[i], 2.0f, GREEN);
    }

    // Logic Kéo Thả (Xanh Dương)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isMenuDragging = true;
        menuStartPos = GetMousePosition();
    }

    if (isMenuDragging) {
        Vector2 currentPos = GetMousePosition();
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
            
            printf("\n// [COPIED] Code nut:\n");
            printf("if (DrawButton(\"NUT\", (Rectangle){ %.0f, %.0f, %.0f, %.0f })) { }\n", 
                   rect.x, rect.y, rect.width, rect.height);
        }
    }

    // [FIX] Thêm log báo Undo cho Menu Tool
    if (IsKeyPressed(KEY_C) && tempBtnCount > 0) {
        tempBtnCount--;
        printf(">> [DEBUG] Undo Last Menu Button.\n");
    }
}