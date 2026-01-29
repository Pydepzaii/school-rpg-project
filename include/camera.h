// FILE: src/camera.h
#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "raylib.h"
#include "player.h" // Cần để biết vị trí nhân vật
#include "map.h"    // Cần để biết kích thước map (để không soi ra ngoài map)

// Khai báo camera toàn cục để dùng ở main
extern Camera2D gameCamera;

void Camera_Init();
void Camera_Update(Player *player, GameMap *map);

#endif