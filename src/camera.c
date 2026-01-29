// FILE: src/camera.c
#include "camera.h"
#include "settings.h"

Camera2D gameCamera = { 0 };

// Biến trạng thái: true = Bám nhân vật | false = Nhìn toàn map
static bool isCameraActive = true; 

void Camera_Init() {
    gameCamera.target = (Vector2){ 0, 0 };
    // [GIẢI THÍCH]: Offset giúp nhân vật luôn nằm giữa màn hình thay vì góc trên trái.
    gameCamera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }; 
    gameCamera.rotation = 0.0f;
    gameCamera.zoom = 2.0f; 
}

void Camera_Update(Player *player, GameMap *map) {
    // 1. Xử lý phím tắt bật/tắt Camera (Phím Y)
    // [CÓ THỂ THỪA]: Tính năng này thường dùng để debug, khi game release có thể xóa để người chơi không vô tình bấm nhầm.
    if (IsKeyPressed(KEY_Y)) {
        isCameraActive = !isCameraActive;
    }

    // Lấy kích thước thật của Map
    float mapWorldWidth = map->texture.width * map->scale;
    float mapWorldHeight = map->texture.height * map->scale;

    if (isCameraActive) {
        // --- CHẾ ĐỘ 1: RPG (Bám theo nhân vật) ---
        gameCamera.zoom = 2.0f; // Trả về độ zoom chuẩn

        // Logic bám nhân vật
        gameCamera.target = (Vector2){ 
            player->position.x + player->frameRec.width/2, 
            player->position.y + player->frameRec.height/2 
        };

        // Logic Kẹp Map (Clamp) - Giữ nguyên từ code trước
        // [GIẢI THÍCH]: Các dòng dưới đây tính toán xem camera có đang chạm mép map chưa.
        // Nếu chạm mép, nó sẽ ngừng di chuyển để không lộ phần đen ngoài map.
        float visibleWidth = SCREEN_WIDTH / gameCamera.zoom;
        float visibleHeight = SCREEN_HEIGHT / gameCamera.zoom;
        float halfVisW = visibleWidth / 2.0f;
        float halfVisH = visibleHeight / 2.0f;

        // Kẹp trục X
        if (mapWorldWidth > visibleWidth) {
            if (gameCamera.target.x < halfVisW) gameCamera.target.x = halfVisW;
            if (gameCamera.target.x > mapWorldWidth - halfVisW) gameCamera.target.x = mapWorldWidth - halfVisW;
        } else {
            gameCamera.target.x = mapWorldWidth / 2.0f;
        }

        // Kẹp trục Y
        if (mapWorldHeight > visibleHeight) {
            if (gameCamera.target.y < halfVisH) gameCamera.target.y = halfVisH;
            if (gameCamera.target.y > mapWorldHeight - halfVisH) gameCamera.target.y = mapWorldHeight - halfVisH;
        } else {
            gameCamera.target.y = mapWorldHeight / 2.0f;
        }

    } else {
        // --- CHẾ ĐỘ 2: TOÀN CẢNH (Overview) ---
        // [GIẢI THÍCH]: Chế độ này zoom out hết cỡ để thấy toàn bộ map. 
        
        // Căn target vào chính giữa tâm bản đồ
        gameCamera.target = (Vector2){ mapWorldWidth / 2.0f, mapWorldHeight / 2.0f };

        // Tính toán độ Zoom để nhét vừa cả cái map vào màn hình
        // Công thức: Lấy màn hình chia cho kích thước map -> Ra tỉ lệ thu nhỏ
        float scaleX = (float)SCREEN_WIDTH / mapWorldWidth;
        float scaleY = (float)SCREEN_HEIGHT / mapWorldHeight;

        // Chọn tỉ lệ nhỏ hơn để đảm bảo map lọt thỏm vào trong màn hình (không bị cắt)
        // (Thêm 0.9f để chừa ra chút viền cho đẹp)
        gameCamera.zoom = (scaleX < scaleY ? scaleX : scaleY) * 0.95f;
    }
}