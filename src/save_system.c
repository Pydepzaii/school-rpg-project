// FILE: src/save_system.c
#include "save_system.h"
#include <stdio.h>

// Include thêm các hệ thống để kiểm tra trạng thái
#include "combat.h"
#include "combatbychatting.h"
#include "gameplay.h"

#define SAVE_VERSION 100 // Tương đương bản 1.0.0

extern void Inventory_SaveToFile(FILE *file);
extern void Inventory_LoadFromFile(FILE *file);

void Game_Save(int mapID, Vector2 pos, Player *player) {
    // 1. CHỐT CHẶN AN TOÀN: Không cho phép Save khi đang ở trạng thái nguy hiểm
    if (Combat_IsActive() || CBC_IsActive() || Gameplay_IsEnding()) {
        printf(">> [WARNING] Cannot save right now! (Busy/Combat/Ending)\n");
        // Tùy chọn: Có thể gọi một hàm UI để hiện thông báo "Không thể lưu lúc này!" lên màn hình
        return; 
    }

    SaveData data;
    data.version = SAVE_VERSION; // Gắn mác phiên bản
    data.mapID = mapID;
    data.playerPos = pos;
    data.pClass = player->pClass;
    data.stats = player->stats;
    data.cbcStats = player->cbcStats;
    // [FIX]: Sao chép toàn bộ 4 kỹ năng vào SaveData
    for(int i = 0; i < 4; i++) {
        data.skills[i] = player->skills[i];
    }
    FILE *file = fopen(SAVE_FILE_NAME, "wb"); 
    if (file) {
        fwrite(&data, sizeof(SaveData), 1, file);
        Inventory_SaveToFile(file);
        fclose(file);
        printf(">> [SYSTEM] Game Saved successfully to %s (Version: %d)\n", SAVE_FILE_NAME, SAVE_VERSION);
    } else {
        printf(">> [ERROR] Could not save game!\n");
    }
}

bool Game_Load(int *mapID, Vector2 *pos, Player *player) {
    FILE *file = fopen(SAVE_FILE_NAME, "rb"); 
    if (file) {
        SaveData data;
        size_t readCount = fread(&data, sizeof(SaveData), 1, file);

        if (readCount > 0) {
            // 2. KIỂM TRA PHIÊN BẢN (Chống hỏng Struct)
            if (data.version != SAVE_VERSION) {
                printf(">> [ERROR] Save file version mismatch! Expected %d, got %d. Load aborted to prevent crash.\n", SAVE_VERSION, data.version);
                fclose(file);
                return false; 
            }

            *mapID = data.mapID;
            *pos = data.playerPos;
            player->pClass = data.pClass;
            player->stats = data.stats;
            player->cbcStats = data.cbcStats;
            // [FIX]: Trả lại mảng kỹ năng cho nhân vật
            for(int i = 0; i < 4; i++) {
                player->skills[i] = data.skills[i];
            }
            Inventory_LoadFromFile(file);
            fclose(file);
            printf(">> [SYSTEM] Game Loaded! Map: %d, Progress: %d\n", *mapID, player->stats.storyProgress);
            return true;
        }
        fclose(file);
    }
    printf(">> [ERROR] Save file not found or corrupted.\n");
    return false;
}

bool Game_HasSaveFile() {
    FILE *file = fopen(SAVE_FILE_NAME, "rb");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}