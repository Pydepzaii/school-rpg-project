// FILE: src/save_system.c
#include "save_system.h"
#include <stdio.h>

void Game_Save(int mapID, Vector2 pos, Player *player) {
    SaveData data;
    data.mapID = mapID;
    data.playerPos = pos;
    
    // Copy toàn bộ thông tin quan trọng của Player vào gói Data
    data.pClass = player->pClass;
    data.stats = player->stats;
    data.cbcStats = player->cbcStats;

    FILE *file = fopen(SAVE_FILE_NAME, "wb"); // wb = write binary
    if (file) {
        fwrite(&data, sizeof(SaveData), 1, file);
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
        fclose(file);

        if (readCount > 0) {
            *mapID = data.mapID;
            *pos = data.playerPos;
            
            // Phục hồi chỉ số
            player->pClass = data.pClass;
            player->stats = data.stats;
            player->cbcStats = data.cbcStats;
            
            printf(">> [SYSTEM] Game Loaded! Map: %d, Progress: %d\n", *mapID, player->stats.storyProgress);
            return true;
        }
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