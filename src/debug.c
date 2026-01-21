// FILE: src/debug.c
#include "debug.h"
#include <stdio.h> 

// --- BIẾN NỘI BỘ ---
static Vector2 devStartPos = {0}; 
static bool devIsDragging = false; 
static bool showDebugWalls = false; // Mặc định tắt

void Debug_UpdateAndDraw(GameMap *map) {
    // 1. PHÍM 0: BẬT / TẮT CHẾ ĐỘ DEBUG
    if (IsKeyPressed(KEY_ZERO)) {
        showDebugWalls = !showDebugWalls; 
    }

    // CHỈ CHẠY KHI ĐÃ BẬT DEBUG
    if (showDebugWalls) {
        
        // A. HIỂN THỊ TƯỜNG CŨ (Gọi hàm từ map.c - Màu ĐỎ)
        DrawMapDebug(map); 
        DrawText("MODE: DEBUG (Tuong cu = DO | Dang ve = XANH)", 10, 70, 20, RED);

        // B. TOOL KÉO CHUỘT (VẼ TƯỜNG MỚI)
        
        // Bắt đầu kéo
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            devStartPos = GetMousePosition();
            devIsDragging = true;
        }

        // Đang kéo -> Vẽ hình màu XANH (BLUE)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && devIsDragging) {
            Vector2 currentPos = GetMousePosition();
            float width = currentPos.x - devStartPos.x;
            float height = currentPos.y - devStartPos.y;
            
            Rectangle rec = { devStartPos.x, devStartPos.y, width, height };

            // --- TÔ MÀU XANH ---
            DrawRectangleRec(rec, Fade(BLUE, 0.5f)); // Nền xanh mờ
            DrawRectangleLinesEx(rec, 3.0f, BLUE);   // Viền xanh đậm
            
            // Hiện kích thước màu xanh
            DrawText(TextFormat("W:%.0f H:%.0f", width, height), currentPos.x + 10, currentPos.y, 20, BLUE);
        }

        // Thả chuột -> In code
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && devIsDragging) {
            devIsDragging = false;
            Vector2 endPos = GetMousePosition();
            
            float x = devStartPos.x;
            float y = devStartPos.y;
            float w = endPos.x - devStartPos.x;
            float h = endPos.y - devStartPos.y;

            if (w < 0) { x += w; w = -w; }
            if (h < 0) { y += h; h = -h; }

            printf("\n--- COPY VAO map.c ---\n");
            printf("map->walls[map->wallCount++] = (Rectangle){ %.0f, %.0f, %.0f, %.0f };\n", x, y, w, h);
            printf("----------------------\n");
        }
    } 
    else {
        DrawText("Nhan phim 0 de hien thi tuong va cham", 10, 70, 20, DARKGRAY);
    }
}