// FILE: src/interact.c
#include "interact.h"
#include "raymath.h" // Để tính khoảng cách (Vector2Distance)
#include "ui_style.h" // [MỚI] Để dùng font Tiếng Việt

void Interact_Update(Player *player, Npc *npcList, int npcCount) {
    for (int i = 0; i < npcCount; i++) {
        // Chỉ xét NPC đang ở cùng Map với người chơi
        if (npcList[i].mapID != player->pClass) { // Lưu ý: Chỗ này logic mapID của player đang dùng tạm, sau này cần chuẩn hơn
             // Tạm thời bỏ qua check mapID của player vì struct Player chưa có biến mapID rõ ràng
             // Logic đúng: if (npcList[i].mapID != currentMapID) ...
        }

        // Tính khoảng cách
        float distance = Vector2Distance(player->position, npcList[i].position);

        if (distance < INTERACT_DISTANCE) {
            // Nếu bấm E thì bật/tắt hội thoại
            if (IsKeyPressed(KEY_E)) {
                npcList[i].isTalking = !npcList[i].isTalking;
            }
        } else {
            // Đi xa tự động tắt hội thoại
            npcList[i].isTalking = false;
        }
    }
}

void Interact_DrawUI(Player *player, Npc *npcList, int npcCount) {
    for (int i = 0; i < npcCount; i++) {
        float distance = Vector2Distance(player->position, npcList[i].position);

        // 1. Vẽ gợi ý nút [E] khi đứng gần
        if (distance < INTERACT_DISTANCE && !npcList[i].isTalking) {
            Vector2 textPos = { npcList[i].position.x - 10, npcList[i].position.y - 40 };
            DrawTextEx(globalFont, "[E] to talk", textPos, 24, 1, YELLOW);
        }

        // 2. Vẽ Hộp Thoại khi đang nói chuyện
        if (npcList[i].isTalking) {
            // Vẽ khung hộp thoại (Màu đen bán trong suốt)
            Rectangle boxRec = { 100, 300, 600, 120 };
            DrawRectangleRec(boxRec, Fade(BLACK, 0.8f));
            DrawRectangleLinesEx(boxRec, 2, WHITE);

            // Vẽ Tên NPC (Màu Vàng)
            DrawTextEx(globalFont, npcList[i].name, (Vector2){ boxRec.x + 20, boxRec.y + 10 }, 28, 1, YELLOW);

            // Vẽ Nội dung hội thoại Tiếng Việt (Màu Trắng)
            DrawTextEx(globalFont, npcList[i].dialog, (Vector2){ boxRec.x + 20, boxRec.y + 45 }, 24, 1, WHITE);
            
            // Vẽ hướng dẫn đóng
            DrawTextEx(globalFont, "Bấm [E] để đóng", (Vector2){ boxRec.x + 450, boxRec.y + 90 }, 18, 1, LIGHTGRAY);
        }
    }
}