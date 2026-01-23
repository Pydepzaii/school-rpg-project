// FILE: src/player.c
#include "player.h"
#include "settings.h"

// --- QUY ĐỊNH HÀNG TRONG ẢNH SPRITE ---
#define ANIM_ROW_LEFT   0   
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
        case CLASS_STUDENT:
        default:            player->stats = (PlayerStats){100, 100, 50, 10, 10, 4.0f}; break;
    }

    player->spriteWidth = player->texture.width / MAX_FRAME_COLS; 
    player->spriteHeight = player->texture.height / 4;          

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

// HÀM KIỂM TRA VA CHẠM TƯƠNG LAI
bool CheckCollisionFuture(Rectangle hitbox, GameMap *map, Npc *npcList, int npcCount) {
    // 1. Kiểm tra va chạm với Tường
    for (int i = 0; i < map->wallCount; i++) {
        if (CheckCollisionRecs(hitbox, map->walls[i])) return true; 
    }

    // 2. Kiểm tra va chạm với NPC
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID) { 
            // Tính toán kích thước ảnh NPC
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            float npcH = (float)npcList[i].texture.height;

            // --- [FIX] CHỈNH HITBOX NPC NHỎ GỌN & CĂN GIỮA ---
            float boxWidth = 24.0f;   // Chiều ngang giữ nguyên
            float boxHeight = 10.0f;  // [QUAN TRỌNG] Chiều cao giảm xuống (ép dẹt)
            float paddingBottom = 17.0f; // Kéo hitbox lên cao 4px so với đáy ảnh
            
            // Công thức căn giữa: (Rộng ảnh - Rộng hộp) / 2
            float offsetX = (npcW - boxWidth) / 2.0f; 

            Rectangle npcFeetRect = { 
                npcList[i].position.x + offsetX,            
                npcList[i].position.y + npcH - boxWidth - paddingBottom, 
               boxWidth,  // <--- Dòng này trước đây là boxSize, phải sửa thành boxWidth
                boxHeight                            
            };

            if (CheckCollisionRecs(hitbox, npcFeetRect)) return true;
        }
    }
    return false; 
}

void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount) {
    bool isMoving = false;
    Vector2 nextPos = player->position; 
    
    int targetRow = (int)(player->frameRec.y / player->spriteHeight); 

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

    player->frameRec.y = (float)(targetRow * player->spriteHeight);
    player->frameRec.width = (float)player->spriteWidth - 0.5f;

    // --- [FIX] XỬ LÝ VA CHẠM (COLLISION PHYSICS) ---
    // Chỉ tạo hitbox ở dưới CHÂN nhân vật
    float pW = (float)player->spriteWidth;
    float pH = (float)player->spriteHeight;
    float pFeetH = 20.0f; // Chiều cao chân player

    // 1. Kiểm tra trục X
    Rectangle boxX = { 
        nextPos.x + 15,                 
        player->position.y + pH - pFeetH, 
        pW - 30,                        
        pFeetH                          
    };
    
    bool colX = CheckCollisionFuture(boxX, map, npcList, npcCount);
    if (nextPos.x < -10 || nextPos.x > SCREEN_WIDTH - pW + 10) colX = true;
    if (!colX) player->position.x = nextPos.x; 

    // 2. Kiểm tra trục Y
    Rectangle boxY = { 
        player->position.x + 15, 
        nextPos.y + pH - pFeetH,      
        pW - 30, 
        pFeetH 
    };

    bool colY = CheckCollisionFuture(boxY, map, npcList, npcCount);
    if (nextPos.y < -10 || nextPos.y > SCREEN_HEIGHT - pH + 10) colY = true;
    if (!colY) player->position.y = nextPos.y;

    // --- XỬ LÝ ANIMATION ---
    if (isMoving) {
        player->framesCounter++;
        if (player->framesCounter >= player->framesSpeed) {
            player->framesCounter = 0;
            player->currentFrame++;
            if (player->currentFrame >= MAX_FRAME_COLS) player->currentFrame = 0;

            int displayFrame = player->currentFrame;
            if (targetRow == ANIM_ROW_LEFT) {
                displayFrame = (MAX_FRAME_COLS - 1) - player->currentFrame;
            }
            player->frameRec.x = (float)(displayFrame * player->spriteWidth);
        }
    } else {
        player->currentFrame = 0;
        int displayFrame = 0;
        if (targetRow == ANIM_ROW_LEFT) displayFrame = (MAX_FRAME_COLS - 1); 
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