#include "inventory.h"
#include "settings.h" 
#include "audio_manager.h"
#include "gameplay.h" // Để dùng các hàm hồi máu, v.v.
#include "debug.h"
#include <stdio.h>

// --- CẤU HÌNH ---
#define SLOTS_PER_PAGE 20 // 4 hàng x 5 cột

// --- DỮ LIỆU TĨNH (DATABASE ITEM) ---
static ItemDef itemDatabase[ITEM_COUNT] = {
    { ITEM_NONE, "Empty", "", BLANK },
    { ITEM_KEY_ALPHA, "Key Alpha", "Chia khoa mo cua toa Alpha", GOLD },
    { ITEM_KEY_VO, "Key Nha Vo", "Chia khoa vao san tap vo", ORANGE },
    { ITEM_POTION_HP, "Nuoc Tang Luc", "Hoi 50 HP", RED }
};

// --- TRẠNG THÁI INVENTORY ---
static InventorySlot inventory[MAX_INVENTORY_SLOTS];
static bool isInventoryOpen = false;
static Rectangle uiBounds; // Khung túi đồ

// --- TƯƠNG TÁC (NEW) ---
static int selectedSlot = -1; // -1: Chưa chọn ô nào
static int currentPage = 0;   // Trang hiện tại (0, 1, 2...)
static int maxPages = 0;      // Tổng số trang

// --- HÀM VẼ NÚT (Đã có sẵn từ code của bạn) ---
static bool DrawInvButton(const char* text, Rectangle rec) {
    bool clicked = false;
    Vector2 mousePoint = GetVirtualMousePos(); 
    bool isHover = CheckCollisionPointRec(mousePoint, rec);

    // Vẽ nền nút
    Color colorBg = isHover ? DARKBLUE : BLACK;
    Color colorBorder = isHover ? YELLOW : WHITE;
    
    DrawRectangleRec(rec, Fade(colorBg, 0.8f));
    DrawRectangleLinesEx(rec, 2.0f, colorBorder);
    
    // Căn giữa chữ
    int textW = MeasureText(text, 20);
    DrawText(text, (int)(rec.x + (rec.width - textW)/2), (int)(rec.y + (rec.height - 20)/2), 20, colorBorder);

    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Audio_PlaySoundEffect(SFX_UI_CLICK); 
        clicked = true;
    }
    if (IsMenuDebugActive()) {
        DrawRectangleLinesEx(rec, 2.0f, RED);       // Viền đỏ
        DrawRectangleRec(rec, Fade(RED, 0.8f));     // Nền đỏ đậm
        DrawText(text, (int)rec.x, (int)rec.y - 20, 10, RED); // Tên nút
    }
    return clicked;
}

// --- KHỞI TẠO (Đã sửa lại kích thước và tính trang) ---
void Inventory_Init() {
    // 1. Xóa sạch túi đồ
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        inventory[i].itemID = ITEM_NONE;
        inventory[i].quantity = 0;
    }
    
    // 2. Tính toán khung UI (Tăng chiều rộng lên 600 để chứa bảng thông tin bên phải)
    float w = 600; 
    float h = 450;
    uiBounds = (Rectangle){ (SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2, w, h };
    
    // 3. Tính tổng số trang
    maxPages = (MAX_INVENTORY_SLOTS + SLOTS_PER_PAGE - 1) / SLOTS_PER_PAGE;
    if (maxPages < 1) maxPages = 1;

    printf(">> [INVENTORY] System Initialized. Max Pages: %d\n", maxPages);
    
    // TEST: Tặng đồ
    Inventory_AddItem(ITEM_POTION_HP, 5);
    Inventory_AddItem(ITEM_KEY_ALPHA, 1);
}

void Inventory_Update() {
    if (IsKeyPressed(KEY_B)) {
        isInventoryOpen = !isInventoryOpen;
        if (isInventoryOpen) {
            Audio_PlaySoundEffect(SFX_UI_CLICK);
            selectedSlot = -1; // Reset lựa chọn mỗi khi mở lại
        }
    }
}

// --- LOGIC DÙNG ITEM (MỚI) ---
void Inventory_UseItem(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_INVENTORY_SLOTS) return;
    
    ItemID id = inventory[slotIndex].itemID;
    
    if (id == ITEM_POTION_HP) {
        printf(">> [ACTION] Su dung Potion!\n");
        // [TODO] Gọi hàm hồi máu nhân vật ở đây, ví dụ: Gameplay_HealPlayer(50);
        
        // Trừ số lượng
        inventory[slotIndex].quantity--;
        if (inventory[slotIndex].quantity <= 0) {
            inventory[slotIndex].itemID = ITEM_NONE; 
            selectedSlot = -1; // Hủy chọn vì item đã mất
        }
    }
    else {
        printf(">> [ACTION] Item nay khong su dung duoc!\n");
    }
}

