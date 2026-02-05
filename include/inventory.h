#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"
#include <stdbool.h>

// Định nghĩa tối đa slot túi đồ
#define MAX_INVENTORY_SLOTS 40 
#define SLOTS_PER_PAGE 20
// Danh sách ID các vật phẩm (Thêm thoải mái vào đây)
typedef enum {
    ITEM_NONE = 0,
    ITEM_KEY_ALPHA,
    ITEM_KEY_VO,
    ITEM_POTION_HP,
    ITEM_COUNT // Luôn để cuối để đếm số lượng loại item
} ItemID;

// Định nghĩa thông tin gốc của 1 Item (Dữ liệu tĩnh)
typedef struct {
    int id;
    const char* name;
    const char* description;
    // Sau này thay Color bằng Texture2D icon;
    Color debugColor; 
} ItemDef;

// Định nghĩa Slot trong túi đồ (Dữ liệu động)
typedef struct {
    ItemID itemID;
    int quantity;
} InventorySlot;

// --- QUẢN LÝ ---
void Inventory_Init();
void Inventory_Update(); // Xử lý bật tắt túi đồ
void Inventory_Draw();   // Vẽ giao diện túi đồ (Lớp UI)

// --- LOGIC ---
bool Inventory_AddItem(ItemID id, int quantity);
bool Inventory_RemoveItem(ItemID id, int quantity);
bool Inventory_HasItem(ItemID id); // Dùng để check Quest
bool Inventory_IsActive(); // Check xem đang mở túi không

// --- DEBUG HELPER ---
// Hàm này giúp Debug Tool lấy được info item để in ra console nếu cần
const char* Inventory_GetItemName(ItemID id);
void Inventory_UseItem(int slotIndex);

#endif