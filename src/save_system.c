// FILE: src/save_system.c
#include "save_system.h"
#include <stdio.h>

void Game_Save(int mapID, Vector2 pos, PlayerStats stats) {
    SaveData data;
    data.mapID = mapID;
    data.playerPos = pos;
    data.stats = stats;

    FILE *file = fopen(SAVE_FILE_NAME, "wb"); // wb = write binary
    if (file) {
        fwrite(&data, sizeof(SaveData), 1, file);
        fclose(file);
        printf(">> [SYSTEM] Game Saved successfully to %s\n", SAVE_FILE_NAME);
    } else {
        printf(">> [ERROR] Could not save game!\n");
    }
}

bool Game_Load(int *mapID, Vector2 *pos, PlayerStats *stats) {
    FILE *file = fopen(SAVE_FILE_NAME, "rb"); // rb = read binary
    if (file) {
        SaveData data;
        size_t readCount = fread(&data, sizeof(SaveData), 1, file);
        fclose(file);

        if (readCount > 0) {
            *mapID = data.mapID;
            *pos = data.playerPos;
            *stats = data.stats;
            printf(">> [SYSTEM] Game Loaded!\n");
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