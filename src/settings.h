// FILE: src/settings.h
#ifndef SETTINGS_H
#define SETTINGS_H

// --- CẤU HÌNH MÀN HÌNH ---
#define SCREEN_WIDTH 800  // Chiều rộng cửa sổ game
#define SCREEN_HEIGHT 450 // Chiều cao cửa sổ game
#define GAME_TITLE "RPG Game - Scalable System" // Tên hiển thị trên thanh tiêu đề
#define FPS 60            // Tốc độ khung hình (60 là mượt chuẩn)

// --- GIỚI HẠN HỆ THỐNG (MEMORY LIMITS) ---
// Lưu ý: Nếu game báo lỗi "Segmentation Fault", hãy kiểm tra xem có vượt quá số này không
#define MAX_MAPS 20       // Game chỉ chứa tối đa 20 map khác nhau
#define MAX_MAP_WALLS 100 // Mỗi map chỉ được vẽ tối đa 100 bức tường
#define MAX_NPCS 50       // Tối đa 50 NPC xuất hiện trong game cùng lúc

// --- ID CÁC MAP (DANH SÁCH BẢN ĐỒ) ---
// Thêm map mới thì thêm số tiếp theo vào đây
#define MAP_THU_VIEN   0
#define MAP_NHA_AN     1
#define MAP_SAN_TRUONG 2

#endif