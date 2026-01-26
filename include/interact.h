// FILE: src/interact.h
#ifndef INTERACT_H
#define INTERACT_H

#include "raylib.h"
#include "player.h"
#include "npc.h"

// --- CẤU HÌNH TƯƠNG TÁC ---
#define INTERACT_DISTANCE 60.0f // Khoảng cách tối đa để hiện nút [E] (Pixel)

// --- CÁC HÀM CHÍNH ---

/**
 * Xử lý logic tương tác:
 * 1. Kiểm tra khoảng cách giữa Player và các NPC.
 * 2. Bắt sự kiện phím E.
 * 3. Bật/Tắt trạng thái hội thoại (isTalking).
 */
void Interact_Update(Player *player, Npc *npcList, int npcCount);

/**
 * Vẽ giao diện tương tác (UI):
 * 1. Vẽ nhắc nhở "[E]" trên đầu NPC khi đứng gần.
 * 2. Vẽ hộp thoại (Dialog Box) khi đang nói chuyện.
 */
void Interact_DrawUI(Player *player, Npc *npcList, int npcCount);

#endif
