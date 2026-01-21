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
    char dialog[100];     // Câu thoại (chưa hỗ trợ tiếng Việt có dấu)
    bool isTalking;       

} Npc;

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name);
void UpdateNpc(Npc *npc);            
void DrawNpc(Npc *npc);              
void UnloadNpc(Npc *npc);            

#endif