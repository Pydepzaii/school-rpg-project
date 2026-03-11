// FILE: src/dialog_system.h
#ifndef DIALOG_SYSTEM_H
#define DIALOG_SYSTEM_H

#include <stdbool.h>

#define MAX_DIALOG_EVENTS 200      // Tối đa 200 sự kiện trong game
#define MAX_LINES_PER_EVENT 50     // Mỗi sự kiện tối đa 20 câu thoại qua lại
#define MAX_DIALOG_LENGTH 256      // Độ dài tối đa 1 câu thoại
#define MAX_CHOICES 10 // Tối đa 10 lựa chọn cho 1 câu thoại
// Cấu trúc 1 dòng thoại
typedef struct {
    int speakerType; // 0: NPC nói, 1: Player nói
    char content[MAX_DIALOG_LENGTH];
    // --- NÂNG CẤP RẼ NHÁNH (LÊN TỚI 10 LỰA CHỌN) ---
    int choiceCount;                         // Số lượng lựa chọn (0 nghĩa là câu thoại bình thường)
    char choices[MAX_CHOICES][64];           // Mảng chứa text của các lựa chọn
    char nextKeys[MAX_CHOICES][32];          // Mảng chứa Key tiếp theo tương ứng với mỗi lựa chọn
} DialogLine;

// Cấu trúc 1 Sự kiện (Gồm nhiều dòng thoại)
typedef struct {
    int npcID;          // Bỏ MapID, chỉ cần NpcID là đủ
    char key[32];       // Tên sự kiện (VD: "DEFAULT", "QUEST_1")
    
    DialogLine lines[MAX_LINES_PER_EVENT]; // Danh sách các câu thoại
    int lineCount;                         // Số lượng câu thoại hiện có trong sự kiện này
} DialogEvent;

// --- CÁC HÀM ---
void Dialog_Init(const char* filePath);
void Dialog_Shutdown();
void Dialog_SaveToFile(const char* filePath); // Thêm hàm Save để dùng cho Tool

// Trả về toàn bộ Event thay vì chỉ 1 chuỗi để UI có thể duyệt qua từng câu
DialogEvent* Dialog_GetEvent(int npcId, const char* key);
// --- CÁC HÀM HỖ TRỢ CHO DIALOG EDITOR TOOL ---
int Dialog_GetNpcEventCount(int npcID);
DialogEvent* Dialog_GetNpcEventByIndex(int npcID, int index);
DialogEvent* Dialog_CreateEvent(int npcID, const char* key);
#endif