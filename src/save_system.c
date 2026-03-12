// FILE: src/save_system.c
#include "save_system.h"
#include <stdio.h>

// Kéo 2 hàm thao tác file từ inventory.c sang
extern void Inventory_SaveToFile(FILE *file);
extern void Inventory_LoadFromFile(FILE *file);

void Game_Save(int mapID, Vector2 pos, Player *player) {
    SaveData data;
    data.mapID = mapID;
    data.playerPos = pos;
    
    // Lấy thông tin Player
    data.pClass = player->pClass;
    data.stats = player->stats;
    data.cbcStats = player->cbcStats;

    FILE *file = fopen(SAVE_FILE_NAME, "wb"); // wb = write binary
    if (file) {
        // 1. Ghi dữ liệu cơ bản của nhân vật và Cốt truyện
        fwrite(&data, sizeof(SaveData), 1, file);
        
        // 2. Ghi nối dữ liệu Túi đồ và Đồ đang rớt dưới đất vào đuôi file
        Inventory_SaveToFile(file);
        
        fclose(file);
        printf(">> [SYSTEM] Game Saved successfully to %s\n", SAVE_FILE_NAME);
    } else {
        printf(">> [ERROR] Could not save game!\n");
    }
}

bool Game_Load(int *mapID, Vector2 *pos, Player *player) {
    FILE *file = fopen(SAVE_FILE_NAME, "rb"); // rb = read binary
    if (file) {
        SaveData data;
        size_t readCount = fread(&data, sizeof(SaveData), 1, file);

        if (readCount > 0) {
            *mapID = data.mapID;
            *pos = data.playerPos;
            
            // Phục hồi chỉ số
            player->pClass = data.pClass;
            player->stats = data.stats;
            player->cbcStats = data.cbcStats;
            
            // Đọc nối dữ liệu Túi đồ từ file để khôi phục
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