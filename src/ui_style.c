// FILE: src/ui_style.c
#include "ui_style.h"

// [QUAN TRỌNG] Phải có dòng này để kích hoạt code của Raygui
#define RAYGUI_IMPLEMENTATION 
#include "raygui.h"

Font globalFont = { 0 };

void InitUIStyle() {
    // 1. Danh sách ký tự Tiếng Việt
    const char *vietnameseChars = 
        "aáàảãạăắằẳẵặâấầẩẫậbcdeéèẻẽẹêếềểễệfghiíìỉĩịjklmnoóòỏõọôốồổỗộơớờởỡợ"
        "pqrstuúùủũụưứừửữựvwxyýỳỷỹỵz"
        "AÁÀẢÃẠĂẮẰẲẴẶÂẤẦẨẪẬBCDEÉÈẺẼẸÊẾỀỂỄỆFGHIÍÌỈĨỊJKLMNOÓÒỎÕỌÔỐỒỔỖỘƠỚỜỞỠỢ"
        "PQRSTUÚÙỦŨỤƯỨỪỬỮỰVWXYÝỲỶỸỴZ"
        "0123456789!@#$%^&*()_+-=[]{};':\",./<>?|\\ ";

    // 2. Load Codepoints
    int codepointCount = 0;
    int *codepoints = LoadCodepoints(vietnameseChars, &codepointCount);

    // 3. Nạp Font (Nhớ đổi tên file font trong folder resources thành game_font.ttf)
    globalFont = LoadFontEx("resources/game_font.ttf", 28, codepoints, codepointCount);

    // 4. Set font cho Raygui
    GuiSetFont(globalFont);

    // 5. Giải phóng
    UnloadCodepoints(codepoints);
}

void CloseUIStyle() {
    UnloadFont(globalFont);
}