#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include "settings.h" // Cần file này để lấy số MAX_MAP_WALLS
#define MAX_MAP_PROPS 100
typedef struct {
    Rectangle sourceRec; 
    Vector2 position;    
    float originY;      
    Texture2D *texRef;  
} GameProp;
typedef struct {
    int currentMapID;  // ID của map hiện tại (để biết đang ở đâu)
    Texture2D texture; // Ảnh nền của map
    float scale;       // Tỉ lệ phóng to map (VD: 2.0f là to gấp đôi)
    
    // --- HỆ THỐNG VA CHẠM (COLLISION) ---
    // Mảng chứa các hình chữ nhật vô hình dùng làm tường
    Rectangle walls[MAX_MAP_WALLS]; 
    int wallCount; // Đếm số lượng tường thực tế đang dùng trong map này
    Texture2D layerTexture;
    GameProp props[MAX_MAP_PROPS];
    int propCount;
} GameMap;

// Hàm chuyển map: Tự động xóa map cũ và nạp map mới theo ID
void LoadMap(GameMap *map, int mapID);     
void DrawMap(GameMap *map);     
void DrawMapDebug(GameMap *map); // Hàm chỉ dùng khi dev: Vẽ viền đỏ để thấy tường
void UnloadMap(GameMap *map);    // Giải phóng bộ nhớ ảnh

#endif