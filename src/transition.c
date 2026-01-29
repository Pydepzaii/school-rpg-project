// FILE: src/transition.c
#include "transition.h"
#include "audio_manager.h"
#include "camera.h"
#include "settings.h"
#include "menu_system.h"

// --- TRẠNG THÁI ---
typedef enum {
    TRANS_OFF = 0, // Không làm gì cả
    TRANS_FADE_IN, // Đang tối dần (Màn đen hiện lên)
    TRANS_WAIT,    // Đen thui (Lúc load map)
    TRANS_FADE_OUT // Đang sáng dần (Màn đen biến mất)
} TransState;

static TransState currentState = TRANS_OFF;
static float alpha = 0.0f;       // Độ mờ: 0.0 (Trong suốt) -> 1.0 (Đen đặc)
static float fadeSpeed = 3.0f;   // Tốc độ chuyển cảnh (Càng cao càng nhanh)
static float waitTimer = 0.0f;   //Thời gian chờ trong lúc fadein
// Dữ liệu tạm để nhớ mình định đi đâu
static int targetMapID = -1;
static Vector2 targetPlayerPos = {0};
static bool isExitingGame = false;

// [GIẢI THÍCH]: Hàm kích hoạt quá trình chuyển cảnh.
void Transition_StartToMap(int mapID, Vector2 newPlayerPos) {
    if (currentState != TRANS_OFF) return; // Đang chuyển rồi thì thôi
    isExitingGame = false;
    targetMapID = mapID;
    targetPlayerPos = newPlayerPos;
    currentState = TRANS_FADE_IN;
    alpha = 0.0f;
    waitTimer = 0.0f;
}

// Hàm bắt đầu thoát game
void Transition_StartExit() {
    if (currentState != TRANS_OFF) return;

    isExitingGame = true; // Đánh dấu là đang thoát
    currentState = TRANS_FADE_IN;
    alpha = 0.0f;
    waitTimer = 0.0f;
    
    // Có thể fade nhạc nhỏ dần ở đây nếu muốn (nâng cao)
}

void Transition_Update(GameMap *currentMap, Player *player) {
    if (currentState == TRANS_OFF) return;

    float dt = GetFrameTime();

    switch (currentState) {
        case TRANS_FADE_IN:
            alpha += fadeSpeed * dt;
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                currentState = TRANS_WAIT; // Đen thui rồi, chuẩn bị load map
                waitTimer = 0.0f;
            }
            break;

        case TRANS_WAIT:
            //  fix tắt hình đột ngột khi bấm exit(thẩm mĩ thôi hẹ hẹ)
            if (isExitingGame) {
                // Đợi 1 chút xíu cho đen hẳn rồi thoát
                waitTimer += dt;
                if (waitTimer > 0.5f) { // Giữ đen 0.5s cho "ngầu"
                    Menu_RequestClose(); // Gửi lệnh đóng cửa sổ
                }
                return; // Không làm gì thêm, chờ vòng lặp main xử lý thoát
            }
            // --- THỜI KHẮC QUAN TRỌNG: LOAD MAP MỚI ---
            // [GIẢI THÍCH]: Chỉ load map khi màn hình đã tối đen hoàn toàn để người chơi không thấy quá trình load giật lag.
            if (targetMapID != -1) {
                LoadMap(currentMap, targetMapID);
                Audio_PlayMusicForMap(targetMapID);
                
                // Cập nhật vị trí nhân vật (nếu có yêu cầu)
                // (0,0) là giá trị mặc định nếu ta không muốn set vị trí
                if (targetPlayerPos.x != 0 || targetPlayerPos.y != 0) {
                    player->position = targetPlayerPos;
                }
                Camera_Update(player, currentMap);
                Menu_SwitchTo(MENU_NONE);// Âm thầm tắt menu
                targetMapID = -1; // Reset nhiệm vụ
            }
            // 2. GIỮ MÀN ĐEN THÊM MỘT CHÚT (Delay an toàn)
            // Giữ màn đen khoảng 0.2 giây để tránh hiện tượng "nháy hình"
            waitTimer += dt;
            if (waitTimer > 0.2f) {
                currentState = TRANS_FADE_OUT; // Hết giờ chờ -> Bắt đầu sáng lên
            }
            break;

        case TRANS_FADE_OUT:
            alpha -= fadeSpeed * dt;
            if (alpha <= 0.0f) {
                alpha = 0.0f;
                currentState = TRANS_OFF; // Kết thúc chuyển cảnh
            }
            break;
            
        default: break;
    }
}

void Transition_Draw() {
    if (currentState == TRANS_OFF) return;

    // Vẽ một hình chữ nhật đen trùm kín màn hình
    // Lưu ý: Vẽ đè lên tất cả nên phải gọi cuối cùng trong hàm vẽ
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
}

bool Transition_IsActive() {
    return currentState != TRANS_OFF;
}