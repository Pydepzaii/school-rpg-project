#ifndef NPC_H
#define NPC_H

#include "raylib.h"

typedef struct {
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
    //debug only
    float hitWidth;      // Chiều rộng hitbox
    float hitHeight;     // Chiều cao hitbox
    float paddingBottom; // Khoảng cách từ chân ảnh lên đến hitbox
} Npc;

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name, int id);
void UpdateNpc(Npc *npc);            
void DrawNpc(Npc *npc);              
void UnloadNpc(Npc *npc);            

#endif