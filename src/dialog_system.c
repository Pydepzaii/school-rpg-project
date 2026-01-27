// FILE: src/dialog_system.c
#include "dialog_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static DialogEntry dialogDatabase[MAX_DIALOG_ENTRIES];
static int dialogCount = 0;
static bool isInitialized = false;

void Dialog_Init(const char* filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf(">> [ERROR] Khong tim thay file thoai: %s\n", filePath);
        return;
    }

    char line[512]; // Buffer đọc từng dòng
    dialogCount = 0;

    while (fgets(line, sizeof(line), file)) {
        // 1. Bỏ qua dòng trống hoặc comment (#)
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 5) continue;

        // 2. Xóa ký tự xuống dòng ở cuối (\n)
        line[strcspn(line, "\n")] = 0;

        // 3. Tách chuỗi dựa trên dấu gạch đứng '|'
        // Cú pháp: MapID | NpcID | Key | Content
        char *token = strtok(line, "|");
        if (!token) continue;
        int mapID = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        int npcID = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        char key[32];
        strcpy(key, token);

        token = strtok(NULL, "|");
        if (!token) continue;
        char content[256];
        strcpy(content, token);

        // 4. Lưu vào Database
        if (dialogCount < MAX_DIALOG_ENTRIES) {
            dialogDatabase[dialogCount].mapID = mapID;
            dialogDatabase[dialogCount].npcID = npcID;
            strcpy(dialogDatabase[dialogCount].key, key);
            strcpy(dialogDatabase[dialogCount].content, content);
            
            dialogCount++;
        }
    }

    fclose(file);
    isInitialized = true;
    printf(">> [DIALOG] Da nap %d cau thoai tu %s\n", dialogCount, filePath);
}

const char* Dialog_Get(int mapId, int npcId, const char* key) {
    if (!isInitialized) return "...";

    // Duyệt mảng để tìm câu thoại khớp 3 điều kiện
    for (int i = 0; i < dialogCount; i++) {
        if (dialogDatabase[i].mapID == mapId &&
            dialogDatabase[i].npcID == npcId &&
            strcmp(dialogDatabase[i].key, key) == 0) {
            
            return dialogDatabase[i].content;
        }
    }
    
    // Fallback: Nếu không tìm thấy key cụ thể, thử tìm key "DEFAULT"
    if (strcmp(key, "DEFAULT") != 0) {
        return Dialog_Get(mapId, npcId, "DEFAULT");
    }

    return "..."; // Không tìm thấy gì cả
}

void Dialog_Shutdown() {
    // Nếu dùng malloc thì free ở đây, hiện tại dùng mảng tĩnh nên không cần làm gì
    dialogCount = 0;
    isInitialized = false;
}