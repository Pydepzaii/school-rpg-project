#include "player.h"
#include "settings.h"

// --- CẤU HÌNH HÀNG ANIMATION (Index từ 0-3) ---
#define ANIM_ROW_LEFT   0   // Hàng đi sang Trái (Cần đọc ngược)
#define ANIM_ROW_UP     1   
#define ANIM_ROW_DOWN   2   
#define ANIM_ROW_RIGHT  3   

#define MAX_FRAME_COLS 6 

void InitPlayer(Player *player, PlayerClass chosenClass) {
    player->pClass = chosenClass;
    player->texture = LoadTexture("resources/main_hocba.png"); 
    player->position = (Vector2){100, 100}; 

    switch (chosenClass) {
        case CLASS_WARRIOR: player->stats = (PlayerStats){150, 150, 20, 20, 0, 3.0f}; break;
        case CLASS_MAGE:    player->stats = (PlayerStats){80, 80, 200, 5, 30, 4.0f}; break;
        case CLASS_ARCHER:  player->stats = (PlayerStats){100, 100, 50, 15, 5, 6.0f}; break;
        case CLASS_STUDENT:
        default:            player->stats = (PlayerStats){100, 100, 50, 10, 10, 4.0f}; break;
    }

    player->spriteWidth = player->texture.width / MAX_FRAME_COLS; // = 25
    player->spriteHeight = player->texture.height / 4;          // = 31

    player->currentFrame = 0;
    player->framesCounter = 0;
    player->framesSpeed = 8; 

    player->frameRec = (Rectangle){
        0.0f, 
        (float)(ANIM_ROW_DOWN * player->spriteHeight), 
        (float)player->spriteWidth - 0.5f, 
        (float)player->spriteHeight - 0.5f
    };
}

bool CheckCollisionFuture(Rectangle hitbox, GameMap *map, Npc *npcList, int npcCount) {
    for (int i = 0; i < map->wallCount; i++) {
        if (CheckCollisionRecs(hitbox, map->walls[i])) return true;
    }
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID) {
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            Rectangle npcRect = { npcList[i].position.x + 10, npcList[i].position.y + 10, npcW - 20, 40 };
            if (CheckCollisionRecs(hitbox, npcRect)) return true;
        }
    }
    return false;
}

void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount) {
    bool isMoving = false;
    Vector2 nextPos = player->position;
    
    // Lấy hàng hiện tại
    int targetRow = (int)(player->frameRec.y / player->spriteHeight);

    // --- XỬ LÝ DI CHUYỂN & CHỌN HÀNG ---
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

    player->frameRec.y = (float)(targetRow * player->spriteHeight);
    player->frameRec.width = (float)player->spriteWidth - 0.5f;

    // --- XỬ LÝ VA CHẠM ---
    Rectangle boxX = { nextPos.x + 10, player->position.y + 20, (float)player->spriteWidth - 20, (float)player->spriteHeight - 20 };
    bool colX = CheckCollisionFuture(boxX, map, npcList, npcCount);
    if (nextPos.x < 0 || nextPos.x > SCREEN_WIDTH - player->spriteWidth) colX = true;
    if (!colX) player->position.x = nextPos.x;

    Rectangle boxY = { player->position.x + 10, nextPos.y + 20, (float)player->spriteWidth - 20, (float)player->spriteHeight - 20 };
    bool colY = CheckCollisionFuture(boxY, map, npcList, npcCount);
    if (nextPos.y < 0 || nextPos.y > SCREEN_HEIGHT - player->spriteHeight) colY = true;
    if (!colY) player->position.y = nextPos.y;

    // --- XỬ LÝ ANIMATION (QUAN TRỌNG: ĐẢO NGƯỢC HÀNG TRÁI) ---
    if (isMoving) {
        player->framesCounter++;
        if (player->framesCounter >= player->framesSpeed) {
            player->framesCounter = 0;
            player->currentFrame++;
            if (player->currentFrame >= MAX_FRAME_COLS) player->currentFrame = 0;

            // --- LOGIC ĐẢO NGƯỢC Ở ĐÂY ---
            int displayFrame = player->currentFrame;

            // Nếu đang là hàng ĐI SANG TRÁI, ta tính frame ngược lại (5 -> 0)
            // Thay vì vẽ frame 0, ta vẽ frame 5. Thay vì 1, vẽ 4...
            if (targetRow == ANIM_ROW_LEFT) {
                displayFrame = (MAX_FRAME_COLS - 1) - player->currentFrame;
            }

            player->frameRec.x = (float)(displayFrame * player->spriteWidth);
        }
    } else {
        // Khi đứng yên
        player->currentFrame = 0;
        int displayFrame = 0;
        
        // Đứng yên của hàng Trái cũng phải lấy frame cuối cùng (hoặc đầu tiên tùy ảnh gốc)
        // Nếu ảnh gốc frame 0 là đứng yên thì giữ nguyên logic này
        // Nếu ảnh gốc frame 5 là đứng yên thì uncomment dòng dưới:
        if (targetRow == ANIM_ROW_LEFT) {
             displayFrame = (MAX_FRAME_COLS - 1); 
        }

        player->frameRec.x = (float)(displayFrame * player->spriteWidth);
    }
}

void DrawPlayer(Player *player) {
    DrawTextureRec(player->texture, player->frameRec, player->position, WHITE);
    
    int barWidth = 40;
    int barHeight = 5;
    int barX = (int)(player->position.x + (player->spriteWidth / 2.0f) - (barWidth / 2.0f));
    int barY = (int)player->position.y - 10;

    DrawRectangle(barX, barY, barWidth, barHeight, Fade(RED, 0.8f));
    float hpPercent = (float)player->stats.hp / player->stats.maxHp;
    DrawRectangle(barX, barY, (int)(barWidth * hpPercent), barHeight, LIME);
}

void UnloadPlayer(Player *player) {
    UnloadTexture(player->texture);
}