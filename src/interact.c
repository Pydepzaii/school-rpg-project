// FILE: src/interact.c
#include "interact.h"
#include "raymath.h" // Thư viện toán học để tính khoảng cách (Vector2Distance)
#include <stddef.h>

// Hàm nội bộ: Tìm NPC gần nhất trong phạm vi tương tác
// Trả về: Con trỏ tới NPC gần nhất, hoặc NULL nếu không có ai
static Npc* GetClosestNpc(Player *player, Npc *npcList, int npcCount) {
    Npc *closest = NULL;
    float minDistance = INTERACT_DISTANCE;

    for (int i = 0; i < npcCount; i++) {
        // Chỉ xét NPC đang ở cùng Map với Player (quan trọng!)
        // Lưu ý: Chúng ta cần đảm bảo logic mapID được xử lý đúng từ main truyền vào
        // Tạm thời tính toán khoảng cách thuần túy
        
        float dist = Vector2Distance(player->position, npcList[i].position);
        
        if (dist < minDistance) {
            minDistance = dist;
            closest = &npcList[i];
        }
    }
    return closest;
}

void Interact_Update(Player *player, Npc *npcList, int npcCount) {
    // 1. Tìm xem có NPC nào đứng gần không
    Npc *targetNpc = GetClosestNpc(player, npcList, npcCount);

    // 2. Nếu có NPC gần
    if (targetNpc != NULL) {
        // Kiểm tra phím E
        if (IsKeyPressed(KEY_E)) {
            // Đảo ngược trạng thái nói chuyện (Bật -> Tắt, Tắt -> Bật)
            targetNpc->isTalking = !targetNpc->isTalking;
        }
    } 
    
    // 3. Logic phụ: Nếu đi ra xa quá thì tự động tắt hội thoại
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].isTalking) {
            float dist = Vector2Distance(player->position, npcList[i].position);
            if (dist > INTERACT_DISTANCE * 1.5f) { // Cho phép đi xa hơn một chút rồi mới tắt
                npcList[i].isTalking = false;
            }
        }
    }
}

void Interact_DrawUI(Player *player, Npc *npcList, int npcCount) {
    // Duyệt qua tất cả NPC để vẽ UI
    for (int i = 0; i < npcCount; i++) {
        // Lấy khoảng cách
        float dist = Vector2Distance(player->position, npcList[i].position);

        // CASE 1: Đứng gần nhưng chưa nói chuyện -> Hiện gợi ý "[E]"
        if (dist < INTERACT_DISTANCE && !npcList[i].isTalking) {
            Vector2 screenPos = npcList[i].position;
            // Vẽ dòng chữ nảy nảy một chút cho sinh động
            float bounce = sinf((float)GetTime() * 5.0f) * 3.0f; 
            
            DrawText("[E] Noi chuyen", (int)screenPos.x - 20, (int)screenPos.y - 30 + (int)bounce, 20, YELLOW);
        }

        // CASE 2: Đang trong trạng thái hội thoại -> Vẽ Hộp Thoại (Dialog Box)
        if (npcList[i].isTalking) {
            // Cấu hình hộp thoại
            int screenW = GetScreenWidth();
            int screenH = GetScreenHeight();
            float boxHeight = 120;
            float margin = 20;

            Rectangle boxRec = { 
                margin, 
                screenH - boxHeight - margin, 
                screenW - (margin * 2), 
                boxHeight 
            };

            // 1. Vẽ nền hộp thoại (Màu đen trong suốt)
            DrawRectangleRec(boxRec, Fade(BLACK, 0.8f));
            
            // 2. Vẽ viền trắng
            DrawRectangleLinesEx(boxRec, 3.0f, WHITE);

            // 3. Vẽ Tên NPC (Màu vàng)
            DrawText(npcList[i].name, (int)boxRec.x + 20, (int)boxRec.y + 15, 20, YELLOW);

            // 4. Vẽ Nội dung hội thoại (Màu trắng)
            DrawText(npcList[i].dialog, (int)boxRec.x + 20, (int)boxRec.y + 50, 20, WHITE);

            // 5. Hướng dẫn đóng
            DrawText("[E] Dong", (int)boxRec.x + boxRec.width - 100, (int)boxRec.y + boxRec.height - 25, 15, GRAY);
        }
    }
}