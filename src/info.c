// FILE: src/info.c
#include "info.h"
#include "ui_style.h"
#include "settings.h" 
#include <stdlib.h>
#include <string.h>

static char* devlogText = NULL;
static char* creditText = NULL;
static InfoTab currentTab = TAB_DEVLOG;

// Hệ thống Phân trang
static int currentSpread = 0; // Số lần lật sách (1 lần lật xem được 2 trang)
static int maxSpreads = 1;
static float textRevealProgress = 1.0f; // [MỚI] Biến chạy từ 0.0 -> 1.0 để lộ chữ

void Info_StartReveal(void) {
    textRevealProgress = 0.0f; 
}

// --- [MỚI] THUẬT TOÁN TỰ ĐỘNG BẺ DÒNG (WORD WRAP) CỰC XỊN ---
static char* WrapText(const char* text, float maxWidth, float fontSize) {
    if (!text) return NULL;
    int len = strlen(text);
    
    // Cấp phát bộ nhớ dư dả (gấp đôi) để thoải mái chèn thêm các ký tự \n
    char* wrapped = (char*)malloc(len * 2 + 1); 
    int wIdx = 0;
    int lineStartIndex = 0;
    int lastSpaceWIdx = -1;

    for (int i = 0; i < len; i++) {
        wrapped[wIdx] = text[i];
        
        if (text[i] == '\n') {
            lineStartIndex = wIdx + 1;
            lastSpaceWIdx = -1;
        } else if (text[i] == ' ') {
            lastSpaceWIdx = wIdx; // Đánh dấu vị trí dấu cách gần nhất
        }

        wIdx++;
        wrapped[wIdx] = '\0'; // Tạm ngắt chuỗi để đo độ dài thực tế
        
        Vector2 size = MeasureTextEx(globalFont, &wrapped[lineStartIndex], fontSize, 1);
        
        // Nếu chữ bắt đầu vượt quá lề phải an toàn
        if (size.x > maxWidth) {
            if (lastSpaceWIdx != -1 && lastSpaceWIdx > lineStartIndex) {
                // Quay lại dấu cách gần nhất và biến nó thành dấu xuống dòng (\n)
                wrapped[lastSpaceWIdx] = '\n';
                lineStartIndex = lastSpaceWIdx + 1;
                lastSpaceWIdx = -1;
            } else {
                // Trường hợp spam chữ dính liền (VD: 3636363636...)
                // Ép cắt đôi chữ xuống dòng ngay lập tức
                wrapped[wIdx - 1] = '\n';
                wrapped[wIdx] = text[i]; 
                wIdx++;
                lineStartIndex = wIdx - 1;
                lastSpaceWIdx = -1;
            }
        }
    }
    wrapped[wIdx] = '\0';
    return wrapped;
}

void Info_Init(void) {
    // Tính toán độ rộng của 1 trang giấy để làm lề bẻ dòng
    float sw = (float)SCREEN_WIDTH;  
    float bookWidth = sw * 0.8f;      
    float paddingX = 35.0f; 
    float gap = -25.0f;       
    float pageWidth = (bookWidth - (paddingX * 2) - gap) / 2.0f;
    
    // Lề an toàn: Cách mép phải của trang giấy đúng 30 pixel
    float safeTextWidth = pageWidth - 40.0f; 

    // Đọc file Text từ ổ cứng
    char* rawDevlog = LoadFileText("resources/info/devlog.txt");
    if (rawDevlog) {
        devlogText = WrapText(rawDevlog, safeTextWidth, 22);
        UnloadFileText(rawDevlog); // Giải phóng bản gốc
    } else {
        devlogText = WrapText("Chua tim thay file devlog.txt!\nHay tao file trong thu muc resources/info/.", safeTextWidth, 22);
    }

    char* rawCredit = LoadFileText("resources/info/credit.txt");
    if (rawCredit) {
        creditText = WrapText(rawCredit, safeTextWidth, 22);
        UnloadFileText(rawCredit); // Giải phóng bản gốc
    } else {
        creditText = WrapText("Chua tim thay file credit.txt!\nHay tao file trong thu muc resources/info/.", safeTextWidth, 22);
    }
}

void Info_Shutdown(void) {
    if (devlogText) free(devlogText);
    if (creditText) free(creditText);
}

void Info_SetTab(InfoTab tab) {
    if (currentTab != tab) {
        currentTab = tab;
        currentSpread = 0; // Reset về trang đầu khi đổi Tab
    }
}
// [NEW] Hàm trả về Tab hiện tại cho các file khác sử dụng
InfoTab Info_GetCurrentTab(void) {
    return currentTab;
}

