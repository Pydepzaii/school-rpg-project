#ifndef NPC_H
#define NPC_H

#include "raylib.h"

typedef struct {
    int id;               // Định danh NPC
    int mapID;            // NPC này sống ở map nào?
    char name[30];        // Tên NPC (VD: "Co Dau Bep")
    
    Vector2 position;     
    Texture2D texture;    
    
    // --- ANIMATION ---
    int frameCount;       
    int currentFrame;     
    float frameTimer;     
    float frameSpeed;     
    
    // --- HỘI THOẠI (Cơ bản) ---
    char dialog[100];     // Câu thoại mặc định
    bool isTalking;       // Đang nói chuyện hay không

} Npc;

// Khởi tạo một NPC cụ thể vào danh sách
void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name);
void UpdateNpc(Npc *npc);            
void DrawNpc(Npc *npc);              
void UnloadNpc(Npc *npc);            

#endif