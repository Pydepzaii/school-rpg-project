// FILE: include/ui_style.h
#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "raylib.h"

// Biến Font toàn cục (extern = dùng chung mọi nơi)
extern Font globalFont;

// Hàm khởi tạo Font và cài đặt Tiếng Việt
void InitUIStyle();

// Hàm giải phóng bộ nhớ khi tắt game
void CloseUIStyle();

#endif