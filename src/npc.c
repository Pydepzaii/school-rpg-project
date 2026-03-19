#include "npc.h"
#include "settings.h"
#include <string.h> 
#include "gameplay.h" // [Quan trọng] Cần để dùng MAP_THU_VIEN
#include "raylib.h"
#include <ui_style.h>
#include "combatbychatting.h"

void InitNpc(Npc *npc, int mapID, char *texturePath, Vector2 pos, char *name, int id) {
    npc->id = id; // [Fix] Gán ID được truyền vào
    npc->mapID = mapID;
    npc->position = pos;
    strcpy(npc->name, name); 

    // Load Texture (Có kiểm tra file tồn tại để tránh crash)
    if (FileExists(texturePath)) {
        npc->texture = LoadTexture(texturePath);  // đã vất vào trong lệnh if
    } else {
        Image img = GenImageColor(32, 48, RED); // Ảnh tạm màu đỏ nếu thiếu file
        npc->texture = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    // [ANIMATION CONFIG]
    npc->frameCount = 4;        // Số lượng frame ngang trong ảnh (Sprite Sheet)
    npc->currentFrame = 0;
    npc->frameTimer = 0.0f;
    npc->frameSpeed = 0.2f;     // Tốc độ chuyển frame (giây) -> Càng nhỏ càng nhanh
    npc->isTalking = false;
    npc->isDead = false;
    npc->currentDialogLine = 0;
    //dữ liệu hitbox npc
    npc->hitWidth = 24.0f;      // Chiều rộng
    npc->hitHeight = 10.0f;     // Chiều cao
    npc->paddingBottom = 15.0f; // Khoảng cách từ đáy ảnh lên hitbox

    // [QUAN TRỌNG] Khởi tạo chỉ số Combat mặc định (Tránh lỗi 0 máu)
    npc->stats.maxHp = 100;
    npc->stats.currentHp = 100;
    npc->stats.damage = 10;
    npc->stats.defense = 0;
    npc->stats.expReward = 10;
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
   
    DrawTextEx(globalFont, npc->name, (Vector2){npc->position.x, npc->position.y - 20}, 10, 1, DARKGRAY);
}

// --- TẢI NPC THEO MAP ---
void Npc_LoadForMap(int mapID, Npc *npcList, int *npcCount) {
    *npcCount = 0;

    switch (mapID) {
        case MAP_TOA_ALPHA:
            // 1. CÔ LỄ TÂN (Đứng ở quầy bên trái)
            // Thay tọa độ (Vector2){100, 200} bằng tọa độ thực tế trên map Nhà Alpha của bạn
            InitNpc(&npcList[*npcCount], MAP_TOA_ALPHA, "resources/npc/map_alpha/coletan.png", (Vector2){533, 240}, "Cô Lễ Tân", NPC_CO_THU_KY);
            strcpy(npcList[*npcCount].dialogKey, "SCENE_1"); 
            (*npcCount)++;

            // 2. THẦY TUẤN VM (Đứng gần bảng thông báo ở giữa)
            // Thay tọa độ (Vector2){400, 250} bằng tọa độ thực tế
            InitNpc(&npcList[*npcCount], MAP_TOA_ALPHA, "resources/npc/map_lab/thaytuanvm.png", (Vector2){10,40}, "Thầy Tuấn VM", NPC_THAY_TUAN_VM);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        case MAP_NHA_VO:
            // 1. THẦY CHÍNH (Đứng bên trái)
            InitNpc(&npcList[*npcCount], MAP_NHA_VO, "resources/npc/map_nha_vo/thaychinh.png", (Vector2){130, 120}, "Thầy Chính", NPC_THAY_CHINH);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;

            // 2. THẦY HÙNG (Đứng bên phải)
            InitNpc(&npcList[*npcCount], MAP_NHA_VO, "resources/npc/map_nha_vo/thayhung.png", (Vector2){670, 120}, "Thầy Hùng", NPC_THAY_HUNG);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        case MAP_NHA_AN:
            // 1. CHÚ PHỤ BẾP (Bên trái)
            // Thay (Vector2){200, 300} bằng tọa độ thực tế đo bằng phím Chuột Phải
            InitNpc(&npcList[*npcCount], MAP_NHA_AN, "resources/npc/map_nha_an/chudaubep.png", (Vector2){80, 160}, "Chú Đầu Bếp", NPC_CHU_PHU_BEP);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;

            // 2. CÔ BẾP TRƯỞNG (Bên phải)
            InitNpc(&npcList[*npcCount], MAP_NHA_AN, "resources/npc/map_nha_an/codaubep.png", (Vector2){515, 160}, "Cô Đầu Bếp", NPC_CO_BEP_TRUONG);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        case MAP_THU_VIEN:
            // --- 1. CHÚ LAO CÔNG (BOSS CHẶN CỬA - ID 7) ---
            // Tạm để tọa độ (400, 300) ở giữa phòng, bạn dùng Chuột phải để đo và sửa lại nhé
            InitNpc(&npcList[*npcCount], MAP_THU_VIEN, "resources/npc/map_thu_vien/laocongdelta.png", (Vector2){370, 200}, "Chú Lao Công", 7);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;

            // --- 2. CÔ THỦ THƯ (BOSS GIAO QUEST - ID 8) ---
            // Tạm để tọa độ (250, 420) ở quầy góc dưới trái
            InitNpc(&npcList[*npcCount], MAP_THU_VIEN, "resources/npc/map_thu_vien/cothuthu.png", (Vector2){180, 345}, "Cô Thủ Thư", 8);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        case MAP_BETA:
            // --- 1. CHÚ LAO CÔNG 2 (BOSS GÁC CỔNG - ID 9) ---
            // Đứng ngoài hành lang chỗ vũng nước
            InitNpc(&npcList[*npcCount], MAP_BETA, "resources/npc/map_beta/chulaocong.png", (Vector2){ 630, 90 }, "Lao Công", 9);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;

            // --- 2. THẦY ANH / CHỦ NHIỆM (BOSS CHÍNH - ID 10) ---
            // Đứng trong lớp học chờ người chơi
            InitNpc(&npcList[*npcCount], MAP_BETA, "resources/npc/map_beta/thayanh.png", (Vector2){ 60, 110 }, "Thầy Anh", 10);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        case MAP_LAB:
            // --- THẦY HIỆU TRƯỞNG (BOSS CUỐI CÙNG - ID 12) ---
            // Tọa độ đứng giữa phòng, trước dàn máy tính
            InitNpc(&npcList[*npcCount], MAP_LAB, "resources/npc/map_lab/hieu_truong.png", (Vector2){ 396, 231 }, "Thầy Hiệu Trưởng", 12);
            strcpy(npcList[*npcCount].dialogKey, "DEFAULT"); 
            (*npcCount)++;
            break;
        default:
            break;
    }
}

void UnloadNpc(Npc *npc) {
    UnloadTexture(npc->texture);
}
