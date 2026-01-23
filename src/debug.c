// FILE: src/debug.c
#include "debug.h"
#include "interact.h" 
#include <stdio.h> 

// --- CONFIG ---
#define MAX_TEMP_WALLS 100 

// --- STATE VARIABLES ---
static Vector2 devStartPos = {0};       
static bool devIsDragging = false;      
static bool showDebugWalls = false;     // Biến tổng (Phím 0)
static bool showDebugUI = true;         // Biến bật tắt bảng ghi chú (Phím V)

static Rectangle tempWalls[MAX_TEMP_WALLS];
static int tempWallCount = 0;

void Debug_UpdateAndDraw(GameMap *map, Player *player, Npc *npcList, int npcCount) {
    // 1. INPUT HANDLING
    // Phím 0: Bật/Tắt toàn bộ chế độ Debug (Master Switch)
    if (IsKeyPressed(KEY_ZERO)) showDebugWalls = !showDebugWalls; 

    // Phím C: Xóa tường vừa vẽ (Undo)
    if (IsKeyPressed(KEY_C)) { 
        if (tempWallCount > 0) {
            tempWallCount--; 
            printf("--- UNDO LAST WALL ---\n");
        }
    }

    // [MỚI] Phím V: Chỉ bật/tắt bảng ghi chú hướng dẫn
    if (IsKeyPressed(KEY_V)) showDebugUI = !showDebugUI;

    // 2. RENDER & LOGIC LOOP
    if (showDebugWalls) {
        
        // --- LAYER 1: TƯỜNG CŨ ---
        DrawMapDebug(map); 
        
        // --- LAYER 2: TƯỜNG MỚI ---
        for (int i = 0; i < tempWallCount; i++) {
            DrawRectangleRec(tempWalls[i], Fade(LIME, 0.5f)); 
            DrawRectangleLinesEx(tempWalls[i], 3.0f, LIME);   
        }

        // --- LAYER 3: NPC (HITBOX & INTERACT RANGE) ---
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == map->currentMapID) {
                float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
                float npcH = (float)npcList[i].texture.height;

                // --- HITBOX LOGIC ---
                float boxWidth = 24.0f;  
                float boxHeight = 10.0f; 
                float paddingBottom = 17.0f;  
                float offsetX = (npcW - boxWidth) / 2.0f;

                Rectangle npcHitbox = { 
                    npcList[i].position.x + offsetX,            
                    npcList[i].position.y + npcH - boxWidth - paddingBottom, 
                    boxWidth,  
                    boxHeight                           
                };

                // 1. Vẽ Hitbox Vật lý (Tím)
                DrawRectangleRec(npcHitbox, Fade(PURPLE, 0.5f));
                DrawRectangleLinesEx(npcHitbox, 2.0f, PURPLE);
                
                // 2. Vẽ điểm Pivot Sort Y (Vàng chấm nhỏ)
                DrawCircle(npcHitbox.x + npcHitbox.width/2, npcHitbox.y + npcHitbox.height, 3, YELLOW);

                // 3. Vẽ Phạm vi tương tác (MÀU XANH DƯƠNG ĐẬM)
                Vector2 centerPos = { 
                    npcList[i].position.x + (npcW / 2), 
                    npcList[i].position.y + (npcH / 2) 
                };

                // Vẽ vòng tròn
                DrawCircleV(centerPos, INTERACT_DISTANCE, Fade(DARKBLUE, 0.3f));
                DrawCircleLines((int)centerPos.x, (int)centerPos.y, INTERACT_DISTANCE, DARKBLUE);
            }
        }

        // --- LAYER 4: HITBOX PLAYER (MÀU VÀNG) ---
        float pW = (float)player->spriteWidth;
        float pH = (float)player->spriteHeight;
        float pFeetH = 20.0f;

        Rectangle playerHitbox = {
            player->position.x + 15,
            player->position.y + pH - pFeetH,
            player->spriteWidth - 30,
            pFeetH
        };

        DrawRectangleRec(playerHitbox, Fade(YELLOW, 0.5f));
        DrawRectangleLinesEx(playerHitbox, 2.0f, YELLOW);
        DrawCircle(playerHitbox.x + playerHitbox.width/2, playerHitbox.y + playerHitbox.height, 3, YELLOW);

        // --- UI HƯỚNG DẪN (BẢNG ĐEN) ---
        // Chỉ vẽ bảng này nếu biến showDebugUI đang bật
        if (showDebugUI) {
            Rectangle infoBox = { 10, 60, 320, 125 }; // Tăng chiều cao một chút để chứa dòng hướng dẫn mới
            DrawRectangleRec(infoBox, Fade(BLACK, 0.85f)); 
            DrawRectangleLinesEx(infoBox, 2, WHITE);       

            DrawText("CH CHE DO: DEBUG (Phim 0)", infoBox.x + 15, infoBox.y + 10, 20, RED);
            
            DrawText("- [TIM]  : Hitbox Vat ly", infoBox.x + 15, infoBox.y + 40, 18, VIOLET);
            DrawText("- [XANH] : Pham vi [E]", infoBox.x + 15, infoBox.y + 65, 18, SKYBLUE); 
            
            // Hướng dẫn phím tắt mới
            DrawText("- [V]    : An/Hien Bang Nay", infoBox.x + 15, infoBox.y + 95, 18, GRAY);
        }


        // --- LAYER 5: TOOL VẼ ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            devStartPos = GetMousePosition();
            devIsDragging = true;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && devIsDragging) {
            Vector2 currentPos = GetMousePosition();
            float width = currentPos.x - devStartPos.x;
            float height = currentPos.y - devStartPos.y;
            Rectangle rec = { devStartPos.x, devStartPos.y, width, height };
            DrawRectangleRec(rec, Fade(BLUE, 0.5f)); 
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && devIsDragging) {
            devIsDragging = false;
            Vector2 endPos = GetMousePosition();
            float x = devStartPos.x;
            float y = devStartPos.y;
            float w = endPos.x - devStartPos.x;
            float h = endPos.y - devStartPos.y;
            if (w < 0) { x += w; w = -w; }
            if (h < 0) { y += h; h = -h; }
            if (tempWallCount < MAX_TEMP_WALLS) {
                tempWalls[tempWallCount] = (Rectangle){ x, y, w, h };
                tempWallCount++;
                printf("map->walls[map->wallCount++] = (Rectangle){ %.0f, %.0f, %.0f, %.0f };\n", x, y, w, h);
            }
        }
    } 
}