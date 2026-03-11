// FILE: include/inventory.h
#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"
#include <stdbool.h>

// --- CẤU HÌNH ---
#define MAX_INVENTORY_SLOTS 40 
#define SLOTS_PER_PAGE 20

// CẤU HÌNH CHO ITEM RỚT DƯỚI ĐẤT
#define ITEM_GRID_SIZE 16       // Kích thước 1 ô item trên ảnh
#define MAX_ITEMS_ON_MAP 100    // Số lượng rác/đồ rớt tối đa trên map
#define MAX_NOTIFICATIONS 10    // Số lượng chữ bay lên tối đa

typedef enum {
    ITEM_NONE = 0,
    ITEM_KEY_ALPHA,
    ITEM_KEY_BETA,
    ITEM_KEY_DELTA,
    ITEM_BOOK_1,
    ITEM_BOOK_2,
    ITEM_BOOK_3,
    ITEM_BOOK_4,
    ITEM_BOOK_5,
    ITEM_BOOK_6,
    ITEM_POTION_HP,
    ITEM_1,
    ITEM_2,
    ITEM_NOTE_PAPER,
    ITEM_KEY_SPECIAL,
    ITEM_COUNT 
} ItemID;

typedef struct {
    int id;
    const char* name;
    const char* description;
    Rectangle sourceRec; // [ĐÃ SỬA] Thay Color debugColor bằng Tọa độ cắt ảnh
} ItemDef;

typedef struct {
    ItemID itemID;
    int quantity;
} InventorySlot;

// --- QUẢN LÝ ---
void Inventory_Init();
void Inventory_Unload(); // [MỚI] Giải phóng ảnh khi tắt game
void Inventory_Update(); // Xử lý bật tắt bằng phím B
void Inventory_Draw();   // Vẽ Grid Item (Không vẽ nút nữa)

// --- [MỚI] QUẢN LÝ RỚT & NHẶT ĐỒ ---
void Inventory_SpawnItem(ItemID id, Vector2 pos, int mapID);
// Dùng Rectangle thay cho Player để không phải include chéo player.h
void Inventory_UpdateItemsOnMap(Rectangle playerHitbox, int currentMapID);
void Inventory_DrawItemsOnMap(int currentMapID);
void Inventory_DrawNotifications();
bool Inventory_IsPickingUpAnimActive();
// --- LOGIC ---
bool Inventory_AddItem(ItemID id, int quantity);
bool Inventory_RemoveItem(ItemID id, int quantity);
bool Inventory_HasItem(ItemID id); 
bool Inventory_IsActive(); 

// --- [NEW] CÁC HÀM EXTERNAL ĐỂ MENU SYSTEM GỌI ---
void Inventory_NextPage();
void Inventory_PrevPage();
void Inventory_UseSelected();
void Inventory_Unselect();
bool Inventory_HasSelectedSlot();
int Inventory_GetCurrentPage();
int Inventory_GetMaxPages();
const char* Inventory_GetItemName(ItemID id);
// --- [MỚI] HỆ THỐNG HIỂN THỊ BẢN ĐỒ BÍ MẬT ---
extern bool isShowingSecretMap;
void Inventory_ShowSecretMap();
void Inventory_DrawSecretMap();
#endif