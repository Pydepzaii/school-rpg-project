#include "player.h"
#include "settings.h"
#include "npc.h" // [FIX 1] BẮT BUỘC PHẢI CÓ DÒNG NÀY ĐỂ HIỂU STRUCT NPC
#include <string.h>

// [CONFIG] KÍCH THƯỚC CẮT TỪ ẢNH GỐC (Sprite Sheet)
#define FRAME_WIDTH  80  
#define FRAME_HEIGHT 80  
//flame animation main
#define MAX_FRAME_WALK 8
#define MAX_FRAME_IDLE 14

// [CONFIG] ĐỊNH NGHĨA HÀNG (ROW) TRONG ẢNH
#define ROW_DOWN     0    
#define ROW_UP       2    
#define ROW_RIGHT    1
// Lưu ý: Không cần ROW_LEFT vì ta dùng kỹ thuật lật hình từ ROW_RIGHT
//Quản lí các date quan trọng
void InitPlayer(Player *player, PlayerClass chosenClass) {
    //load ảnh main
    //để mặc định
    const char* pathWalk = "resources/player/class_1/main1walk.png"; // Mặc định
    const char* pathIdle = "resources/player/class_1/main1idle.png";

    // Chọn đường dẫn ảnh dựa trên class được truyền vào
    switch (chosenClass) {

        case 0: case CLASS_DAU_GAU:// Class 1
            pathWalk = "resources/player/class_1/main1walk.png";
            pathIdle = "resources/player/class_1/main1idle.png";
            break;

        case 1: case CLASS_HOC_BA: // Class 2
            pathWalk = "resources/player/class_2/main2walk.png"; 
            pathIdle = "resources/player/class_2/main2idle.png";
            break;

        case 2: case CLASS_SOAI_CA: // Class 3
            pathWalk = "resources/player/class_3/main3walk.png"; 
            pathIdle = "resources/player/class_3/main3idle.png";
            break;

        case 3: case CLASS_PHU_NHI_DAI: // Class 4
            pathWalk = "resources/player/class_4/main4walk.png"; 
            pathIdle = "resources/player/class_4/main4idle.png";
            break;
    }

    // Load ảnh thật từ đường dẫn đã chọn ở trên
    player->textureWalk = LoadTexture(pathWalk);
    player->textureIdle = LoadTexture(pathIdle);

    // 1. Gán kích thước frame thủ công
    player->drawWidth = 50.0f;   // Muốn vẽ to nhỏ thì chỉnh ở đây
    player->drawHeight = 50.0f;  // Chỉnh ở đây là Debug tự nhận
    player->spriteWidth = FRAME_WIDTH; 
    player->spriteHeight = FRAME_HEIGHT;
    player->position = (Vector2){100, 100};

    //set mặc định đứng yên
    player->currentTexture = &player->textureIdle;
    player->maxFrames = MAX_FRAME_IDLE;

    // 2. Cấu hình chỉ số RPG và Hỏi Đáp (Gộp chung Cũ và Mới)
    switch (chosenClass) {
        
        // CẶP 1: Đầu gấu / Student (Máu 7, 3 Skill)
        case CLASS_STUDENT:
        case CLASS_DAU_GAU: 
            player->stats = (PlayerStats){150, 150, 100, 20, 0, 2.5f, 5, 20, 0}; 
            player->cbcStats = (CBC_Stats){7, 7, 3, 0, 0, false, false}; 
            break;

        // CẶP 2: Học bá / Warrior (Máu 3, 2 Skill, Nội tại chọn lại)
        case CLASS_WARRIOR:
        case CLASS_HOC_BA:
            player->stats = (PlayerStats){100, 100, 100, 10, 0, 2.5f, 5, 20, 0}; 
            player->cbcStats = (CBC_Stats){3, 3, 2, 0, 0, true, false};  
            break;

        // CẶP 3: Soái ca / Mage (Máu 4, 3 Skill)
        case CLASS_MAGE:
        case CLASS_SOAI_CA:
            player->stats = (PlayerStats){120, 120, 100, 15, 0, 2.5f, 5, 20, 0}; 
            player->cbcStats = (CBC_Stats){4, 4, 3, 0, 0, false, false}; 
            break;

        // CẶP 4: Phú nhị đại / Archer (Máu 5, 2 Skill)
        case CLASS_ARCHER:
        case CLASS_PHU_NHI_DAI:
        default:
            player->stats = (PlayerStats){130, 130, 100, 12, 0, 2.5f, 5, 20,0}; 
            player->cbcStats = (CBC_Stats){5, 5, 2, 0, 0, false, false}; 
            break;
    }       
    
    player->textureWalk = LoadTexture(pathWalk);
    player->textureIdle = LoadTexture(pathIdle);
    player->currentTexture = &player->textureIdle;
    player->pClass = chosenClass;

    // 2. [CẤU HÌNH LẠI CHỈ SỐ VẬT LÝ]
    // Máu 100, Stamina 100 (Thể lực dồi dào), Dame 10
    //player->stats = (PlayerStats){100, 100, 100, 10, 0, 2.5f, 5, 20}; 

    // --- CẤU HÌNH KỸ NĂNG VÕ THUẬT ---
    
    // Skill 1 (Q): Đấm Thường (Nhẹ, không tốn sức)
    strcpy(player->skills[0].name, "Dam Thuong");
    player->skills[0].damage = 15;
    player->skills[0].staminaCost = 0;

    // Skill 2 (W): Đấm Móc (Mạnh hơn, tốn ít sức)
    strcpy(player->skills[1].name, "Dam Moc");
    player->skills[1].damage = 30;
    player->skills[1].staminaCost = 15; // Tốn 15 Thể lực

    // Skill 3 (E): Liên Hoàn Cước (Rất mạnh, tốn nhiều sức)
    strcpy(player->skills[2].name, "Lien Hoan");
    player->skills[2].damage = 50;
    player->skills[2].staminaCost = 40; // Tốn 40 Thể lực

    // 3. Khởi tạo Animation
    player->currentFrame = 0;
    player->framesCounter = 0;
    player->framesSpeed = 8; 
    player->currentDir = FACE_DOWN; // Mặc định nhìn xuống
    // sửa dữ liệu hitbox
    player->hitWidth = 10.0f;  // chỉnh to/nhỏ thì sửa số này
    player->hitHeight = 8.0f;  // Sửa số này
    player->paddingBottom = 6.0f; // Khoảng cách từ chân ảnh đến đáy hitbox
    player->frameRec = (Rectangle){
        0.0f, 
        (float)(ROW_DOWN * player->spriteHeight), 
        (float)player->spriteWidth - 0.5f, 
        (float)player->spriteHeight - 0.5f
    };
}

