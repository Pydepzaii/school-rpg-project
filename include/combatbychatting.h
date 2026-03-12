// FILE: src/combatbychatting.h
#ifndef COMBAT_BY_CHATTING_H
#define COMBAT_BY_CHATTING_H

#include <stdbool.h>
#include "raylib.h"
#include "player.h"
#include "npc.h"

// Các trạng thái của mini-game (Giữ nguyên vì ông đã có đủ)
typedef enum {
    CBC_STATE_INTRO,    // Màn hình chào
    CBC_STATE_QUESTION, // Đang chọn đáp án (Hỗ trợ cả phím và chuột)
    CBC_STATE_RESULT,   // Hiện đúng/sai
    CBC_STATE_FINAL     // Tổng kết thắng/thua
} CBC_State;

// --- CÁC HÀM GIAO TIẾP ---

// Khởi tạo ngân hàng câu hỏi (Gọi 1 lần duy nhất trong main.c)
void CBC_Init();

// Dọn dẹp tài nguyên
void CBC_Shutdown();

// Bắt đầu Mini-game
// Lưu ý: Ta truyền Npc *enemyPtr vào để trong file .c 
// mình check "enemyPtr->id" rồi tự nạp số câu hỏi tương ứng.
void CBC_Start(Player *playerPtr, Npc *enemyPtr);

// Cập nhật logic mỗi khung hình (Check click chuột vào đáp án)
void CBC_Update();

// Vẽ giao diện (Khung hình, hiệu ứng Hover chuột)
void CBC_Draw();

// Kiểm tra trạng thái để chặn Player di chuyển khi đang thi cử
bool CBC_IsActive();

bool CBC_IsJustLost(void);

#endif