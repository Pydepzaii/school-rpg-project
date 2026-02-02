#include "npc.h"
#include "settings.h"
#include <string.h> 

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name, int id) {
    npc->mapID = mapID;
    npc->position = pos;
    strcpy(npc->name, name); 

    npc->texture = LoadTexture(texturePath);

    // [ANIMATION CONFIG]
    npc->frameCount = 4;        // Số lượng frame ngang trong ảnh (Sprite Sheet)
    npc->currentFrame = 0;
    npc->frameTimer = 0.0f;
    npc->frameSpeed = 0.2f;     // Tốc độ chuyển frame (giây) -> Càng nhỏ càng nhanh
    npc->isTalking = false;
    //dữ liệu hitbox npc
    npc->hitWidth = 24.0f;      // Chiều rộng
    npc->hitHeight = 10.0f;     // Chiều cao
    npc->paddingBottom = 15.0f; // Khoảng cách từ đáy ảnh lên hitbox
}

void UpdateNpc(Npc *npc) {
    // [ANIMATION LOOP] 
    // Tự động chuyển frame theo thời gian thực (Delta Time)
    npc->frameTimer += GetFrameTime();
    if (npc->frameTimer >= npc->frameSpeed) {
        npc->frameTimer = 0.0f;
        npc->currentFrame++;
        
        // Loop lại frame đầu nếu hết ảnh
        if (npc->currentFrame >= npc->frameCount) {
            npc->currentFrame = 0;
        }
    }
}

void DrawNpc(Npc *npc) {
    // 1. Tính toán Source Rectangle (Cắt ảnh từ Sprite Sheet)
    // [GIẢI THÍCH]: Texture chứa nhiều hình nhân vật, ta chỉ cắt 1 hình tương ứng với frame hiện tại.
    float frameWidth = (float)npc->texture.width / npc->frameCount;
    
    Rectangle source = {
        npc->currentFrame * frameWidth, 0.0f, frameWidth, (float)npc->texture.height
    };
    
    // 2. Tính toán Destination (Vị trí vẽ trên màn hình)
    Rectangle dest = {
        npc->position.x, npc->position.y, frameWidth, (float)npc->texture.height
    };

    // 3. Render
    DrawTexturePro(npc->texture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    // Debug name tag - Hiện tên NPC
    DrawText(npc->name, (int)npc->position.x, (int)npc->position.y - 20, 10, DARKGRAY);
}

void UnloadNpc(Npc *npc) {
    UnloadTexture(npc->texture);
}
//tải npc vào các map cụ thể
void Npc_LoadForMap(int mapID, Npc *npcList, int *npcCount) {
    // 1. Reset số lượng NPC
    *npcCount = 0;

    // 2. Tùy theo Map mà sinh ra NPC tương ứng
    switch (mapID) {
        case MAP_THU_VIEN:
            // NPC 1: Cô Đầu Bếp
            InitNpc(&npcList[*npcCount], MAP_THU_VIEN, "resources/npc/map_thu_vien/codaubep.png", (Vector2){206, 250}, "Cô đầu bếp", 0);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++; // Tăng biến đếm

            // Nếu muốn thêm NPC nữa thì copy đoạn trên paste xuống đây
            break;

        case MAP_NHA_AN:
            // Code load NPC nhà ăn...
            break;

        // Các map khác...
        default:
            break;
    }
}