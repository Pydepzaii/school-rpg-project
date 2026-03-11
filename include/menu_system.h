// FILE: src/menu_system.h
#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include "raylib.h"

// Các loại Menu trong game
typedef enum {
    MENU_NONE = 0,      
    MENU_TITLE,  //tiêu đề
    MENU_CHARACTER_SELECT, //chọn nhân vật
    MENU_CHARACTER_PROFILE,      // thông tin sau khi chọn nhân vật
    MENU_PAUSE,        //tạm dừng 
    MENU_UPGRADE,       //menu nâng cấp
    MENU_SETTINGS,       // cài đặt
    MENU_INVENTORY,      //menu túi đồ
    MENU_INFO           //Menu dạng Sách mở (Cho Devlog & Credit)

} MenuType;

// [NEW] Định danh hành động của nút (Để Debug Tool chọn)
typedef enum {
    ACT_NONE = -1,
    ACT_START_GAME = 0,
    ACT_CONTINUE,
    ACT_OPEN_SETTINGS,
    ACT_EXIT_GAME,
    ACT_RESUME,
    ACT_SAVE_GAME,
    ACT_QUIT_TO_TITLE,
    ACT_OPEN_INVENTORY,

    // --- INVENTORY ACTIONS ---
    ACT_INV_PREV_PAGE = 10,
    ACT_INV_NEXT_PAGE,
    ACT_INV_USE,
    ACT_INV_UNSELECT,
    ACT_INV_CLOSE,
    
    // [NEW] ACTION CHO SETTINGS
    ACT_SET_MASTER_VOL, // Chỉnh âm lượng tổng
    ACT_SET_MUSIC_VOL,  // Chỉnh nhạc nền
    ACT_SET_SFX_VOL,    // Chỉnh âm thanh
   ACT_TOGGLE_MUTE,    // Bật tắt tiếng
    ACT_TOGGLE_MUSIC_MUTE, // [NEW] Bật tắt nhạc nền
    ACT_TOGGLE_SFX_MUTE,   // [NEW] Bật tắt tiếng động
    ACT_TOGGLE_FULLSCREEN, // Bật tắt toàn màn hình
     // --- CHAR SELECT ACTIONS ---
    ACT_SEL_CLASS_1,
    ACT_SEL_CLASS_2,
    ACT_SEL_CLASS_3,
    ACT_SEL_CLASS_4,
    ACT_CONFIRM_CHAR,

    // --- CHAR PROFILE ACTIONS ---
    ACT_PROFILE_START_GAME, // Nút Next (Vào game thật)
    ACT_PROFILE_BACK,       // Nút Back (Quay lại chọn nhân vật)
    //menu info 
    ACT_OPEN_INFO,      // Nút mở bảng Info từ Menu chính
    ACT_CLOSE_INFO,
    ACT_TAB_DEVLOG,     // Nút Icon Tab: Xem Devlog
    ACT_TAB_CREDIT,     // Nút Icon Tab: Xem Credit

    // --- [NEW] CÁC NÚT CHO HỆ THỐNG SÁCH ---
    ACT_INFO_NEXT_PAGE, // Lật sang trang chữ tiếp theo
    ACT_INFO_PREV_PAGE, // Lật lùi trang chữ trước đó
    ACT_INFO_BACK_INDEX,// Gập trang trái lại để chọn Tab khác

    ACT_COUNT // Để đếm số lượng hành động
} ButtonAction;

// [FIX] CHUYỂN ControlType LÊN TRƯỚC MenuButton ĐỂ TRÌNH BIÊN DỊCH HIỂU
// [NEW] Loại điều khiển
typedef enum {
    CTRL_BUTTON = 0, // Nút bấm thường
    CTRL_SLIDER,     // Thanh trượt (Volume)
    CTRL_TOGGLE,      // Công tắc (Checkbox)
    CTRL_CUSTOM_BTN, //Nút bấm chính thức
    CTRL_ICON_BTN
} ControlType;

// [NEW] Cấu trúc nút động
typedef struct {
    Rectangle rect;
    int actionID;       // ID hành động (ButtonAction)
    bool isInvisible;   // True = Nút tàng hình (chỉ có hitbox)
    char label[32];     // Tên hiển thị (hoặc ghi chú cho nút tàng hình)
    
    //loại điều khiển
    ControlType type;   // Loại: Nút, Slider hay Toggle?
    float value;
    
    // Biến runtime (không lưu file)
    bool isHover;
    bool isClicked;
} MenuButton;

#define MAX_MENU_BUTTONS 20
extern MenuButton currentButtons[MAX_MENU_BUTTONS];
extern int currentBtnCount;

extern MenuType currentMenu;

void Menu_Init();
void Menu_Update(); 
void Menu_Draw();   
void Menu_SwitchTo(MenuType type); 
bool Menu_IsActive();
void Menu_Shutdown();

// [NEW] Hàm kiểm tra xem người chơi có bấm nút THOÁT không
bool Menu_ShouldCloseGame(); 
void Menu_RequestClose();

// [NEW] Hàm hỗ trợ Debug & Data
void Menu_LoadLayout(MenuType type);
void Menu_SaveLayout(); // Lưu đè lại file menus.txt
void Menu_SetDebugMode(bool enabled); // Bật tắt chế độ hiện nút tàng hình
const char* GetActionName(int actionID); // Lấy tên hành động dạng chuỗi
extern Texture2D texUIBook;
#endif