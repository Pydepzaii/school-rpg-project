// FILE: src/combat.h
#ifndef COMBAT_H
#define COMBAT_H

#include <stdbool.h> // Để dùng kiểu bool
#include "raylib.h"  // Thư viện gốc
#include "player.h"  // Để nhận diện struct Player
#include "npc.h"     // Để nhận diện struct Npc

// --- ĐỊNH NGHĨA CÁC TRẠNG THÁI TRẬN ĐẤU ---
// (combat.c sẽ dùng cái này để chuyển đổi lượt chơi)
typedef enum {
    COMBAT_STATE_START,         // 0: Hiệu ứng bắt đầu (Intro)
    COMBAT_STATE_PLAYER_TURN,   // 1: Lượt người chơi (Hiện menu Q/R)
    COMBAT_STATE_ACTION,        // 2: Xử lý hoạt ảnh tấn công
    COMBAT_STATE_ENEMY_TURN,    // 3: Lượt quái (Quái suy nghĩ)
    COMBAT_STATE_VICTORY,       // 4: Chiến thắng
    COMBAT_STATE_DEFEAT         // 5: Thua cuộc
} CombatState;

// --- KHAI BÁO CÁC HÀM (PROTOTYPES) ---

// 1. Khởi tạo & Hủy (Load/Unload ảnh nền)
void Combat_Init();
void Combat_Shutdown();

// 2. Bắt đầu trận đấu
// Cần truyền vào: Ai đánh (Player) và Đánh ai (Npc)
void Combat_Start(Player *player, Npc *enemy);

// 3. Vòng lặp Logic & Vẽ
void Combat_Update();
void Combat_Draw();

// 4. Kiểm tra trạng thái
// main.c sẽ dùng hàm này để biết nên vẽ Map hay vẽ Combat
bool Combat_IsActive();

#endif