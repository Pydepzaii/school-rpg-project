// FILE: src/debug.c
#include "debug.h"
#include "settings.h" 
#include "camera.h"   
#include <stdio.h> 
#include <player.h>
#include <interact.h>
#include <stdlib.h> 

// --- STATE QUẢN LÝ RIÊNG BIỆT ---
static bool showMapDebug = false;  
static bool showMenuDebug = false; 
static bool showDebugUI = true;    
static bool showDrawFrame = false;
// [MAP TOOL DATA]
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
    if (IsKeyPressed(KEY_ZERO)) {
        showMapDebug = !showMapDebug;
        if (showMapDebug) {
            showMenuDebug = false; 
            printf(">> [DEBUG] MAP TOOL: ON (Camera Sync Active)\n");
        } 
    }
    
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMapDebug) return;
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

    Vector2 mouseWorldPos = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // 1. Tạm thời tắt Camera để vẽ UI
    EndMode2D();
    DrawDebugInfoBox("MAP TOOL (0)", "Keo: Blue | Tha: Green", "Copy Code -> Map.c -> Red");
    
    // 2. Bật lại Camera để vẽ tường
    BeginMode2D(gameCamera);

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
    // [SỬA]: Khôi phục logic tính toán dựa trên Texture NPC (giống player.c)
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID) {
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            float npcH = (float)npcList[i].texture.height;

            float offsetX = (npcW - npcList[i].hitWidth) / 2.0f;
            Rectangle npcHitbox = { 
                npcList[i].position.x + offsetX,            
                npcList[i].position.y + npcH - npcList[i].paddingBottom - npcList[i].hitHeight,
                npcList[i].hitWidth, 
                npcList[i].hitHeight                       
            };

            DrawRectangleRec(npcHitbox, Fade(PURPLE, 0.5f));
            DrawRectangleLinesEx(npcHitbox, 1.0f, PURPLE);
        }
    }

    // Logic Kéo Thả
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
            if (tempMapWallCount < MAX_TEMP_WALLS) {
                tempMapWalls[tempMapWallCount++] = rect;
            }
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
// ---------------------------------------------
void Debug_RunMenuTool() {
    if (IsKeyPressed(KEY_EQUAL)) {
        showMenuDebug = !showMenuDebug;
        if (showMenuDebug) {
            showMapDebug = false; 
            printf(">> [DEBUG] MENU TOOL: ON\n");
        } 
    }

    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    if (!showMenuDebug) return;

    DrawDebugInfoBox("MENU TOOL (=)", "Do Nut Menu UI", "Copy Code -> Menu.c");

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