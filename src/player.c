// FILE: src/player.c
#include "player.h"
#include "settings.h"

// [CONFIG] SPRITE SHEET MAPPING
// Quy định thứ tự hàng (Row) trong file ảnh nhân vật
// [GIẢI THÍCH]: Sprite sheet là 1 ảnh lớn chứa nhiều ảnh nhỏ.
// Các dòng này định nghĩa: Hàng 0 là đi trái, Hàng 1 là đi lên...
#define ANIM_ROW_LEFT   0   
#define ANIM_ROW_UP     1   
#define ANIM_ROW_DOWN   2   
#define ANIM_ROW_RIGHT  3   

#define MAX_FRAME_COLS 6    

void InitPlayer(Player *player, PlayerClass chosenClass) {
    player->pClass = chosenClass;
    player->texture = LoadTexture("resources/main_hocba.png"); 
    player->position = (Vector2){100, 100}; 

    // [RPG STATS] Cấu hình chỉ số theo class
    // [GIẢI THÍCH]: Set máu, mana, tốc độ chạy tùy theo nghề nghiệp (Warrior/Student).
    switch (chosenClass) {
        case CLASS_WARRIOR: player->stats = (PlayerStats){150, 150, 20, 20, 0, 3.0f}; break;
        case CLASS_STUDENT:
        default:            player->stats = (PlayerStats){100, 100, 50, 10, 10, 4.0f}; break;
    }

    // Tính kích thước 1 frame
    // [GIẢI THÍCH]: Chia chiều rộng ảnh tổng cho số cột để ra chiều rộng 1 frame đơn lẻ.
    player->spriteWidth = player->texture.width / MAX_FRAME_COLS; 
    player->spriteHeight = player->texture.height / 4;          

    // Init Animation state
    player->currentFrame = 0;
    player->framesCounter = 0;
    player->framesSpeed = 8; // Tốc độ animation (frames per second game loop)

    player->frameRec = (Rectangle){
        0.0f, 
        (float)(ANIM_ROW_DOWN * player->spriteHeight), 
        (float)player->spriteWidth - 0.5f, 
        (float)player->spriteHeight - 0.5f
    };
}

// [PHYSICS] PREDICTIVE COLLISION
// Kiểm tra xem hình chữ nhật (hitbox) CÓ SẼ va chạm không nếu di chuyển
// [GIẢI THÍCH]: Logic này rất quan trọng. Nó "dự đoán" va chạm ở tương lai.
// Nếu hàm này trả về true, nhân vật sẽ bị chặn lại, không cho đi tiếp.
bool CheckCollisionFuture(Rectangle hitbox, GameMap *map, Npc *npcList, int npcCount) {
    // 1. Check Tường (Môi trường)
    for (int i = 0; i < map->wallCount; i++) {
        if (CheckCollisionRecs(hitbox, map->walls[i])) return true; 
    }

    // 2. Check NPC (Dynamic Entities)
    for (int i = 0; i < npcCount; i++) {
        // [GIẢI THÍCH]: Chỉ check va chạm với NPC ở cùng Map.
        if (npcList[i].mapID == map->currentMapID) { 
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            float npcH = (float)npcList[i].texture.height;

            // [CRITICAL] NPC HITBOX CALCULATION
            // Cần tạo hitbox nhỏ nằm dưới CHÂN NPC, không phải toàn thân.
            // Nếu hitbox quá lớn, player sẽ bị kẹt khi đi ngang qua đầu NPC.
            float boxWidth = 24.0f;   
            float boxHeight = 10.0f;  // Chiều cao thấp (dẹt) để mô phỏng không gian 3D
            float paddingBottom = 17.0f; 
            
            float offsetX = (npcW - boxWidth) / 2.0f; 

            Rectangle npcFeetRect = { 
                npcList[i].position.x + offsetX,            
                npcList[i].position.y + npcH - boxWidth - paddingBottom, 
                boxWidth, 
                boxHeight                            
            };

            if (CheckCollisionRecs(hitbox, npcFeetRect)) return true;
        }
    }
    return false; 
}

