#ifndef INTRO_H
#define INTRO_H

#include "raylib.h"

void InitIntro(const char* fileName); // Nạp video
bool UpdateIntro();                   // Xử lý (trả về true khi video hết)
void DrawIntro();                     // Vẽ video
void UnloadIntro();                   // Dọn dẹp

// --- [THÊM MỚI] Hệ thống Story Cutscene (Chạy trước khi chọn nhân vật) ---
void StoryCutscene_Start(void);
bool StoryCutscene_IsActive(void);
bool StoryCutscene_Update(void); // Trả về true khi Cutscene kết thúc
void StoryCutscene_Draw(void);
void StoryCutscene_Unload(void);

#endif