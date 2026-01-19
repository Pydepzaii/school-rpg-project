#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include "settings.h" // Để lấy MAX_MAP_WALLS

typedef struct {
    int currentMapID;  // Đang ở map số mấy
    Texture2D texture; 
    float scale;       
    
    // --- HỆ THỐNG VA CHẠM (WALLS) ---
    // Danh sách các hình chữ nhật vô hình ngăn cản người chơi
    Rectangle walls[MAX_MAP_WALLS]; 
    int wallCount; // Số lượng tường hiện có trong map này
} GameMap;

// Hàm LoadMap giờ nhận vào ID để biết load map nào
void LoadMap(GameMap *map, int mapID);     
void DrawMap(GameMap *map);     
void DrawMapDebug(GameMap *map); // Hàm vẽ viền đỏ để debug tường
void UnloadMap(GameMap *map);   

#endif