#include "npc.h"
#include <string.h> // Để dùng hàm strcpy

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name) {
    npc->mapID = mapID;
    npc->position = pos;
    strcpy(npc->name, name); // Copy tên vào
    strcpy(npc->dialog, "Xin chao! Toi la NPC."); // Thoại mặc định

    npc->texture = LoadTexture(texturePath);

    npc->frameCount = 4;        
    npc->currentFrame = 0;
    npc->frameTimer = 0.0f;
    npc->frameSpeed = 0.2f;     
    npc->isTalking = false;
}

void UpdateNpc(Npc *npc) {
    // Animation đơn giản: Nhún nhảy tại chỗ
    npc->frameTimer += GetFrameTime();
    if (npc->frameTimer >= npc->frameSpeed) {
        npc->frameTimer = 0.0f;
        npc->currentFrame++;
        if (npc->currentFrame >= npc->frameCount) {
            npc->currentFrame = 0;
        }
    }
}

void DrawNpc(Npc *npc) {
    float frameWidth = (float)npc->texture.width / npc->frameCount;
    
    Rectangle source = {
        npc->currentFrame * frameWidth, 0.0f, frameWidth, (float)npc->texture.height
    };
    Rectangle dest = {
        npc->position.x, npc->position.y, frameWidth, (float)npc->texture.height
    };

    DrawTexturePro(npc->texture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    // (Tùy chọn) Vẽ tên NPC trên đầu
    DrawText(npc->name, (int)npc->position.x, (int)npc->position.y - 20, 10, DARKGRAY);
}

void UnloadNpc(Npc *npc) {
    UnloadTexture(npc->texture);
}