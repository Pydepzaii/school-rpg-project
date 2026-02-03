// FILE: src/debug.c
#include "raylib.h"
#include "debug.h"
<<<<<<< Updated upstream
#include "settings.h" 
#include "camera.h"   
#include <stdio.h> 
#include <player.h>
#include <interact.h>
=======
#include "map.h"      // [FIX] Đổi <> thành "" cho chuẩn file nội bộ
#include "settings.h" 
#include "camera.h"   
#include <stdio.h> 
#include "player.h"   // [FIX] Đổi <> thành ""
#include "interact.h" // [FIX] Đổi <> thành ""
>>>>>>> Stashed changes
#include <stdlib.h> 

// --- STATE QUẢN LÝ RIÊNG BIỆT ---
static bool showMapDebug = false;  
static bool showMenuDebug = false; 
static bool showDebugUI = true;    
static bool showDrawFrame = false;
<<<<<<< Updated upstream
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

=======

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

>>>>>>> Stashed changes
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
            // 2. VẼ VÙNG TƯƠNG TÁC (Vòng Tròn Vàng) - CẦN SỬA ĐOẠN NÀY
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
<<<<<<< Updated upstream
        }
    }

    if (IsKeyPressed(KEY_C) && tempBtnCount > 0) {
        tempBtnCount--;
        printf(">> [DEBUG] Undo Last Menu Button.\n");
=======
        }
    }

    if (IsKeyPressed(KEY_C) && tempBtnCount > 0) {
        tempBtnCount--;
        printf(">> [DEBUG] Undo Last Menu Button.\n");
    }
}

// ---------------------------------------------
// TOOL 3: PROP CUTTER TOOL (PHÍM P)
// ---------------------------------------------
// Biến toàn cục cho Tool Prop
static bool showPropTool = false;
static bool isPropDragging = false;
static Vector2 propStartPos = {0};
static Rectangle tempProps[50]; 
static int tempPropCount = 0;

void Debug_RunPropTool(GameMap *map) {
    if (IsKeyPressed(KEY_P)) {
        showPropTool = !showPropTool;
        if (showPropTool) {
            showMapDebug = false; // Tắt tool Map đi cho đỡ rối
            printf(">> [DEBUG] PROP TOOL: ON (Keo chuot de cat vat the)\n");
        }
    }

    if (!showPropTool) return;

    // Vẽ hướng dẫn
    DrawDebugInfoBox("PROP TOOL (P)", "Keo chuot trai: Cat Anh", "Nha chuot: Them code");

    // Lấy tọa độ chuột TRONG GAME (đã tính Camera)
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetVirtualMousePos(), gameCamera);

    // 1. Logic Kéo Thả
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
            if (tempPropCount < 50) tempProps[tempPropCount++] = rect;
            
            // IN CODE RA CONSOLE ĐỂ BẠN COPY
            // Format: sourceX, sourceY, width, height, originY
            float s = map->scale; 
            
            printf("map->props[map->propCount++] = (GameProp){ (Rectangle){%.0f, %.0f, %.0f, %.0f}, (Vector2){%.0f, %.0f}, %.0f, &map->layerTexture };\n", 
                   rect.x / s, rect.y / s, rect.width / s, rect.height / s, // Source Rec (Chia 2)
                   rect.x, rect.y,                                          // Position (Giữ nguyên)
                   rect.height);
        }
    } // [FIX] Đã thêm dấu ngoặc đóng ở đây (Lỗi cũ của bạn là thiếu cái này)

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
>>>>>>> Stashed changes
    }
}