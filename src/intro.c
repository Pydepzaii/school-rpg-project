// FILE: src/intro.c
#include "intro.h"
#include "audio_manager.h" // [THÊM] Import Audio Manager
#include <stdio.h>
#include <stdlib.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

static plm_t *plm = NULL;
static Texture2D videoTexture = { 0 };
static uint8_t *rgbBuffer = NULL;
static bool isInitialized = false;

// [ĐÃ XÓA] static Music introMusic; -> Không cần nữa vì Audio Manager quản lý

// Callback xử lý hình ảnh (Giữ nguyên)
void OnVideoFrame(plm_t *plm, plm_frame_t *frame, void *user) {
    uint8_t *buffer = (uint8_t *)user;
    plm_frame_to_rgb(frame, buffer, frame->width * 3);
}

void InitIntro(const char* fileName) {
    plm = plm_create_with_filename(fileName);
    if (!plm) {
        printf("LOI: Khong tim thay file video: %s\n", fileName);
        isInitialized = false;
        return;
    }
    
    // Tắt audio của video
    plm_set_audio_enabled(plm, 0);

    // Setup bộ đệm hình ảnh
    int w = plm_get_width(plm);
    int h = plm_get_height(plm);
    rgbBuffer = (uint8_t *)malloc(w * h * 3);
    plm_set_video_decode_callback(plm, OnVideoFrame, rgbBuffer);

    Image image = GenImageColor(w, h, BLACK);
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8; 
    videoTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    // [THAY ĐỔI] Gọi Audio Manager để phát nhạc
    Audio_PlayMusic(MUSIC_INTRO);
    
    isInitialized = true;
}

bool UpdateIntro() {
    if (!isInitialized) return true;

    if (GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return true; 
    }

    // [ĐÃ XÓA] UpdateMusicStream(introMusic); -> Main Loop sẽ làm việc này

    plm_decode(plm, GetFrameTime());
    UpdateTexture(videoTexture, rgbBuffer);

    if (plm_has_ended(plm)) return true;

    return false;
}

void DrawIntro() {
    if (!isInitialized) return;

    Rectangle source = { 0, 0, (float)videoTexture.width, (float)videoTexture.height };
    Rectangle dest = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
    
    DrawTexturePro(videoTexture, source, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    DrawText("Bam phim bat ky de bo qua", 20, GetScreenHeight() - 40, 20, LIGHTGRAY);
}

void UnloadIntro() {
    if (isInitialized) {
        // [THAY ĐỔI] Dừng nhạc qua Audio Manager
        Audio_StopMusic(MUSIC_INTRO);

        plm_destroy(plm);
        UnloadTexture(videoTexture);
        if (rgbBuffer) free(rgbBuffer);
        
        isInitialized = false;
    }
}