// FILE: include/info.h
#ifndef INFO_H
#define INFO_H

#include "raylib.h"

// Định nghĩa 2 Tab nội dung
typedef enum {
    TAB_DEVLOG = 0,
    TAB_CREDIT
} InfoTab;

void Info_Init(void);               // Load file text vào RAM và xử lý Word Wrap
void Info_Shutdown(void);           // Giải phóng RAM
void Info_SetTab(InfoTab tab);      // Chuyển Tab 
InfoTab Info_GetCurrentTab(void);   // [NEW] Lấy Tab hiện tại

void Info_NextPage(void);           // Lật tới
void Info_PrevPage(void);           // Lật lùi

void Info_Update(void);             // Dùng cho logic Update nếu có
void Info_Draw(void);               // Vẽ chữ lên trang sách
void Info_StartReveal(void);
#endif