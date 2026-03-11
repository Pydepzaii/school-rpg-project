// FILE: src/intro.c
#include "intro.h"
#include "audio_manager.h" 
#include <stdio.h>
#include <stdlib.h>
#include "ui_style.h"
#include "settings.h"
#include <string.h>
#include <math.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

// [STATE] Video Context
static plm_t *plm = NULL;
static Texture2D videoTexture = { 0 };
static uint8_t *rgbBuffer = NULL;
static bool isInitialized = false;

// [CALLBACK] Video Decoder
// Hàm này được gọi bởi thư viện pl_mpeg mỗi khi giải mã xong 1 frame.
// Chuyển đổi dữ liệu YUV sang RGB để Raylib có thể vẽ được.
void OnVideoFrame(plm_t *plm, plm_frame_t *frame, void *user) {
    uint8_t *buffer = (uint8_t *)user;
    plm_frame_to_rgb(frame, buffer, frame->width * 3);
}

void InitIntro(const char* fileName) {
    plm = plm_create_with_filename(fileName);
    if (!plm) {
        printf(">> [ERROR] Intro File Not Found: %s\n", fileName);
        isInitialized = false;
        return;
    }
    
    // [CONFIG] Disable Internal Audio
    // Tắt audio của video decoder để dùng Audio Manager của Raylib (đồng bộ tốt hơn).
    plm_set_audio_enabled(plm, 0);

    // [MEMORY] Buffer Setup
    // Cấp phát bộ nhớ cho từng pixel của frame video.
    int w = plm_get_width(plm);
    int h = plm_get_height(plm);
    rgbBuffer = (uint8_t *)malloc(w * h * 3);
    // [GIẢI THÍCH]: Đăng ký hàm callback để nhận dữ liệu ảnh từ video.
    plm_set_video_decode_callback(plm, OnVideoFrame, rgbBuffer);

    // Tạo Texture rỗng để hứng dữ liệu video
    Image image = GenImageColor(w, h, BLACK);
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8; 
    videoTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    // Trigger nhạc intro
    Audio_PlayMusic(MUSIC_INTRO);
    
    isInitialized = true;
}

bool UpdateIntro() {
    if (!isInitialized) return true; // Skip nếu lỗi init

    // [INPUT] Skip Logic
    // Bấm bất kỳ phím nào hoặc chuột trái để bỏ qua intro.
    if (GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return true; 
    }

    // [CORE] Decode Frame
    // Giải mã frame tiếp theo dựa trên thời gian trôi qua (Delta Time).
    plm_decode(plm, GetFrameTime());
    UpdateTexture(videoTexture, rgbBuffer); // Upload dữ liệu mới lên GPU

    // Check kết thúc video
    if (plm_has_ended(plm)) return true;

    return false; // Vẫn đang chạy
}