void Info_NextPage(void) {
    if (currentSpread < maxSpreads - 1) currentSpread++;
}

void Info_PrevPage(void) {
    if (currentSpread > 0) currentSpread--;
}


void Info_Update(void) {
    // Tự động chạy tiến độ hiện chữ theo thời gian thực
    if (textRevealProgress < 1.0f) {
        textRevealProgress += GetFrameTime() * 1.5f; // Tăng số 1.5 để chữ viết nhanh hơn
        if (textRevealProgress > 1.0f) textRevealProgress = 1.0f;
    }
}

void Info_Draw(void) {
    float sw = (float)SCREEN_WIDTH;  
    float sh = (float)SCREEN_HEIGHT;
    float bookWidth = sw * 0.8f;      
    float bookHeight = sh * 0.75f;    
    float startX = (sw - bookWidth) / 2.0f;
    float startY = (sh - bookHeight) / 2.0f + 20.0f; 
    
    float spineX = startX + bookWidth / 2.0f; // Tâm gáy sách

   float paddingX = 35.0f; 
    // --- [SỬA ĐỔI] Tách riêng lề trên và lề dưới, tăng thông số lên để né viền sách ---
    float paddingTop = 55.0f;    // Đẩy khung chữ tụt xuống 55px né viền trên
    float paddingBottom = 65.0f; // Đẩy khung chữ rút lên 65px chừa chỗ cho số trang
    
    float pageWidth = (bookWidth / 2.0f) - paddingX - 20.0f; 
    float pageHeight = bookHeight - paddingTop - paddingBottom;

    Rectangle leftPage = { startX + paddingX, startY + paddingTop, pageWidth, pageHeight };
    Rectangle rightPage = { spineX + paddingX, startY + paddingTop, pageWidth, pageHeight };

    const char* textToDraw = (currentTab == TAB_DEVLOG) ? devlogText : creditText;
    Color textColor = (currentTab == TAB_DEVLOG) ? DARKGRAY : DARKBLUE; 

    Vector2 textSize = MeasureTextEx(globalFont, textToDraw, 22, 1);
    int totalSinglePages = (int)(textSize.y / pageHeight) + 1; 
    maxSpreads = (totalSinglePages + 1) / 2; 

    // --- LOGIC HIỆU ỨNG VIẾT CHỮ (SCISSOR THEO CHIỀU RỘNG) ---
    // Nửa thời gian đầu (0.0 -> 0.5): Hiện trang trái
    float leftReveal = (textRevealProgress > 0.5f) ? 1.0f : (textRevealProgress * 2.0f);
    if (leftReveal > 0.01f) {
        BeginScissorMode((int)leftPage.x, (int)leftPage.y, (int)(leftPage.width * leftReveal), (int)leftPage.height);
            float leftYOffset = leftPage.y - (currentSpread * 2) * pageHeight;
            DrawTextEx(globalFont, textToDraw, (Vector2){leftPage.x, leftYOffset}, 22, 1, textColor);
        EndScissorMode();
    }

    // Nửa thời gian sau (0.5 -> 1.0): Hiện trang phải
    float rightReveal = (textRevealProgress < 0.5f) ? 0.0f : ((textRevealProgress - 0.5f) * 2.0f);
    if (rightReveal > 0.01f) {
        BeginScissorMode((int)rightPage.x, (int)rightPage.y, (int)(rightPage.width * rightReveal), (int)rightPage.height);
            float rightYOffset = rightPage.y - (currentSpread * 2 + 1) * pageHeight;
            DrawTextEx(globalFont, textToDraw, (Vector2){rightPage.x, rightYOffset}, 22, 1, textColor);
        EndScissorMode();
    }
    
  // Vẽ số trang ở góc dưới cùng (Ghim cố định theo độ cao thực tế của sách)
    float pageNumY = startY + bookHeight - 45.0f; // Luôn cách đáy sách 45px
    
    DrawTextEx(globalFont, TextFormat("Trang %d", currentSpread * 2 + 1), (Vector2){leftPage.x, pageNumY}, 16, 1, GRAY);
    if (currentSpread * 2 + 2 <= totalSinglePages) {
        DrawTextEx(globalFont, TextFormat("Trang %d", currentSpread * 2 + 2), (Vector2){rightPage.x + pageWidth - 80, pageNumY}, 16, 1, GRAY);
    }
}