// --- HÀM VẼ CHÍNH (Đã nâng cấp Phân trang & Tương tác) ---
void Inventory_Draw() {
    if (!isInventoryOpen) return;

    // 1. Overlay & Khung chính
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
    DrawRectangleRec(uiBounds, Fade(DARKBLUE, 0.9f));
    DrawRectangleLinesEx(uiBounds, 3.0f, WHITE);
    
    // Vẽ tiêu đề trang
    DrawText(TextFormat("TRANG %d / %d", currentPage + 1, maxPages), 
             (int)uiBounds.x + 20, (int)uiBounds.y + 20, 20, YELLOW);

    // 2. Tính toán Grid (Lưới hiển thị item)
    float startX = uiBounds.x + 30;
    float startY = uiBounds.y + 60;
    float slotSize = 60;
    float padding = 10;
    int cols = 5; // 5 cột

    // Xác định ô bắt đầu và kết thúc của trang hiện tại
    int startIdx = currentPage * SLOTS_PER_PAGE;
    int endIdx = startIdx + SLOTS_PER_PAGE;
    if (endIdx > MAX_INVENTORY_SLOTS) endIdx = MAX_INVENTORY_SLOTS;

    Vector2 mousePos = GetVirtualMousePos();

    // --- VÒNG LẶP VẼ ITEM THEO TRANG ---
    for (int i = startIdx; i < endIdx; i++) {
        // Tính vị trí vẽ tương đối trong trang (0 -> 19)
        int localIdx = i - startIdx;
        int col = localIdx % cols;
        int row = localIdx / cols;
        
        Rectangle slotRect = {
            startX + col * (slotSize + padding),
            startY + row * (slotSize + padding),
            slotSize,
            slotSize
        };

        // Màu nền Slot (Nếu đang chọn thì sáng lên)
        Color slotColor = Fade(BLACK, 0.5f);
        if (i == selectedSlot) slotColor = Fade(WHITE, 0.3f); 
        
        DrawRectangleRec(slotRect, slotColor);
        DrawRectangleLinesEx(slotRect, 1.0f, GRAY);

        // Vẽ Item nếu có
        if (inventory[i].itemID != ITEM_NONE) {
            ItemDef def = itemDatabase[inventory[i].itemID];
            
            // Icon
            DrawRectangleRec((Rectangle){slotRect.x + 5, slotRect.y + 5, slotSize - 10, slotSize - 10}, def.debugColor);
            
            // Số lượng
            if (inventory[i].quantity > 1) {
                DrawText(TextFormat("%d", inventory[i].quantity), 
                         (int)slotRect.x + 2, (int)slotRect.y + slotSize - 15, 10, WHITE);
            }

            // LOGIC CLICK CHỌN: Bấm vào item để xem thông tin
            if (CheckCollisionPointRec(mousePos, slotRect)) {
                DrawRectangleLinesEx(slotRect, 2.0f, YELLOW); // Hover
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                    if (selectedSlot == i) selectedSlot = -1; // Bấm lại thì bỏ chọn
                    else selectedSlot = i; // Chọn ô mới
                }
            }
        }
    }

    // --- 3. VẼ NÚT CHUYỂN TRANG (Ở dưới cùng) ---
    // [LƯU Ý]: Bạn hãy dùng Debug Tool (=) để chỉnh lại tọa độ rect này cho đẹp
    if (maxPages > 1) {
        // Nút Lùi (<)
        if (DrawInvButton("<", (Rectangle){ uiBounds.x + 30, uiBounds.y + 350, 40, 40 })) {
            if (currentPage > 0) currentPage--;
        }
        
        // Nút Tiến (>)
        if (DrawInvButton(">", (Rectangle){ uiBounds.x + 340, uiBounds.y + 350, 40, 40 })) {
            if (currentPage < maxPages - 1) currentPage++;
        }
    }

    // --- 4. ACTION PANEL (Bảng thông tin bên phải) ---
    // Chỉ hiện khi có item được chọn
    if (selectedSlot != -1) {
        ItemDef def = itemDatabase[inventory[selectedSlot].itemID];
        
        // Khu vực bên phải lưới item
        float panelX = uiBounds.x + 400; 
        float panelY = uiBounds.y + 60;
        
        DrawText("THONG TIN:", (int)panelX, (int)panelY, 20, YELLOW);
        DrawText(def.name, (int)panelX, (int)panelY + 30, 20, WHITE);
        
        // Mô tả (font nhỏ hơn)
        DrawText(def.description, (int)panelX, (int)panelY + 60, 10, LIGHTGRAY);
        
        // Nút SỬ DỤNG
        if (DrawInvButton("DUNG", (Rectangle){ panelX, panelY + 120, 100, 40 })) {
            Inventory_UseItem(selectedSlot);
        }

        // Nút BỎ CHỌN
        if (DrawInvButton("HUY", (Rectangle){ panelX, panelY + 170, 100, 40 })) {
            selectedSlot = -1;
        }
    }
}

// --- HELPER FUNCTIONS ---

bool Inventory_AddItem(ItemID id, int quantity) {
    if (id == ITEM_NONE || quantity <= 0) return false;
    // Stack logic
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == id) {
            inventory[i].quantity += quantity;
            return true;
        }
    }
    // Find empty logic
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == ITEM_NONE) {
            inventory[i].itemID = id;
            inventory[i].quantity = quantity;
            return true;
        }
    }
    return false;
}

bool Inventory_HasItem(ItemID id) {
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == id && inventory[i].quantity > 0) return true;
    }
    return false;
}

bool Inventory_IsActive() { return isInventoryOpen; }

const char* Inventory_GetItemName(ItemID id) {
    if (id >= 0 && id < ITEM_COUNT) return itemDatabase[id].name;
    return "Unknown";
}