void DrawIntro() {
    if (!isInitialized) return;

    // Vẽ Fullscreen (Scale texture video ra toàn màn hình)
    Rectangle source = { 0, 0, (float)videoTexture.width, (float)videoTexture.height };
    Rectangle dest = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
    
    DrawTexturePro(videoTexture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    DrawText("Bam phim bat ky de bo qua", 20, GetScreenHeight() - 40, 20, LIGHTGRAY);
}

void UnloadIntro() {
    if (isInitialized) {
        Audio_StopMusic(MUSIC_INTRO);

        // [CLEANUP] Free toàn bộ bộ nhớ video
        plm_destroy(plm);
        UnloadTexture(videoTexture);
        if (rgbBuffer) free(rgbBuffer);
        
        isInitialized = false;
    }
}
// ==========================================
// PHẦN 2: STORY CUTSCENE (THÊM MỚI)
// ==========================================

typedef struct {
    Texture2D bg;
    const char* text;
    bool hasBg;
} StoryFrame;

static StoryFrame storyFrames[7];
static int currentStoryIndex = 0;
static bool isStoryActive = false;

// Trạng thái: 0: Fade In, 1: Typing, 2: Wait Click, 3: Fade Out
static int storyState = 0;
static float storyAlpha = 1.0f;
static char currentDisplayedText[2048] = {0};
static int letterCount = 0;
static float typeTimer = 0.0f;

void StoryCutscene_Start(void) {
    // Tải ảnh và chèn kịch bản (Đã ngắt dòng bằng \n và sửa lỗi font dấu gạch ngang)
    storyFrames[0] = (StoryFrame){ .hasBg = false, .text = "Trò chơi lấy bối cảnh tại Trường Đại học FPT.\nMọi nhân vật, sự kiện và chi tiết trong game đều là hư cấu." };
    
    Texture2D tex1 = LoadTexture("resources/intro/scene1.png");
    // Đã thêm \n để chia thành 3 dòng, thay dấu gạch dài thành dấu trừ ngang (-)
    storyFrames[1] = (StoryFrame){ .bg = tex1, .hasBg = true, .text = "Trường Đại học FPT - nơi quy tụ những con người ưu tú và kiệt xuất.\n\nVà trên đỉnh cao ấy, danh hiệu cao quý bậc nhất\n- Cóc Vàng - được trao cho sinh viên xuất sắc nhất…" };
    
    Texture2D tex2 = LoadTexture("resources/intro/scene2.png");
    storyFrames[2] = (StoryFrame){ .bg = tex2, .hasBg = true, .text = "[6:00 AM, Thứ Sáu]\nLại một buổi sáng nhàm chán…\nChỉ cần cố nốt hôm nay thôi, là được nghỉ rồi." };
    
    Texture2D tex3 = LoadTexture("resources/intro/scene3.png");
    storyFrames[3] = (StoryFrame){ .bg = tex3, .hasBg = true, .text = "Đã là ngày thứ 13 kể từ khi cậu ấy nghỉ học rồi…\nKhông biết có chuyện gì xảy ra không nữa." };
    
    Texture2D tex4 = LoadTexture("resources/intro/scene4.png");
    // Đã chèn thêm \n\n giữa các tin nhắn để tạo cảm giác giống khung chat Messenger
    storyFrames[4] = (StoryFrame){ .bg = tex4, .hasBg = true, .text = "Thứ Hai - 07:18 AM\n Cậu có chuyện gì vậy?\n\nThứ Hai - 09:42 PM\n Hôm nay cũng không đi học à?\n\nThứ Tư - 06:55 AM\n Sao mấy hôm nay không thấy cậu nữa?\n\nThứ Tư - 10:11 PM\n Cậu ổn không? Trả lời tớ với." };
    
    Texture2D tex5 = LoadTexture("resources/intro/scene5.png");
    storyFrames[5] = (StoryFrame){ .bg = tex5, .hasBg = true, .text = "Thứ Sáu (hiện tại) - 08:21 PM\n Giúp tớ với…\n\n---------------------------------\nHệ thống - 08:22 AM\n Bạn không thể gửi tin nhắn này.\n Người dùng đã chặn liên lạc." };
    
    storyFrames[6] = (StoryFrame){ .hasBg = false, .text = "Rốt cuộc chuyện gì đã xảy ra?\nMai là ngày nghỉ rồi…\nCó lẽ mình phải làm gì đó." };

    currentStoryIndex = 0;
    storyState = 0; // Bắt đầu bằng Fade In
    storyAlpha = 1.0f; // Màn hình đen hoàn toàn
    letterCount = 0;
    currentDisplayedText[0] = '\0';
    isStoryActive = true;
}

bool StoryCutscene_IsActive(void) {
    return isStoryActive;
}

bool StoryCutscene_Update(void) {
    if (!isStoryActive) return true;
    
    float dt = GetFrameTime();
    
    switch (storyState) {
        case 0: // FADE IN (Mở khung hình)
            storyAlpha -= 1.5f * dt;
            if (storyAlpha <= 0.0f) {
                storyAlpha = 0.0f;
                storyState = 1; // Mở xong -> Chuyển sang gõ chữ
            }
            break;
            
        case 1: // TYPING (Gõ chữ)
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // Click chuột -> Bỏ qua hiệu ứng gõ, hiện full chữ ngay lập tức
                strcpy(currentDisplayedText, storyFrames[currentStoryIndex].text);
                letterCount = strlen(currentDisplayedText);
                storyState = 2; // Chờ click tiếp theo
            } else {
                typeTimer += dt;
                if (typeTimer > 0.03f) { // Tốc độ gõ chữ (thấp = nhanh)
                    typeTimer = 0.0f;
                    const char* fullText = storyFrames[currentStoryIndex].text;
                    
                    if (letterCount < (int)strlen(fullText)) {
                        // Logic cắt chuỗi an toàn cho Tiếng Việt (UTF-8)
                        int bytes = 1;
                        unsigned char c = fullText[letterCount];
                        if ((c & 0xE0) == 0xC0) bytes = 2;
                        else if ((c & 0xF0) == 0xE0) bytes = 3;
                        else if ((c & 0xF8) == 0xF0) bytes = 4;
                        
                        for(int i = 0; i < bytes; i++) {
                            currentDisplayedText[letterCount] = fullText[letterCount];
                            letterCount++;
                        }
                        currentDisplayedText[letterCount] = '\0';
                        
                        if (letterCount % 3 == 0) Audio_PlaySoundEffect(SFX_UI_HOVER); // Tiếng "tạch tạch"
                    } else {
                        storyState = 2; // Gõ xong
                    }
                }
            }
            break;
            
        case 2: // WAIT FOR CLICK (Chờ bấm chuột để sang cảnh)
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                storyState = 3; // Bắt đầu đóng cảnh
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            break;
            
        case 3: // FADE OUT (Đóng khung hình)
            storyAlpha += 1.5f * dt;
            if (storyAlpha >= 1.0f) {
                storyAlpha = 1.0f;
                currentStoryIndex++; // Sang scene tiếp theo
                
                if (currentStoryIndex >= 7) {
                    isStoryActive = false;
                    StoryCutscene_Unload(); // Dọn dẹp RAM
                    return true; // Báo hiệu đã xem xong toàn bộ
                } else {
                    // Reset chuẩn bị cho Scene mới
                    storyState = 0; 
                    letterCount = 0;
                    currentDisplayedText[0] = '\0';
                }
            }
            break;
    }
    
    return false;
}

