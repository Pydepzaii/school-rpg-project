// FILE: src/intro.c
#include "intro.h"
#include "audio_manager.h" 
#include <stdio.h>
#include <stdlib.h>

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