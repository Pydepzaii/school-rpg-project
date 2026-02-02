// FILE: src/menu_system.h
#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include "raylib.h"

// Các loại Menu trong game
typedef enum {
    MENU_NONE = 0,      
    MENU_TITLE,  
    MENU_CHARACTER_SELECT,       
    MENU_PAUSE,         
    MENU_UPGRADE,       
    MENU_SETTINGS       
} MenuType;

extern MenuType currentMenu;

void Menu_Init();
void Menu_Update(); 
void Menu_Draw();   
void Menu_SwitchTo(MenuType type); 
void Menu_Shutdown();

// [NEW] Hàm kiểm tra xem người chơi có bấm nút THOÁT không
// Hàm này giúp Main loop biết khi nào cần dừng vòng lặp an toàn
bool Menu_ShouldCloseGame(); 
void Menu_RequestClose();
#endif