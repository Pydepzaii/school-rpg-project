// FILE: src/ui_style.c
#include "ui_style.h"

// [CONFIG] Raygui Implementation
// Define này BẮT BUỘC phải có đúng 1 lần trong toàn bộ project để kích hoạt code Raygui.
// [LƯU Ý]: Nếu bạn include file này ở nhiều nơi, sẽ bị lỗi "Multiple definition". Chỉ để ở file .c này thôi.
#define RAYGUI_IMPLEMENTATION 
#include "raygui.h"

Font globalFont = { 0 };

void InitUIStyle() {
    // 1. VIETNAMESE CHARSET
    // Danh sách đầy đủ các ký tự Tiếng Việt có dấu để load vào VRAM.
    // Nếu thiếu ký tự nào ở đây, trong game sẽ hiện ô vuông (□).
    const char *vietnameseChars = 
        "aáàảãạăắằẳẵặâấầẩẫậbcdđeéèẻẽẹêếềểễệfghiíìỉĩịjklmnoóòỏõọôốồổỗộơớờởỡợ"
        "pqrstuúùủũụưứừửữựvwxyýỳỷỹỵz"
        "AÁÀẢÃẠĂẮẰẲẴẶÂẤẦẨẪẬBCDĐEÉÈẺẼẸÊẾỀỂỄỆFGHIÍÌỈĨỊJKLMNOÓÒỎÕỌÔỐỒỔỖỘƠỚỜỞỠỢ"
        "PQRSTUÚÙỦŨỤƯỨỪỬỮỰVWXYÝỲỶỸỴZ"
        "0123456789!@#$%^&*()_+-=[]{};':\",./<>?|\\ ";

    // 2. Generate Codepoints
    int codepointCount = 0;
    int *codepoints = LoadCodepoints(vietnameseChars, &codepointCount);

    // 3. Load Font
    // Lưu ý: Font phải hỗ trợ Unicode. Size 28 là base size, khi vẽ có thể scale.
    // [GIẢI THÍCH]: Load font với bộ ký tự tùy chỉnh để nhẹ RAM hơn là load full bộ Unicode.
    globalFont = LoadFontEx("resources/game_font.ttf", 28, codepoints, codepointCount);

    // 4. Apply to Raygui
    // Set font mặc định cho các control của thư viện Raygui (Button, Textbox...)
    GuiSetFont(globalFont);
    SetTextureFilter(globalFont.texture, TEXTURE_FILTER_BILINEAR);

    // 5. Cleanup RAM
    UnloadCodepoints(codepoints);
}

void CloseUIStyle() {
    UnloadFont(globalFont);
}