// Hàm kiểm tra va chạm dự đoán 
bool CheckCollisionFuture(Rectangle hitbox, GameMap *map, Npc *npcList, int npcCount) {
    // 1. Check Tường
    for (int i = 0; i < map->wallCount; i++) {
        if (CheckCollisionRecs(hitbox, map->walls[i])) return true; 
    }

    // 2. Check NPC
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID) { 
            float npcW = (float)npcList[i].texture.width / npcList[i].frameCount;
            float npcH = (float)npcList[i].texture.height;

            // Hitbox NPC nằm ở chân
            float boxWidth = npcList[i].hitWidth;
            float boxHeight = npcList[i].hitHeight;
            float paddingBottom = npcList[i].paddingBottom;
            float offsetX = (npcW - boxWidth) / 2.0f; 

            Rectangle npcFeetRect = { 
                npcList[i].position.x + offsetX,            
                npcList[i].position.y + npcH - paddingBottom - boxHeight, 
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
    Vector2 nextPos = player->position; 
    
    int targetRow = ROW_DOWN; 
    if (!isMoving) {
        if (player->currentDir == FACE_UP) targetRow = ROW_UP;
        else if (player->currentDir == FACE_LEFT || player->currentDir == FACE_RIGHT) targetRow = ROW_RIGHT;
        else targetRow = ROW_DOWN;
    }
    bool isAnyNpcTalking = false;
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID == map->currentMapID && npcList[i].isTalking) {
            isAnyNpcTalking = true;
            break;
        }
    }
    // Nếu KHÔNG có ai đang nói chuyện thì mới cho phép bấm phím di chuyển
    if (!isAnyNpcTalking) {

        // --- 1. XỬ LÝ PHÍM BẤM (INPUT) ---
        //DI chuyển
        if (IsKeyDown(KEY_A)) { 
            nextPos.x -= player->stats.moveSpeed; 
            player->currentDir = FACE_LEFT; 
            targetRow = ROW_RIGHT; // Mẹo: Đi trái dùng ảnh hàng PHẢI
            isMoving = true;
        }
        else if (IsKeyDown(KEY_D)) { 
            nextPos.x += player->stats.moveSpeed; 
            player->currentDir = FACE_RIGHT;
            targetRow = ROW_RIGHT; 
            isMoving = true;
        }
        else if (IsKeyDown(KEY_W)) { 
            nextPos.y -= player->stats.moveSpeed; 
            player->currentDir = FACE_UP;
            targetRow = ROW_UP;   
            isMoving = true;
        }
        else if (IsKeyDown(KEY_S)) { 
            nextPos.y += player->stats.moveSpeed; 
            player->currentDir = FACE_DOWN;
            targetRow = ROW_DOWN; 
            isMoving = true;
        }
    }


    // --- 2. XỬ LÝ VA CHẠM (COLLISION) ---
    // Hitbox siêu nhỏ (chỉ ở chân)
    float hitW = player->hitWidth; 
    float hitH = player->hitHeight;
    
    // Căn giữa hitbox vào nhân vật
    float offsetX = (player->drawWidth - hitW) / 2.0f; 
    float offsetY = player->drawHeight - hitH - player->paddingBottom;

    // [TRỤC X]
    Rectangle boxX = { nextPos.x + offsetX, player->position.y + offsetY, hitW, hitH };
    
    
    bool colX = CheckCollisionFuture(boxX, map, npcList, npcCount);
    
    // Check biên màn hình
    if (nextPos.x < -5 || nextPos.x > SCREEN_WIDTH - player->drawWidth + 5) colX = true;
    
    if (!colX) player->position.x = nextPos.x;    

    // [TRỤC Y]
    Rectangle boxY = { player->position.x + offsetX, nextPos.y + offsetY, hitW, hitH };
    bool colY = CheckCollisionFuture(boxY, map, npcList, npcCount);
    
   if (nextPos.y < -5 || nextPos.y > SCREEN_HEIGHT - player->drawHeight + 5) colY = true;
    
    if (!colY) player->position.y = nextPos.y;

    // --- 3. ANIMATION ---
    if (isMoving) {
        // Nếu đang dùng ảnh IDLE mà chuyển sang đi -> Reset frame về 0 để tránh giật
        if (player->currentTexture == &player->textureIdle) player->currentFrame = 0;
        
        // Trỏ vào bộ ảnh ĐI BỘ
        player->currentTexture = &player->textureWalk;
        player->maxFrames = MAX_FRAME_WALK; // Giới hạn 8 frame
    } else {
        // Nếu đang dùng ảnh WALK mà dừng lại -> Reset frame về 0
        if (player->currentTexture == &player->textureWalk) player->currentFrame = 0;
        
        // Trỏ vào bộ ảnh ĐỨNG Yên
        player->currentTexture = &player->textureIdle;
        player->maxFrames = MAX_FRAME_IDLE; // Giới hạn 14 frame
    }

    // --- 4. CHẠY ANIMATION CHO CẢ 2 TRẠNG THÁI ---
    player->framesCounter++;
    if (player->framesCounter >= player->framesSpeed) {
        player->framesCounter = 0;
        player->currentFrame++;
        
        // Loop dựa trên maxFrames 
        if (player->currentFrame >= player->maxFrames) {
            player->currentFrame = 0;
        }
    }
    
    // --- 5.  CẬP NHẬT VÙNG CẮT ;
    
    player->frameRec.x = (float)(player->currentFrame * player->spriteWidth); 
    player->frameRec.y = (float)(targetRow * player->spriteHeight);
    player->frameRec.width = (float)player->spriteWidth - 0.5f;
    player->frameRec.height = (float)player->spriteHeight - 0.5f;
}