void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount) {
    bool isMoving = false;
    Vector2 nextPos = player->position; // Biến tạm để tính toán vị trí tương lai
    
    int targetRow = (int)(player->frameRec.y / player->spriteHeight); 

    // --- INPUT HANDLING ---
    // Di chuyển độc lập trục X/Y để cho phép trượt tường (Wall Sliding)
    // [GIẢI THÍCH]: Check 4 phím điều hướng để tính toán vị trí tiếp theo (nextPos).
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

    // Update vùng cắt ảnh (Row) dựa trên hướng di chuyển
    player->frameRec.y = (float)(targetRow * player->spriteHeight);
    player->frameRec.width = (float)player->spriteWidth - 0.5f;

    // --- COLLISION RESOLUTION ---
    // Nguyên lý: Tách biệt check trục X và trục Y để tránh kẹt góc.
    // Hitbox của Player cũng nằm ở CHÂN (Feet) giống NPC.
    float pW = (float)player->spriteWidth;
    float pH = (float)player->spriteHeight;
    float pFeetH = 20.0f; // Độ cao hitbox chân

    // 1. Resolve X Axis (Xử lý va chạm ngang)
    Rectangle boxX = { 
        nextPos.x + 15,                 // Thu hẹp 15px mỗi bên
        player->position.y + pH - pFeetH, 
        pW - 30,                        
        pFeetH                          
    };
    
    bool colX = CheckCollisionFuture(boxX, map, npcList, npcCount);
    // Check biên giới hạn màn hình
    if (nextPos.x < -10 || nextPos.x > SCREEN_WIDTH - pW + 10) colX = true;
    
    if (!colX) player->position.x = nextPos.x; // Nếu không va chạm thì cho phép đi

    // 2. Resolve Y Axis (Xử lý va chạm dọc)
    Rectangle boxY = { 
        player->position.x + 15, 
        nextPos.y + pH - pFeetH,      
        pW - 30, 
        pFeetH 
    };

    bool colY = CheckCollisionFuture(boxY, map, npcList, npcCount);
    if (nextPos.y < -10 || nextPos.y > SCREEN_HEIGHT - pH + 10) colY = true;
    
    if (!colY) player->position.y = nextPos.y;

    // --- ANIMATION UPDATE ---
    // [GIẢI THÍCH]: Logic chuyển đổi frame ảnh để tạo hiệu ứng bước đi.
    if (isMoving) {
        player->framesCounter++;
        if (player->framesCounter >= player->framesSpeed) {
            player->framesCounter = 0;
            player->currentFrame++;
            if (player->currentFrame >= MAX_FRAME_COLS) player->currentFrame = 0;

            // Xử lý Flip sprite (nếu cần) hoặc mapping frame index
            int displayFrame = player->currentFrame;
            if (targetRow == ANIM_ROW_LEFT) {
                // Ví dụ: Đảo ngược frame nếu animation đi trái bị ngược
                displayFrame = (MAX_FRAME_COLS - 1) - player->currentFrame;
            }
            player->frameRec.x = (float)(displayFrame * player->spriteWidth);
        }
    } else {
        // Idle State: Reset về frame đầu tiên khi đứng yên
        player->currentFrame = 0;
        int displayFrame = 0;
        if (targetRow == ANIM_ROW_LEFT) displayFrame = (MAX_FRAME_COLS - 1); 
        player->frameRec.x = (float)(displayFrame * player->spriteWidth);
    }
}

void DrawPlayer(Player *player) {
    DrawTextureRec(player->texture, player->frameRec, player->position, WHITE);
    
    // Vẽ thanh máu (Health Bar) đơn giản
    int barWidth = 40;
    int barHeight = 5;
    int barX = (int)(player->position.x + (player->spriteWidth / 2.0f) - (barWidth / 2.0f));
    int barY = (int)player->position.y - 10;

    DrawRectangle(barX, barY, barWidth, barHeight, Fade(RED, 0.8f)); // Background
    float hpPercent = (float)player->stats.hp / player->stats.maxHp;
    DrawRectangle(barX, barY, (int)(barWidth * hpPercent), barHeight, LIME); // Foreground
}

void UnloadPlayer(Player *player) {
    UnloadTexture(player->texture);
}