void StoryCutscene_Draw(void) {
    if (!isStoryActive) return;
    
    // 1. Vẽ Background
    if (storyFrames[currentStoryIndex].hasBg) {
        Texture2D bg = storyFrames[currentStoryIndex].bg;
        Rectangle src = { 0, 0, (float)bg.width, (float)bg.height };
        Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
        DrawTexturePro(bg, src, dst, (Vector2){0,0}, 0.0f, WHITE);
        
        // Phủ màu đen mờ (40%) để chữ nổi lên
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.4f));
    } else {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    }
    
    // 2. Vẽ Chữ
    Vector2 tSize = MeasureTextEx(globalFont, currentDisplayedText, 24, 1);
    Vector2 textPos = {
        (SCREEN_WIDTH - tSize.x) / 2.0f,
        (SCREEN_HEIGHT - tSize.y) / 2.0f
    };
    
    // Nếu là đoạn chat dài (Scene 4, 5), đẩy chữ cao lên một chút cho đẹp
    if (currentStoryIndex == 4 || currentStoryIndex == 5) {
        textPos.y = 80.0f;
    }
    
    DrawTextEx(globalFont, currentDisplayedText, textPos, 24, 1, WHITE);
    // --- [THÊM MỚI TỪ ĐÂY] ---
    // Nếu đang ở khung đầu tiên (nền đen cảnh báo)
    if (currentStoryIndex == 0) {
        const char* warningTitle = "LƯU Ý";
        // Chữ to cỡ 40
        Vector2 titleSize = MeasureTextEx(globalFont, warningTitle, 40, 1); 
        Vector2 titlePos = {
            (SCREEN_WIDTH - titleSize.x) / 2.0f, // Căn giữa màn hình
            textPos.y - 60.0f // Đẩy lên cao hơn đoạn text chính 60 pixel
        };
        // Vẽ chữ màu Đỏ
        DrawTextEx(globalFont, warningTitle, titlePos, 40, 1, RED); 
    }
    
    // Icon nhấp nháy góc dưới báo hiệu có thể click
    if (storyState == 2) {
        float blink = (sinf((float)GetTime() * 8.0f) + 1.0f) * 0.5f;
        DrawTextEx(globalFont, "▼", (Vector2){SCREEN_WIDTH/2.0f - 10, SCREEN_HEIGHT - 40}, 20, 1, Fade(WHITE, blink));
    }
    
    // 3. Vẽ Lớp Fade chuyển cảnh
    if (storyAlpha > 0.0f) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, storyAlpha));
    }
}

void StoryCutscene_Unload(void) {
    for (int i = 0; i < 7; i++) {
        if (storyFrames[i].hasBg) {
            UnloadTexture(storyFrames[i].bg);
            storyFrames[i].hasBg = false;
        }
    }
}