#ifndef INTRO_H
#define INTRO_H

#include "raylib.h"

void InitIntro(const char* fileName); // Nạp video
bool UpdateIntro();                   // Xử lý (trả về true khi video hết)
void DrawIntro();                     // Vẽ video
void UnloadIntro();                   // Dọn dẹp

#endif