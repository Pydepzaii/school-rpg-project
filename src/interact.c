// FILE: src/interact.c
#include "interact.h"
#include "raymath.h" 
#include "ui_style.h" 
#include "settings.h" // Cần để dùng MAP_ID
#include "dialog_system.h"
#include <string.h>
// [MỚI] Tọa độ cửa (Lấy từ code Dev)
static Vector2 exitToBlack = { 100.0f, 350.0f };
static Vector2 exitToWhite = { 700.0f, 350.0f };

// [MỚI] Hàm logic kiểm tra cửa
void Interact_CheckExits(Player *player, GameMap *map) {
    if (map->currentMapID == MAP_THU_VIEN) {
        // Cửa sang Map Đen
        if (Vector2Distance(player->position, exitToBlack) < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                LoadMap(map, MAP_DEN);
                player->position = (Vector2){ 400, 200 }; 
            }
        }
        // Cửa sang Map Trắng
        if (Vector2Distance(player->position, exitToWhite) < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                LoadMap(map, MAP_TRANG);
                player->position = (Vector2){ 400, 200 }; 
            }
        }
    }
    // Nếu đang ở map phụ thì bấm E để về lại Thư Viện
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
         if (IsKeyPressed(KEY_E)) {
             LoadMap(map, MAP_THU_VIEN);
             player->position = (Vector2){ 400, 250 };
         }
    }
}

void Interact_Update(Player *player, Npc *npcList, int npcCount, GameMap *map) {
    for (int i = 0; i < npcCount; i++) {
        // [ĐÃ SỬA] Logic Map Checking hoạt động
        if (npcList[i].mapID != map->currentMapID) continue;

        // Logic khoảng cách (Giữ nguyên code của bạn)
        float distance = Vector2Distance(player->position, npcList[i].position);

        if (distance < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                npcList[i].isTalking = !npcList[i].isTalking;
                // [MỚI] KHI BẮT ĐẦU NÓI, LẤY DỮ LIỆU TỪ FILE TXT
                if (npcList[i].isTalking) {
                    const char* text = Dialog_Get(npcList[i].mapID, npcList[i].id, npcList[i].dialogKey);
                    strcpy(npcList[i].currentText, text); // Copy vào biến hiển thị của NPC
                }
            }
        } else {
            npcList[i].isTalking = false;
        }
    }
    
    // [MỚI] Gọi hàm kiểm tra cửa
    Interact_CheckExits(player, map);
}

void Interact_DrawUI(Player *player, Npc *npcList, int npcCount, GameMap *map) {
    // 1. UI NPC (Code đẹp của bạn + Logic lọc map của Dev)
    for (int i = 0; i < npcCount; i++) {
        // Chỉ vẽ NPC đang ở cùng map
        if (npcList[i].mapID != map->currentMapID) continue;

        float distance = Vector2Distance(player->position, npcList[i].position);

        // UI: Gợi ý nút bấm
        if (distance < INTERACT_DISTANCE && !npcList[i].isTalking) {
            Vector2 textPos = { npcList[i].position.x - 10, npcList[i].position.y - 40 };
            DrawTextEx(globalFont, "[E] Noi chuyen", textPos, 24, 1, YELLOW);
        }

        // UI: Hộp thoại (Giữ nguyên thiết kế đẹp của bạn)
        if (npcList[i].isTalking) {
            Rectangle boxRec = { 100, 300, 600, 120 };
            
            DrawRectangleRec(boxRec, Fade(BLACK, 0.8f));
            DrawRectangleLinesEx(boxRec, 2, WHITE);

            DrawTextEx(globalFont, npcList[i].name, (Vector2){ boxRec.x + 20, boxRec.y + 10 }, 28, 1, YELLOW);
            DrawTextEx(globalFont, npcList[i].currentText, (Vector2){ boxRec.x + 20, boxRec.y + 45 }, 24, 1, WHITE);
            DrawTextEx(globalFont, "Bam [E] de dong", (Vector2){ boxRec.x + 450, boxRec.y + 90 }, 18, 1, LIGHTGRAY);
        }
    }

    // 2. [MỚI] UI Cửa ra vào (Thêm vào để người chơi biết chỗ nào có cửa)
    if (map->currentMapID == MAP_THU_VIEN) {
        if (Vector2Distance(player->position, exitToBlack) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Den", (Vector2){exitToBlack.x - 40, exitToBlack.y - 40}, 24, 1, GREEN);
        }
        if (Vector2Distance(player->position, exitToWhite) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Trang", (Vector2){exitToWhite.x - 40, exitToWhite.y - 40}, 24, 1, GREEN);
        }   
    }
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
        DrawTextEx(globalFont, "[E] Ve Thu Vien", (Vector2){10, 10}, 24, 1, GREEN);
    }
}