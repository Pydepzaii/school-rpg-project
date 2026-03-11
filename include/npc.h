#ifndef NPC_H
#define NPC_H
#include "player.h"
#include "raylib.h"

// --- DANH SÁCH ID NPC & BOSS THEO KỊCH BẢN ---
#define NPC_CO_THU_KY        1  // Boss 1 (Alpha)
#define NPC_THAY_TUAN_VM     2  // Boss 2 (Alpha)
#define NPC_THAY_CHINH       3  // Boss phụ (Nhà Võ)
#define NPC_THAY_HUNG        4  // Boss chính (Nhà Võ)
#define NPC_CHU_PHU_BEP      5  // Boss phụ (Căng Tin)
#define NPC_CO_BEP_TRUONG    6  // Boss chính (Căng Tin)
#define NPC_LAO_CONG_MAP4    7  // Boss phụ (Thư Viện)
#define NPC_CO_THU_THU       8  // Boss chính (Thư Viện)
#define NPC_LAO_CONG_MAP5    9  // Boss phụ (Beta)
#define NPC_THAY_CHU_NHIEM   10 // Boss chính (Beta)
#define NPC_TRO_LY_HT        11 // Boss phụ (Phòng Lab)
#define NPC_THAY_HIEU_TRUONG 12 // Boss cuối (Phòng Lab)
#define NPC_BA_GIA_CO_DON    13 // Easter Egg (Nhiệm vụ phụ)

typedef struct Npc {
    int id;               
    int mapID;            // Quan trọng: NPC thuộc về map nào?
    char name[30];        // Tên hiển thị
    
    Vector2 position;     // Vị trí đứng
    Texture2D texture;    // Ảnh Sprite sheet
    
    // --- ANIMATION (Hoạt hình) ---
    int frameCount;       // Tổng số khung hình trong ảnh (VD: 4 hình)
    int currentFrame;     // Khung hình đang vẽ hiện tại
    float frameTimer;     // Bộ đếm thời gian để chuyển hình
    float frameSpeed;     // Tốc độ nhấp nháy (càng nhỏ càng nhanh)
    
    // --- HỘI THOẠI ---
    char dialogKey[32];   // Ví dụ: "DEFAULT", "QUEST_1", "NO_MONEY"
    char currentText[256]; // Chứa nội dung lấy từ file txt ra
    bool isTalking;   
    int currentDialogLine;    
    //debug only
    float hitWidth;      // Chiều rộng hitbox
    float hitHeight;     // Chiều cao hitbox
    float paddingBottom; // Khoảng cách từ chân ảnh lên đến hitbox

    // [FIX] Thêm stats vào đây (Dùng chung kiểu với Player)
    PlayerStats stats;
    bool isDead;
} Npc;

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name, int id);
void UpdateNpc(Npc *npc);            
void DrawNpc(Npc *npc);              
void UnloadNpc(Npc *npc);            
void Npc_LoadForMap(int mapID, Npc *npcList, int *npcCount);
#endif