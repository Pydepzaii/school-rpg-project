#include "npc.h"
#include <string.h> // Thư viện xử lý chuỗi (strcpy)

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name) {
    npc->mapID = mapID;
    npc->position = pos;
    strcpy(npc->name, name); // Copy tên vào bộ nhớ
    strcpy(npc->dialog, "Xin chao! Toi la NPC."); // Câu thoại mặc định

    npc->texture = LoadTexture(texturePath);

    // Cấu hình Animation
    npc->frameCount = 4;        // Giả sử ảnh NPC có 4 hình ngang
    npc->currentFrame = 0;
    npc->frameTimer = 0.0f;
    npc->frameSpeed = 0.2f;     // 0.2 giây đổi hình 1 lần
    npc->isTalking = false;
}

void UpdateNpc(Npc *npc) {
    // Logic Animation: Thay đổi khung hình theo thời gian
    npc->frameTimer += GetFrameTime();
    if (npc->frameTimer >= npc->frameSpeed) {
        npc->frameTimer = 0.0f;
        npc->currentFrame++;
        // Nếu chạy hết hình thì quay lại hình đầu tiên
        if (npc->currentFrame >= npc->frameCount) {
            npc->currentFrame = 0;
        }
    }
}

void DrawNpc(Npc *npc) {
    // Tính toán cắt ảnh từ Sprite Sheet
    float frameWidth = (float)npc->texture.width / npc->frameCount;
    
    Rectangle source = {
        npc->currentFrame * frameWidth, 0.0f, frameWidth, (float)npc->texture.height
    };
    Rectangle dest = {
        npc->position.x, npc->position.y, frameWidth, (float)npc->texture.height
    };

    // Vẽ NPC lên màn hình
    DrawTexturePro(npc->texture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    // Vẽ tên trên đầu NPC
    DrawText(npc->name, (int)npc->position.x, (int)npc->position.y - 20, 10, DARKGRAY);
}

void UnloadNpc(Npc *npc) {
    UnloadTexture(npc->texture);
}