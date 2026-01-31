// FILE: src/dialog_system.h
#ifndef DIALOG_SYSTEM_H
#define DIALOG_SYSTEM_H

#include <stdbool.h>

// Định nghĩa độ dài tối đa
#define MAX_DIALOG_ENTRIES 500  // Tối đa 500 câu thoại trong game
#define MAX_DIALOG_LENGTH 256   // Mỗi câu dài tối đa 256 ký tự

// Cấu trúc một dòng thoại trong bộ nhớ
typedef struct {
    int mapID;          // Map nào?
    int npcID;          // NPC nào?
    char key[32];       // Trạng thái (VD: "DEFAULT", "QUEST_1")
    char content[MAX_DIALOG_LENGTH]; // Nội dung: "Xin chào..."
} DialogEntry;

// --- CÁC HÀM ---
void Dialog_Init(const char* filePath);    // Nạp file txt vào RAM
void Dialog_Shutdown();                    // Dọn dẹp bộ nhớ

// Hàm quan trọng: Lấy thoại dựa trên ID và Trạng thái
// Nếu không tìm thấy, nó sẽ trả về câu mặc định "..."
const char* Dialog_Get(int mapId, int npcId, const char* key);

#endif