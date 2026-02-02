// FILE: src/ui_style.c
#include "ui_style.h"
#include "raygui.h"

// [CONFIG] Raygui Implementation
// Define này BẮT BUỘC phải có đúng 1 lần trong toàn bộ project để kích hoạt code Raygui.
// [LƯU Ý]: Nếu bạn include file này ở nhiều nơi, sẽ bị lỗi "Multiple definition". Chỉ để ở file .c này thôi.
#define RAYGUI_IMPLEMENTATION 
#include "raygui.h"
static Texture2D texDialogBox;
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
    globalFont = LoadFontEx("resources/font_dialog/game_font.ttf", 28, codepoints, codepointCount);
    //LOAD UI dailog
    texDialogBox = LoadTexture("resources/font_dialog/dialog_box.png");
    SetTextureFilter(texDialogBox, TEXTURE_FILTER_BILINEAR);
    // 4. Apply to Raygui
    // Set font mặc định cho các control của thư viện Raygui (Button, Textbox...)
    GuiSetFont(globalFont);
    SetTextureFilter(globalFont.texture, TEXTURE_FILTER_BILINEAR);

    // 5. Cleanup RAM
    UnloadCodepoints(codepoints);
}

void CloseUIStyle() {
    UnloadFont(globalFont);
    UnloadTexture(texDialogBox);
}
//Hàm phụ trách vẽ khung hội thoại bằng texture
void UI_DrawDialog(const char* name, const char* content) {
    // 1. Định nghĩa vị trí khung thoại 
    Rectangle boxRec = { 80, 90, 636, 334  };

    // 2. Cấu hình 9-Slice (Cắt ảnh)
    NPatchInfo ninePatchInfo = { 
        .source = (Rectangle){ 0.0f, 0.0f, (float)texDialogBox.width, (float)texDialogBox.height },
        .left = 12, .top = 12, .right = 12, .bottom = 12, 
        .layout = NPATCH_NINE_PATCH 
    };

    // 3. Vẽ khung ảnh
    DrawTextureNPatch(texDialogBox, ninePatchInfo, boxRec, (Vector2){0,0}, 0.0f, WHITE);

    // 4. Vẽ chữ (Dùng Font global đã load)
    // Tên NPC (Màu vàng)
    DrawTextEx(globalFont, name, (Vector2){ boxRec.x + 30, boxRec.y + 200 }, 28, 1, YELLOW);
    
    // Nội dung thoại 
    DrawTextEx(globalFont, content, (Vector2){ boxRec.x + 30, boxRec.y + 240 }, 24, 1, WHITE);
    
    // Hướng dẫn đóng (Màu xám)
    DrawTextEx(globalFont, "Bấm [E] để đóng", (Vector2){ boxRec.x + 450, boxRec.y + 300 }, 18, 1, GREEN);
}