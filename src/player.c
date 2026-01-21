#include "player.h"
#include "settings.h"

// --- QUY ĐỊNH HÀNG TRONG ẢNH SPRITE ---
// Cần mở file ảnh lên xem hàng nào là hướng nào
#define ANIM_ROW_LEFT   0   // Hàng 0: Đi sang Trái
#define ANIM_ROW_UP     1   // Hàng 1: Đi lên
#define ANIM_ROW_DOWN   2   // Hàng 2: Đi xuống
#define ANIM_ROW_RIGHT  3   // Hàng 3: Đi sang Phải

#define MAX_FRAME_COLS 6    // Số cột (số hành động) trong 1 hàng

void InitPlayer(Player *player, PlayerClass chosenClass) {
    player->pClass = chosenClass;
    player->texture = LoadTexture("resources/main_hocba.png"); 
    player->position = (Vector2){100, 100}; // Vị trí xuất hiện ban đầu

    // Cài đặt chỉ số theo Class
    switch (chosenClass) {
        case CLASS_WARRIOR: player->stats = (PlayerStats){150, 150, 20, 20, 0, 3.0f}; break;
        // ... các class khác
        case CLASS_STUDENT:
        default:            player->stats = (PlayerStats){100, 100, 50, 10, 10, 4.0f}; break;
    }

    // Tính toán kích thước 1 frame dựa trên ảnh gốc chia cho số cột/hàng
    player->spriteWidth = player->texture.width / MAX_FRAME_COLS; 
    player->spriteHeight = player->texture.height / 4;          

    // Thiết lập Animation ban đầu
    player->currentFrame = 0;
    player->framesCounter = 0;
    player->framesSpeed = 8; // Số frame game trôi qua thì mới đổi hình nhân vật 1 lần

    // Khởi tạo khung cắt ảnh
    player->frameRec = (Rectangle){
        0.0f, 
        (float)(ANIM_ROW_DOWN * player->spriteHeight), 
        (float)player->spriteWidth - 0.5f, 
        (float)player->spriteHeight - 0.5f
    };
}

// HÀM KIỂM TRA VA CHẠM TƯƠNG LAI
// "Nếu tôi đi đến đó, tôi có đâm vào tường hay NPC không?"
bool CheckCollisionFuture(Rectangle hitbox, GameMap *map, Npc *npcList, int npcCount) {
    // 1. Kiểm tra va chạm với Tường
    for (int i = 0; i < map->wallCount; i++) {
        if (CheckCollisionRecs(hitbox, map->walls[i])) return true; // Có va chạm
    }
    // 2. Kiểm tra va chạm với NPC
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID) { // Chỉ tính NPC cùng map
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            Rectangle npcRect = { npcList[i].position.x + 10, npcList[i].position.y + 10, npcW - 20, 40 };
            if (CheckCollisionRecs(hitbox, npcRect)) return true;
        }
    }
    return false; // Không đâm vào đâu cả
}

void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount) {
    bool isMoving = false;
    Vector2 nextPos = player->position; // Vị trí dự kiến sẽ đi tới
    
    int targetRow = (int)(player->frameRec.y / player->spriteHeight); // Hàng hiện tại

    // --- XỬ LÝ PHÍM BẤM ---
    if (IsKeyDown(KEY_LEFT)) { 
        nextPos.x -= player->stats.moveSpeed; 
        targetRow = ANIM_ROW_LEFT; 
        isMoving = true; 
    }
    else if (IsKeyDown(KEY_RIGHT)) { 
        nextPos.x += player->stats.moveSpeed; 
        targetRow = ANIM_ROW_RIGHT; 
        isMoving = true; 
    }
    else if (IsKeyDown(KEY_UP)) { 
        nextPos.y -= player->stats.moveSpeed; 
        targetRow = ANIM_ROW_UP;   
        isMoving = true; 
    }
    else if (IsKeyDown(KEY_DOWN)) { 
        nextPos.y += player->stats.moveSpeed; 
        targetRow = ANIM_ROW_DOWN; 
        isMoving = true; 
    }

    // Cập nhật hàng animation (Y của khung cắt)
    player->frameRec.y = (float)(targetRow * player->spriteHeight);
    player->frameRec.width = (float)player->spriteWidth - 0.5f;

    // --- XỬ LÝ VA CHẠM (COLLISION PHYSICS) ---
    // Kiểm tra trục X
    Rectangle boxX = { nextPos.x + 10, player->position.y + 20, (float)player->spriteWidth - 20, (float)player->spriteHeight - 20 };
    bool colX = CheckCollisionFuture(boxX, map, npcList, npcCount);
    // Kiểm tra biên giới hạn màn hình
    if (nextPos.x < 0 || nextPos.x > SCREEN_WIDTH - player->spriteWidth) colX = true;
    
    if (!colX) player->position.x = nextPos.x; // Nếu không va chạm thì cho đi

    // Kiểm tra trục Y
    Rectangle boxY = { player->position.x + 10, nextPos.y + 20, (float)player->spriteWidth - 20, (float)player->spriteHeight - 20 };
    bool colY = CheckCollisionFuture(boxY, map, npcList, npcCount);
    if (nextPos.y < 0 || nextPos.y > SCREEN_HEIGHT - player->spriteHeight) colY = true;
    
    if (!colY) player->position.y = nextPos.y;

    // --- XỬ LÝ ANIMATION (Chuyển động hình ảnh) ---
    if (isMoving) {
        player->framesCounter++;
        if (player->framesCounter >= player->framesSpeed) {
            player->framesCounter = 0;
            player->currentFrame++;
            if (player->currentFrame >= MAX_FRAME_COLS) player->currentFrame = 0;

            int displayFrame = player->currentFrame;

            // XỬ LÝ ĐẶC BIỆT CHO HÀNG TRÁI (Lật ngược frame)
            // Vì ảnh gốc vẽ theo chiều phải, nên khi đi trái cần đọc ngược frame
            if (targetRow == ANIM_ROW_LEFT) {
                displayFrame = (MAX_FRAME_COLS - 1) - player->currentFrame;
            }

            player->frameRec.x = (float)(displayFrame * player->spriteWidth);
        }
    } else {
        // Khi đứng yên: Reset về frame đầu tiên
        player->currentFrame = 0;
        int displayFrame = 0;
        if (targetRow == ANIM_ROW_LEFT) displayFrame = (MAX_FRAME_COLS - 1); 
        player->frameRec.x = (float)(displayFrame * player->spriteWidth);
    }
}

void DrawPlayer(Player *player) {
    // Vẽ nhân vật
    DrawTextureRec(player->texture, player->frameRec, player->position, WHITE);
    
    // Vẽ thanh máu trên đầu
    int barWidth = 40;
    int barHeight = 5;
    int barX = (int)(player->position.x + (player->spriteWidth / 2.0f) - (barWidth / 2.0f));
    int barY = (int)player->position.y - 10;

    DrawRectangle(barX, barY, barWidth, barHeight, Fade(RED, 0.8f)); // Nền đỏ
    float hpPercent = (float)player->stats.hp / player->stats.maxHp;
    DrawRectangle(barX, barY, (int)(barWidth * hpPercent), barHeight, LIME); // Máu xanh
}

void UnloadPlayer(Player *player) {
    UnloadTexture(player->texture);
}