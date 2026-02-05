// FILE: src/renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "player.h"
#include "map.h"
#include "npc.h"
// Khởi tạo hệ thống vẽ
void InitRenderer();

// Đăng ký Player vào danh sách cần vẽ
void Render_AddPlayer(Player *player);

// Đăng ký NPC vào danh sách cần vẽ
void Render_AddNpc(Npc *npc);

// Đăng ký Vật thể tĩnh (Props) vào danh sách
void Render_AddProp(GameProp *prop);

// Hàm quan trọng nhất: Sắp xếp vị trí trên dưới và Vẽ tất cả ra màn hình
void Render_DrawAll();

// Dọn dẹp bộ nhớ sau khi vẽ xong (Reset danh sách cho frame sau)
void Render_Clear();

#endif