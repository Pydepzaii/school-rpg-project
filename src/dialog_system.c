// FILE: src/dialog_system.c
#include "dialog_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static DialogEvent dialogDatabase[MAX_DIALOG_EVENTS];
static int eventCount = 0;
static bool isInitialized = false;

void Dialog_Init(const char* filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printf(">> [ERROR] Khong tim thay file thoai: %s\n", filePath);
        return;
    }

    char line[512];
    eventCount = 0;

    // Khởi tạo mảng
    memset(dialogDatabase, 0, sizeof(dialogDatabase));

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 5) continue;
        line[strcspn(line, "\n")] = 0; // Xóa dấu xuống dòng

        // Parse format mới: NpcID | Key | Speaker | Content
        char *token = strtok(line, "|");
        if (!token) continue;
        int npcID = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        char key[32];
        strcpy(key, token);

        token = strtok(NULL, "|");
        if (!token) continue;
        int speakerType = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        char content[MAX_DIALOG_LENGTH];
        strcpy(content, token);

        // --- ĐỌC SỐ LƯỢNG LỰA CHỌN ---
        token = strtok(NULL, "|");
        int choiceCount = (token != NULL) ? atoi(token) : 0;
        if (choiceCount > MAX_CHOICES) choiceCount = MAX_CHOICES;

        char tempChoices[MAX_CHOICES][64] = {0};
        char tempNextKeys[MAX_CHOICES][32] = {0};

        // Đọc vòng lặp các lựa chọn (nếu có)
        for (int k = 0; k < choiceCount; k++) {
            token = strtok(NULL, "|");
            if (token) strcpy(tempChoices[k], token);
            token = strtok(NULL, "|");
            if (token) strcpy(tempNextKeys[k], token);
        }

        // Tìm xem Event này đã có trong Database chưa (để gộp chung các dòng thoại)
        DialogEvent *targetEvent = NULL;
        for (int i = 0; i < eventCount; i++) {
            if (dialogDatabase[i].npcID == npcID && strcmp(dialogDatabase[i].key, key) == 0) {
                targetEvent = &dialogDatabase[i];
                break;
            }
        }

        // Nếu chưa có, tạo Event mới
        if (!targetEvent) {
            if (eventCount >= MAX_DIALOG_EVENTS) continue;
            targetEvent = &dialogDatabase[eventCount];
            targetEvent->npcID = npcID;
            strcpy(targetEvent->key, key);
            targetEvent->lineCount = 0;
            eventCount++;
        }

        // Thêm câu thoại vào Event
        if (targetEvent->lineCount < MAX_LINES_PER_EVENT) {
            int idx = targetEvent->lineCount;
            targetEvent->lines[idx].speakerType = speakerType;
            strcpy(targetEvent->lines[idx].content, content);
            targetEvent->lines[idx].choiceCount = choiceCount;
            
            for (int k = 0; k < choiceCount; k++) {
                strcpy(targetEvent->lines[idx].choices[k], tempChoices[k]);
                strcpy(targetEvent->lines[idx].nextKeys[k], tempNextKeys[k]);
            }
            targetEvent->lineCount++;
        }
    }

    fclose(file);
    isInitialized = true;
    printf(">> [DIALOG] Da nap %d su kien thoai tu %s\n", eventCount, filePath);
}

// Hàm lưu dữ liệu ngược ra file (Phục vụ cho Debug Tool)
void Dialog_SaveToFile(const char* filePath) {
    if (!isInitialized) return;
    FILE *file = fopen(filePath, "w");
    if (!file) {
        printf(">> [ERROR] Khong the luu file thoai: %s\n", filePath);
        return;
    }

    fprintf(file, "# Dinh dang: NpcID | Key | Speaker(0=NPC, 1=Player) | Noi dung\n");
    
    for (int i = 0; i < eventCount; i++) {
        DialogEvent *ev = &dialogDatabase[i];
        for (int j = 0; j < ev->lineCount; j++) {
          // Ghi phần gốc và số lượng lựa chọn
            fprintf(file, "%d|%s|%d|%s|%d", 
                ev->npcID, ev->key, ev->lines[j].speakerType, ev->lines[j].content, ev->lines[j].choiceCount);
            
            // Ghi tiếp các lựa chọn lên cùng 1 dòng
            for (int k = 0; k < ev->lines[j].choiceCount; k++) {
                fprintf(file, "|%s|%s", ev->lines[j].choices[k], ev->lines[j].nextKeys[k]);
            }
            fprintf(file, "\n"); // Xuống dòng khi kết thúc 1 câu
        }
    }

    fclose(file);
    printf(">> [DIALOG] Da luu thanh cong vao %s\n", filePath);
}

DialogEvent* Dialog_GetEvent(int npcId, const char* key) {
    if (!isInitialized) return NULL;

    for (int i = 0; i < eventCount; i++) {
        if (dialogDatabase[i].npcID == npcId && strcmp(dialogDatabase[i].key, key) == 0) {
            return &dialogDatabase[i];
        }
    }
    
    // Nếu không có Key này, thử tìm Key DEFAULT
    if (strcmp(key, "DEFAULT") != 0) {
        return Dialog_GetEvent(npcId, "DEFAULT");
    }

    return NULL;
}

void Dialog_Shutdown() {
    eventCount = 0;
    isInitialized = false;
}

// --- CÁC HÀM HỖ TRỢ CHO DIALOG EDITOR TOOL ---
int Dialog_GetNpcEventCount(int npcID) {
    int count = 0;
    for (int i = 0; i < eventCount; i++) {
        if (dialogDatabase[i].npcID == npcID) count++;
    }
    return count;
}

DialogEvent* Dialog_GetNpcEventByIndex(int npcID, int index) {
    int count = 0;
    for (int i = 0; i < eventCount; i++) {
        if (dialogDatabase[i].npcID == npcID) {
            if (count == index) return &dialogDatabase[i];
            count++;
        }
    }
    return NULL;
}

DialogEvent* Dialog_CreateEvent(int npcID, const char* key) {
    if (eventCount >= MAX_DIALOG_EVENTS) return NULL;
    DialogEvent* ev = &dialogDatabase[eventCount];
    ev->npcID = npcID;
    strcpy(ev->key, key);
    ev->lineCount = 0;
    eventCount++;
    return ev;
}