void DrawPlayer(Player *player) {
    // 1. Đích: Vẽ nhỏ
    Rectangle dest = { 
        player->position.x, 
        player->position.y, 
        player->drawWidth,   // <== Tự động
        player->drawHeight   // <== Tự động
    };
    
    // 2. Nguồn: Cắt to
    Rectangle source = player->frameRec;

    // 3. Lật hình (Flip) nếu đi trái
    if (player->currentDir == FACE_LEFT) {
        source.width = -source.width; 
    }

    // 4. Vẽ Texture Pro
    DrawTexturePro(*player->currentTexture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    // 5. Vẽ Thanh Máu
    int barWidth = 30; // Chỉnh nhỏ lại cho hợp với nhân vật bé
    int barHeight = 4;
    
    // Căn giữa thanh máu trên đầu
    int barX = (int)(player->position.x + (player->drawWidth - barWidth) / 2.0f);
    int barY = (int)player->position.y - 8;

    DrawRectangle(barX, barY, barWidth, barHeight, Fade(RED, 0.6f)); // Nền
    
    float hpPercent = (float)player->stats.currentHp / player->stats.maxHp;
    DrawRectangle(barX, barY, (int)(barWidth * hpPercent), barHeight, LIME); // Máu hiện tại
}

void UnloadPlayer(Player *player) {
    UnloadTexture(player->textureWalk);
    UnloadTexture(player->textureIdle);
}