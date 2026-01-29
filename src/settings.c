// FILE: src/settings.c
#include "settings.h"
#include <stdio.h>
#include "raylib.h"

// Biến nội bộ: Màn hình ảo (Render Texture)
// [GIẢI THÍCH]: Game sẽ vẽ mọi thứ lên tấm ảnh này trước (độ phân giải thấp 800x600).
// Sau đó tấm ảnh này mới được phóng to ra màn hình thật.
static RenderTexture2D virtualScreen; 
static bool isScalingInit = false;

// Hàm tìm số nhỏ nhất để tính tỉ lệ
static float Min(float a, float b) { return (a < b) ? a : b; }

void ToggleGameFullscreen(void) {
    int monitor = GetCurrentMonitor();
    if (!IsWindowFullscreen()) {
        int monitorW = GetMonitorWidth(monitor);
        int monitorH = GetMonitorHeight(monitor);
        SetWindowSize(monitorW, monitorH);
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        SetWindowPosition((GetMonitorWidth(monitor) - SCREEN_WIDTH) / 2, (GetMonitorHeight(monitor) - SCREEN_HEIGHT) / 2);
    }
}

// 1. Khởi tạo màn hình ảo
void InitScaling(void) {
    virtualScreen = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(virtualScreen.texture, TEXTURE_FILTER_POINT); // Quan trọng: Giữ nét Pixel Art
    isScalingInit = true;
}

// 2. Bắt đầu chế độ vẽ ảo
void BeginScaling(void) {
    if (!isScalingInit) return;
    BeginTextureMode(virtualScreen);
}

// 3. Kết thúc vẽ ảo & Phóng to ra màn hình thật
void EndScaling(void) {
    if (!isScalingInit) return;
    
    EndTextureMode(); // Dừng vẽ vào màn hình ảo

    // Xóa màn hình thật thành màu đen (để tạo viền đen nếu tỉ lệ không khớp)
    ClearBackground(BLACK); 

    // Tính toán tỉ lệ phóng to (Scale)
    // [GIẢI THÍCH]: Chọn tỉ lệ sao cho hình không bị méo (giữ aspect ratio).
    float scale = Min((float)GetScreenWidth()/SCREEN_WIDTH, (float)GetScreenHeight()/SCREEN_HEIGHT);

    // Vẽ màn hình ảo đã phóng to ra giữa màn hình thật
    Rectangle sourceRec = { 0.0f, 0.0f, (float)virtualScreen.texture.width, -(float)virtualScreen.texture.height };
    Rectangle destRec = {
        (GetScreenWidth() - ((float)SCREEN_WIDTH * scale)) * 0.5f,
        (GetScreenHeight() - ((float)SCREEN_HEIGHT * scale)) * 0.5f,
        (float)SCREEN_WIDTH * scale,
        (float)SCREEN_HEIGHT * scale
    };
    
    DrawTexturePro(virtualScreen.texture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

// 4. Dọn dẹp bộ nhớ
void UnloadScaling(void) {
    if (isScalingInit) UnloadRenderTexture(virtualScreen);
}

// [MỚI] Hàm lấy tọa độ chuột đã quy đổi sang màn hình ảo
// [GIẢI THÍCH]: Vì hình ảnh bị scale và dịch chuyển, tọa độ chuột thật sẽ sai lệch so với tọa độ trong game.
// Hàm này tính toán ngược lại để lấy đúng tọa độ chuột trong không gian 800x600.
Vector2 GetVirtualMousePos(void) {
    float scale = Min((float)GetScreenWidth()/SCREEN_WIDTH, (float)GetScreenHeight()/SCREEN_HEIGHT);
    
    // Tính toán phần viền đen (offset) đang vẽ
    Vector2 offset = {
        (GetScreenWidth() - (SCREEN_WIDTH * scale)) * 0.5f,
        (GetScreenHeight() - (SCREEN_HEIGHT * scale)) * 0.5f
    };

    Vector2 mousePos = GetMousePosition();
    
    // Công thức đảo ngược: (Chuột thật - Viền đen) / Tỉ lệ
    Vector2 virtualMouse = {
        (mousePos.x - offset.x) / scale,
        (mousePos.y - offset.y) / scale
    };
    
    // Kẹp giá trị chuột (Clamp) để nó không nhận giá trị ngoài vùng game (nếu cần)
    // [CÓ THỂ THỪA]: Đoạn code bị comment này có thể xóa nếu không dùng tới.
    // if (virtualMouse.x < 0) virtualMouse.x = 0;
    // ...

    return virtualMouse;
}