// FILE: src/interact.c
#include "interact.h"
#include "raymath.h" 
#include "ui_style.h" 
#include "settings.h" // Cần để dùng MAP_ID
#include "dialog_system.h"
#include "transition.h"
#include "ui_style.h"
#include <string.h>

// [MỚI] Tọa độ cửa (Lấy từ code Dev)
// [CẦN CHÚ Ý]: Các biến này đang Hard-code (gán cứng). Nếu bạn thay đổi Map, nhớ vào đây sửa tọa độ cửa.
static Vector2 exitToBlack = { 78.0f, 345.0f };
static Vector2 exitToWhite = { 720.0f, 345.0f };

// Hàm logic kiểm tra cửa
void Interact_CheckExits(Player *player, GameMap *map) {
   //LẤY HITBOX LÀM CHUẨN TÍNH KHOẢNG CÁCH TƯƠNG TÁC
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f), // Tâm X của hitbox
        player->position.y + offsetY + (player->hitHeight / 2.0f) // Tâm Y của hitbox
    };

    // [BƯỚC 2: DÙNG playerHitCenter ĐỂ TÍNH KHOẢNG CÁCH]
    if (map->currentMapID == MAP_THU_VIEN) {
        // Cửa sang Map Đen
        if (Vector2Distance(playerHitCenter, exitToBlack) < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                Transition_StartToMap(MAP_TRANG, (Vector2){ 400, 200 });
            }
        }
        // Cửa sang Map Trắng
        if (Vector2Distance(playerHitCenter, exitToWhite) < INTERACT_DISTANCE) {
            if (Transition_IsActive()) return;
            if (IsKeyPressed(KEY_E)) {
                Transition_StartToMap(MAP_DEN, (Vector2){ 400, 200 });
            }
        }
    }
    // Các map phụ về thư viện
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
         if (IsKeyPressed(KEY_E)) {
            Transition_StartToMap(MAP_THU_VIEN, (Vector2){ 400, 250 });
         }
    }
}
//PHẠM VI TƯƠNG TÁC NPC
void Interact_Update(Player *player, Npc *npcList, int npcCount, GameMap *map) {
    //  lẤY HITBOX LÀM MỐC ĐỂ TÍNH TOÁN PHẠM VI TƯƠNG TÁC
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f),
        player->position.y + offsetY + (player->hitHeight / 2.0f)
    };

    // VÒNG LẶP] CHECK NPC CÓ CÙNG MAP KHÔNG
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID != map->currentMapID) continue;
       // 1. Tính kích thước 1 khung hình của NPC
        float npcFrameW = (float)npcList[i].texture.width / npcList[i].frameCount;
        float npcFrameH = (float)npcList[i].texture.height;

        // 2. Tính khoảng cách từ góc ảnh đến góc Hitbox (Offset)
        float npcOffsetX = (npcFrameW - npcList[i].hitWidth) / 2.0f;
        float npcOffsetY = npcFrameH - npcList[i].hitHeight - npcList[i].paddingBottom;

        // 3. Tính TÂM HITBOX NPC (Tọa độ gốc + Offset + Bán kính Hitbox)
        Vector2 npcHitCenter = {
            npcList[i].position.x + npcOffsetX + (npcList[i].hitWidth / 2.0f),
            npcList[i].position.y + npcOffsetY + (npcList[i].hitHeight / 2.0f)
        };
        // 4. Đo khoảng cách giữa 2 cái TÂM HITBOX
        float distance = Vector2Distance(playerHitCenter, npcHitCenter);

        if (distance < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                npcList[i].isTalking = !npcList[i].isTalking;
                if (npcList[i].isTalking) {
                    const char* text = Dialog_Get(npcList[i].mapID, npcList[i].id, npcList[i].dialogKey);
                    strcpy(npcList[i].currentText, text);
                }
            }
        } else {
            npcList[i].isTalking = false;
<<<<<<< Updated upstream
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
=======
        }
    }
>>>>>>> Stashed changes
    
    Interact_CheckExits(player, map);
}

void Interact_DrawUI(Player *player, Npc *npcList, int npcCount, GameMap *map) {
   
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f),
        player->position.y + offsetY + (player->hitHeight / 2.0f)
    };

    // 1. UI NPC
    for (int i = 0; i < npcCount; i++) {
        // Chỉ vẽ NPC đang ở cùng map
        if (npcList[i].mapID != map->currentMapID) continue;

        
       float npcFrameW = (float)npcList[i].texture.width / npcList[i].frameCount;
        float npcFrameH = (float)npcList[i].texture.height;

        float npcOffsetX = (npcFrameW - npcList[i].hitWidth) / 2.0f;
        float npcOffsetY = npcFrameH - npcList[i].hitHeight - npcList[i].paddingBottom;

        Vector2 npcHitCenter = {
            npcList[i].position.x + npcOffsetX + (npcList[i].hitWidth / 2.0f),
            npcList[i].position.y + npcOffsetY + (npcList[i].hitHeight / 2.0f)
        };

        float distance = Vector2Distance(playerHitCenter, npcHitCenter);

        // UI: Gợi ý nút bấm (Hiện chữ [E] trên đầu NPC)
        if (distance < INTERACT_DISTANCE && !npcList[i].isTalking) {
            Vector2 textPos = { npcList[i].position.x - 10, npcList[i].position.y - 40 };
            DrawTextEx(globalFont, "[E] Nói chuyện", textPos, 24, 1, YELLOW);
        }

        // UI: Hộp thoại
        if (npcList[i].isTalking) {
           UI_DrawDialog(npcList[i].name, npcList[i].currentText);
        }
    }

    // 2. UI Cửa ra vào
    if (map->currentMapID == MAP_THU_VIEN) {
        if (Vector2Distance(playerHitCenter, exitToBlack) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Den", (Vector2){exitToBlack.x - 40, exitToBlack.y - 40}, 24, 1, GREEN);
        }
        if (Vector2Distance(playerHitCenter, exitToWhite) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Trang", (Vector2){exitToWhite.x - 40, exitToWhite.y - 40}, 24, 1, GREEN);
        }   
    }
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
        DrawTextEx(globalFont, "[E] Ve Thu Vien", (Vector2){10, 10}, 24, 1, GREEN);
    }
}
//debug only
void Interact_DrawDebugExits(GameMap *map) {
    if (map->currentMapID == MAP_THU_VIEN) {
        // Vẽ cửa sang Đen
        DrawCircleV(exitToBlack, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToBlack.x, (int)exitToBlack.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > DEN", (int)exitToBlack.x - 20, (int)exitToBlack.y - 10, 10, WHITE);
        DrawCircle((int)exitToBlack.x, (int)exitToBlack.y, 3.0f, RED);
        // Vẽ cửa sang Trắng
        DrawCircleV(exitToWhite, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToWhite.x, (int)exitToWhite.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > TRANG", (int)exitToWhite.x - 20, (int)exitToWhite.y - 10, 10, WHITE);
        DrawCircle((int)exitToWhite.x, (int)exitToWhite.y, 3.0f, RED);
    }
<<<<<<< Updated upstream
} 
=======
} 
>>>>>>> Stashed changes
