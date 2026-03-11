// FILE: src/inventory.c
#include "inventory.h"
#include "settings.h" 
#include "audio_manager.h"
#include "gameplay.h" 
#include "debug.h"
#include "menu_system.h" // [NEW] Để gọi Menu Switch
#include "ui_style.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "raymath.h"

// --- CẤU HÌNH ---
#define SLOTS_PER_PAGE 20 
// --- BIẾN CHO BẢN ĐỒ BÍ MẬT ---
bool isShowingSecretMap = false;
static Texture2D texSecretMap = {0};
static float mapShowTimer = 0.0f; // [MỚI] Biến đếm thời gian chống "Click Ma"

void Inventory_ShowSecretMap() {
    isShowingSecretMap = true;
    mapShowTimer = 0.5f; // [MỚI] Khóa click chuột trong 0.5 giây đầu tiên
}

void Inventory_DrawSecretMap() {
    if (isShowingSecretMap) {
        // Load ảnh (Hãy chắc chắn bạn đã vứt ảnh vào thư mục này)
        if (texSecretMap.id == 0) texSecretMap = LoadTexture("resources/item/mapfpt.png"); 

        // Đếm ngược thời gian chống Click Ma
        if (mapShowTimer > 0.0f) mapShowTimer -= GetFrameTime();

        // Phủ đen mờ màn hình
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.85f));
        
        // [ĐÃ FIX] THUẬT TOÁN TỰ ĐỘNG CĂN CHỈNH ẢNH VỪA KHÍT MÀN HÌNH ẢO
        float padding = 60.0f; // Khoảng cách từ viền ảnh tới viền màn hình
        float scaleX = (SCREEN_WIDTH - padding) / texSecretMap.width;
        float scaleY = (SCREEN_HEIGHT - padding) / texSecretMap.height;
        float scale = (scaleX < scaleY) ? scaleX : scaleY; // Lấy tỷ lệ nhỏ hơn để giữ đúng form không bị méo

        float w = texSecretMap.width * scale;
        float h = texSecretMap.height * scale;
        float x = (SCREEN_WIDTH - w) / 2.0f;
        float y = (SCREEN_HEIGHT - h) / 2.0f;
        
        DrawTextureEx(texSecretMap, (Vector2){x, y}, 0.0f, scale, WHITE);
        DrawRectangleLines((int)x, (int)y, (int)w, (int)h, YELLOW);
        
        DrawTextEx(globalFont, "CLICK CHUOT (HOAC ENTER/ESC) DE DONG BAN DO", (Vector2){x + w/2 - 250, y + h + 10}, 20, 1, YELLOW);

        // [ĐÃ FIX] Chỉ cho phép người chơi đóng bản đồ khi ĐÃ HẾT 0.5 GIÂY DELAY
        if (mapShowTimer <= 0.0f) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                isShowingSecretMap = false;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
    }
}

// --- DỮ LIỆU TĨNH ---
static ItemDef itemDatabase[ITEM_COUNT] = {
    // id, name, description, sourceRec { x, y, width, height }
   [ITEM_NONE] = { ITEM_NONE, "Empty", "", {0, 0, 0, 0} },
    [ITEM_KEY_ALPHA]={ ITEM_KEY_ALPHA, "Key Alpha", "Chia khoa mở cửa nhà võ", {0 * ITEM_GRID_SIZE, 0, 22, 16} },
    [ITEM_KEY_BETA] = { ITEM_KEY_BETA, "Key Beta", "Chia khoa vao beta", {0, 16, 22, 16} },
    [ITEM_KEY_DELTA] = { ITEM_KEY_DELTA, "Key delta", "Chia khoa vao delta", {0, 32, 22, 16} },
    [ITEM_1] = { ITEM_1, "BẢN ĐỒ", "TRÊN ĐÓ CÓ ĐÁNH 1 DẤU x", {3,54,23,20} },
    [ITEM_2] = { ITEM_2, "TẤM THẺ TỪ", "Đây là tấm thẻ để lên tầng 2", {7,78,12,18} },
    [ITEM_BOOK_1] = { ITEM_BOOK_1, "qUẤN SÁCH SỐ 1", "Không biết nó viết gì bên trong nhỉ?", {33,1,14,13} },
    [ITEM_BOOK_2] = { ITEM_BOOK_2, "qUẤN SÁCH SỐ 2", "Không biết nó viết gì bên trong nhỉ?", {49,1,14,13} },
    [ITEM_BOOK_3] = { ITEM_BOOK_3, "qUẤN SÁCH SỐ 3", "Không biết nó viết gì bên trong nhỉ?", {65,1,14,13} },
    [ITEM_BOOK_4] = { ITEM_BOOK_4, "QUẤN SÁCH SỐ 4", "Không biết nó viết gì bên trong nhỉ?", {33,17,14,13} },
    [ITEM_BOOK_5] = { ITEM_BOOK_5, "QUẤN SÁCH SỐ 5", "Không biết nó viết gì bên trong nhỉ?", {49,17,14,13} },
    [ITEM_BOOK_6] = { ITEM_BOOK_6, "QUẤN SÁCH SỐ 6", "Không biết nó viết gì bên trong nhỉ?", {65,17    ,14,13} },
    [ITEM_NOTE_PAPER] = { ITEM_NOTE_PAPER, "Mảnh giấy nhỏ", "Nó có viết gì đó", {36,59,23,22} },
    [ITEM_KEY_SPECIAL] = { ITEM_KEY_SPECIAL, "Key", "Default", {33,35,22,16} }
};

// --- TRẠNG THÁI ---
static InventorySlot inventory[MAX_INVENTORY_SLOTS];
static Rectangle uiBounds; 

// --- TƯƠNG TÁC ---
static int selectedSlot = -1; 
static int currentPage = 0;   
static int maxPages = 0;      

// --- [MỚI] TRẠNG THÁI ĐỒ RỚT TRÊN MAP ---
static Texture2D itemSpriteSheet;
// Cấu trúc 1 vật phẩm rớt
typedef struct { 
    ItemID id; 
    Vector2 position; 
    int mapID; 
    bool active; 
    
    // [THÊM MỚI] CÁC BIẾN CHO HIỆU ỨNG NHẶT ĐỒ
    bool isPickingUp;
    float animTimer;
    Vector2 startPos;
    float scale;
    float rotation;
} ItemEntity;
static ItemEntity droppedItems[MAX_ITEMS_ON_MAP];
// Cấu trúc hiệu ứng chữ
typedef struct { char text[64]; Vector2 position; float timer; float alpha; bool active; } ItemNotification;
static ItemNotification notifications[MAX_NOTIFICATIONS];

// [DELETED] Đã xóa DrawInvButton vì chuyển sang menu_system.c

void Inventory_Init() {
    // Tải ảnh chứa toàn bộ Item (TẮT LÀM MỊN ĐỂ ẢNH RÕ NÉT)
    itemSpriteSheet = LoadTexture("resources/item/items.png"); // <--- NHỚ CHỈNH LẠI PATH NẾU SAI
    SetTextureFilter(itemSpriteSheet, TEXTURE_FILTER_POINT);

    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        inventory[i].itemID = ITEM_NONE;
        inventory[i].quantity = 0;
    }
    for (int i = 0; i < MAX_ITEMS_ON_MAP; i++) droppedItems[i].active = false;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) notifications[i].active = false;
    
    // Tính toán khung UI (GIỮ NGUYÊN TỌA ĐỘ ĐỂ MENU DEBUG KHÔNG LỖI)
    float w = 600; 
    float h = 450;
    uiBounds = (Rectangle){ (SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2, w, h };
    
    maxPages = (MAX_INVENTORY_SLOTS + SLOTS_PER_PAGE - 1) / SLOTS_PER_PAGE;
    if (maxPages < 1) maxPages = 1;

    printf(">> [INVENTORY] System Initialized. Max Pages: %d\n", maxPages);
    
    
}

void Inventory_Unload() {
    UnloadTexture(itemSpriteSheet);
    if (texSecretMap.id != 0) UnloadTexture(texSecretMap);
}

void Inventory_Update() {
   // [ĐÃ KHÓA] Không cho mở bằng phím B nữa, bắt buộc mở qua Menu Pause
    /*
    if (IsKeyPressed(KEY_B) && !Menu_IsActive()) {
        Menu_SwitchTo(MENU_INVENTORY); 
        selectedSlot = -1;             
        Audio_PlaySoundEffect(SFX_UI_CLICK);
    }
    */
}

// --- LOGIC GỌI TỪ MENU SYSTEM ---
void Inventory_NextPage() {
    if (currentPage < maxPages - 1) currentPage++;
}
void Inventory_PrevPage() {
    if (currentPage > 0) currentPage--;
}
void Inventory_UseSelected() {
    if (selectedSlot < 0 || selectedSlot >= MAX_INVENTORY_SLOTS) return;
    
    ItemID id = inventory[selectedSlot].itemID;
    
    if (id == ITEM_POTION_HP) {
        printf(">> [ACTION] Su dung Potion!\n");
        // Gameplay_HealPlayer(50);
        
        inventory[selectedSlot].quantity--;
        if (inventory[selectedSlot].quantity <= 0) {
            inventory[selectedSlot].itemID = ITEM_NONE; 
            selectedSlot = -1; 
        }
    } 
    // [ĐÃ FIX] Chuyển else if ra ngoài ngang hàng với cái if đầu tiên
    else if (id == ITEM_1) {
        printf(">> [ACTION] Su dung Ban Do!\n");
        Inventory_ShowSecretMap(); 
    } 
    else {
        printf(">> [ACTION] Item nay khong su dung duoc!\n");
    }
}
void Inventory_Unselect() { selectedSlot = -1; }
bool Inventory_HasSelectedSlot() { return selectedSlot != -1; }
int Inventory_GetCurrentPage() { return currentPage; }
int Inventory_GetMaxPages() { return maxPages; }

// --- HÀM VẼ (Chỉ vẽ Grid và Info, KHÔNG vẽ nút) ---
void Inventory_Draw() {
    // Hàm này giờ chỉ vẽ nội dung (Grid đồ + Text). Nền sách đã được menu_system lo!

    float sw = (float)SCREEN_WIDTH;  
    float sh = (float)SCREEN_HEIGHT; 
    
    // Kích thước chuẩn của sách (Copy y hệt thông số từ DrawInfoBookAnimation bên menu_system)
    float bookWidth = sw * 0.8f;      
    float bookHeight = sh * 0.75f;    
    float startX = (sw - bookWidth) / 2.0f;
    float startY = (sh - bookHeight) / 2.0f + 20.0f; 

    // ==========================================
    // 1. VẼ TRANG TRÁI (LƯỚI ITEM)
    // ==========================================
    float leftPageStartX = startX + 40.0f; 
    float leftPageStartY = startY + 50.0f;

    // Chữ đếm trang
    DrawTextEx(globalFont, TextFormat("Trang %d / %d", currentPage + 1, maxPages), 
             (Vector2){ leftPageStartX, leftPageStartY - 20 }, 20, 1, DARKGRAY);

    float slotSize = 48; // Thu nhỏ lại chút cho vừa 1 trang giấy
    float padding = 8;
    int cols = 5; 
    int startIdx = currentPage * SLOTS_PER_PAGE;
    int endIdx = startIdx + SLOTS_PER_PAGE;
    if (endIdx > MAX_INVENTORY_SLOTS) endIdx = MAX_INVENTORY_SLOTS;

    // Tọa độ cắt UI trên texUIBook (Bạn tự lấy số liệu tọa độ ô vuông từ file ảnh UI của bạn gắn vào đây nhé)
    Rectangle srcEmptySlot = { 65, 273, 30, 30 }; // Mỏ neo: Khung ô trống (Lúc bình thường)
    Rectangle srcHoverSlot = { 225, 273, 30, 30 }; // Mỏ neo: Khung ô sáng lên (Lúc lướt chuột hoặc đang chọn)

    Vector2 mousePos = GetVirtualMousePos();

    for (int i = startIdx; i < endIdx; i++) {
        int localIdx = i - startIdx;
        int col = localIdx % cols;
        int row = localIdx / cols;
        
        Rectangle slotRect = {
            leftPageStartX + col * (slotSize + padding),
            leftPageStartY + row * (slotSize + padding),
            slotSize,
            slotSize
        };

        bool isHover = CheckCollisionPointRec(mousePos, slotRect);
        
        // Vẽ khung Slot bằng ảnh 9-Slice (cắt từ texUIBook được khai báo extern trong menu_system.h)
        Rectangle activeSrc = (isHover || i == selectedSlot) ? srcHoverSlot : srcEmptySlot;
        NPatchInfo nPatchSlot = { .source = activeSrc, .left = 4, .top = 4, .right = 4, .bottom = 4, .layout = NPATCH_NINE_PATCH };
        DrawTextureNPatch(texUIBook, nPatchSlot, slotRect, (Vector2){0,0}, 0.0f, WHITE);

        // Vẽ Item bên trong
        if (inventory[i].itemID != ITEM_NONE) {
            ItemDef def = itemDatabase[inventory[i].itemID];
            
            // Vẽ ảnh Item lọt lòng khung ô (Thụt lề vô 6px)
            Rectangle destRec = { slotRect.x + 6, slotRect.y + 6, slotSize - 12, slotSize - 12 }; 
            DrawTexturePro(itemSpriteSheet, def.sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);
            
            if (inventory[i].quantity > 1) {
                DrawTextEx(globalFont, TextFormat("%d", inventory[i].quantity), 
                         (Vector2){ slotRect.x + 4, slotRect.y + slotSize - 18 }, 16, 1, WHITE);
            }

            // Logic Click chọn Slot (Chỉ cho bấm khi lướt chuột)
            if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Audio_PlaySoundEffect(SFX_UI_CLICK);
                if (selectedSlot == i) selectedSlot = -1; 
                else selectedSlot = i; 
            }
        }
    }

    // ==========================================
    // 2. VẼ TRANG PHẢI (THÔNG TIN CHI TIẾT)
    // ==========================================
    if (selectedSlot != -1) {
        ItemDef def = itemDatabase[inventory[selectedSlot].itemID];
        
        // Căn lề trang phải (Vượt qua gáy sách, đẩy sang nửa bên phải)
        float rightPageStartX = startX + (bookWidth / 2.0f) + 40.0f; 
        float rightPageStartY = startY + 60.0f;
        
        DrawTextEx(globalFont, "CHI TIẾT VẬT PHẨM:", (Vector2){ rightPageStartX, rightPageStartY }, 20, 1, DARKGRAY);
        DrawTextEx(globalFont, def.name, (Vector2){ rightPageStartX, rightPageStartY + 30 }, 24, 1, MAROON);
        DrawTextEx(globalFont, def.description, (Vector2){ rightPageStartX, rightPageStartY + 60 }, 18, 1, DARKGRAY);
        
        // Các nút bấm "DÙNG", "HỦY", "TRANG TRƯỚC", "TRANG SAU" sẽ được vẽ tự động 
        // bằng hệ thống menu_system.c đè lên trang phải này.
    }
}

// ========================================================
// [MỚI] LOGIC QUẢN LÝ ĐỒ RỚT VÀ CHỮ HIỂU ỨNG TRÊN MAP
// ========================================================
static void ShowNotification(const char* text, Vector2 startPos) {
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (!notifications[i].active) {
            strcpy(notifications[i].text, text);
            notifications[i].position = startPos;
            notifications[i].timer = 4.0f; 
            notifications[i].alpha = 1.0f;
            notifications[i].active = true;
            return;
        }
    }
}

void Inventory_SpawnItem(ItemID id, Vector2 pos, int mapID) {
    for (int i = 0; i < MAX_ITEMS_ON_MAP; i++) {
        if (!droppedItems[i].active) {
            droppedItems[i].id = id;
            droppedItems[i].position = pos;
            droppedItems[i].mapID = mapID;
            droppedItems[i].active = true;
            droppedItems[i].isPickingUp = false;
            droppedItems[i].animTimer = 0.0f;
            return;
        }
    }
}

// Hàm kiểm tra xem có item nào đang chạy hiệu ứng không
bool Inventory_IsPickingUpAnimActive() {
    for (int i = 0; i < MAX_ITEMS_ON_MAP; i++) {
        if (droppedItems[i].active && droppedItems[i].isPickingUp) return true;
    }
    return false;
}

void Inventory_UpdateItemsOnMap(Rectangle playerHitbox, int currentMapID) {
    // Tính tâm của người chơi để hút đồ vào
    Vector2 playerCenter = { playerHitbox.x + playerHitbox.width / 2.0f, playerHitbox.y + playerHitbox.height / 2.0f };

    for (int i = 0; i < MAX_ITEMS_ON_MAP; i++) {
        if (droppedItems[i].active && droppedItems[i].mapID == currentMapID) {
            
            // NẾU ĐANG TRONG TRẠNG THÁI HIỆU ỨNG NHẶT ĐỒ
            if (droppedItems[i].isPickingUp) {
                droppedItems[i].animTimer += GetFrameTime();
                float t = droppedItems[i].animTimer;
                
                // Giai đoạn 1 (0.0s - 0.4s): Phình to và nảy lên đỉnh đầu
                if (t < 0.4f) {
                    float p = t / 0.4f; 
                    droppedItems[i].position.y = droppedItems[i].startPos.y - (60.0f * sin(p * PI / 2.0f));
                    droppedItems[i].scale = 1.0f + (p * 1.5f); // Phình to lên 2.5 lần
                    droppedItems[i].rotation += 600.0f * GetFrameTime(); // Xoay vòng
                } 
                // Giai đoạn 2 (0.4s - 1.0s): Lơ lửng trên không xoay tít thò lò
                else if (t < 1.0f) {
                    droppedItems[i].rotation += 800.0f * GetFrameTime();
                } 
                // Giai đoạn 3 (1.0s - 1.3s): Thu nhỏ và hút thẳng vào người chơi
                else if (t < 1.3f) {
                    float p = (t - 1.0f) / 0.3f; 
                    Vector2 hoverPos = { droppedItems[i].startPos.x, droppedItems[i].startPos.y - 60.0f };
                    
                    // Bay thẳng vào tâm người chơi
                    droppedItems[i].position.x = hoverPos.x + (playerCenter.x - hoverPos.x - 16.0f) * p;
                    droppedItems[i].position.y = hoverPos.y + (playerCenter.y - hoverPos.y - 16.0f) * p;
                    droppedItems[i].scale = 2.5f * (1.0f - p); // Teo nhỏ dần về 0
                    droppedItems[i].rotation += 1200.0f * GetFrameTime();
                } 
                // Giai đoạn cuối: Xóa vật phẩm, Phát tiếng, Hiện chữ và Nhét vào túi
                else {
                    if (Inventory_AddItem(droppedItems[i].id, 1)) {
                        char notifText[100];
                        sprintf(notifText, "Da nhat duoc: %s", itemDatabase[droppedItems[i].id].name);
                        ShowNotification(notifText, (Vector2){playerCenter.x - 40, playerCenter.y - 50});
                        Audio_PlaySoundEffect(SFX_UI_CLICK); // PHÁT ÂM THANH
                        droppedItems[i].active = false;
                    } else {
                        // Túi đầy -> Bỏ hiệu ứng, rớt lại ra đất
                        droppedItems[i].isPickingUp = false;
                        droppedItems[i].position = droppedItems[i].startPos;
                    }
                }
            } 
            // NẾU CHƯA NHẶT -> BÌNH THƯỜNG DƯỚI ĐẤT CHỜ CHẠM
            else {
                // [ĐÃ SỬA] Thay vì bắt nhân vật giẫm lên đồ (CheckCollisionRecs), 
                // ta dùng cơ chế Nam Châm: Tính khoảng cách từ tâm người chơi đến tâm vật phẩm.
                Vector2 itemCenter = { 
                    droppedItems[i].position.x + ITEM_GRID_SIZE, 
                    droppedItems[i].position.y + ITEM_GRID_SIZE 
                };
                
                // Bán kính hút là 60.0f (Bao trọn vòng tương tác)
                if (Vector2Distance(playerCenter, itemCenter) < 40.0f) {
                    // Kích hoạt hiệu ứng hút
                    droppedItems[i].isPickingUp = true;
                    droppedItems[i].animTimer = 0.0f;
                    droppedItems[i].startPos = droppedItems[i].position;
                    droppedItems[i].scale = 1.0f;
                    droppedItems[i].rotation = 0.0f;
                }
            }
        }
    }
}

void Inventory_DrawItemsOnMap(int currentMapID) {
    for (int i = 0; i < MAX_ITEMS_ON_MAP; i++) {
        if (droppedItems[i].active && droppedItems[i].mapID == currentMapID) {
            ItemDef def = itemDatabase[droppedItems[i].id];
            
            if (droppedItems[i].isPickingUp) {
                // VẼ KHI ĐANG BAY (CÓ PHÓNG TO VÀ XOAY TRÒN)
                Rectangle source = def.sourceRec;
                Rectangle dest = { 
                    droppedItems[i].position.x + ITEM_GRID_SIZE, // Căn tâm X
                    droppedItems[i].position.y + ITEM_GRID_SIZE, // Căn tâm Y
                    ITEM_GRID_SIZE * 2 * droppedItems[i].scale, 
                    ITEM_GRID_SIZE * 2 * droppedItems[i].scale 
                };
                Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f }; // Tâm xoay
                DrawTexturePro(itemSpriteSheet, source, dest, origin, droppedItems[i].rotation, WHITE);
            } else {
                // VẼ LÚC CHƯA NHẶT (Lơ lửng nhẹ nhàng)
                float floatOffset = sin(GetTime() * 5.0f) * 3.0f; 
                Rectangle destRec = { droppedItems[i].position.x, droppedItems[i].position.y + floatOffset, ITEM_GRID_SIZE * 2, ITEM_GRID_SIZE * 2 };
                DrawTexturePro(itemSpriteSheet, def.sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
            }
        }
    }
}

void Inventory_DrawNotifications() {
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (notifications[i].active) {
            notifications[i].position.y -= 15.0f * dt; // Bay lên chậm hơn một nửa
            notifications[i].timer -= dt;
            if (notifications[i].timer < 1.0f) notifications[i].alpha = notifications[i].timer; 
            
            if (notifications[i].timer <= 0.0f) {
                notifications[i].active = false;
            } else {
                // Đổi thành chữ màu xanh dạ quang cho ngầu
                DrawTextEx(globalFont, notifications[i].text, notifications[i].position, 22, 1, Fade(GREEN, notifications[i].alpha));
            }
        }
    }
}

// --- HELPER ---
bool Inventory_AddItem(ItemID id, int quantity) {
    if (id == ITEM_NONE || quantity <= 0) return false;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == id) {
            inventory[i].quantity += quantity;
            return true;
        }
    }
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == ITEM_NONE) {
            inventory[i].itemID = id;
            inventory[i].quantity = quantity;
            return true;
        }
    }
    return false;
}

bool Inventory_RemoveItem(ItemID id, int quantity) {
    if (id == ITEM_NONE || quantity <= 0) return false;
    
    // 1. Kiểm tra xem người chơi có ĐỦ số lượng không
    int total = 0;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == id) total += inventory[i].quantity;
    }
    if (total < quantity) return false; // Không đủ -> Trả về false ngay lập tức

    // 2. Nếu đủ thì tiến hành trừ đồ
    int remaining = quantity;
    for (int i = 0; i < MAX_INVENTORY_SLOTS && remaining > 0; i++) {
        if (inventory[i].itemID == id) {
            if (inventory[i].quantity >= remaining) {
                inventory[i].quantity -= remaining;
                remaining = 0;
                if (inventory[i].quantity == 0) inventory[i].itemID = ITEM_NONE;
            } else {
                remaining -= inventory[i].quantity;
                inventory[i].quantity = 0;
                inventory[i].itemID = ITEM_NONE;
            }
        }
    }
    return true; // Trừ thành công
}

bool Inventory_HasItem(ItemID id) {
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (inventory[i].itemID == id && inventory[i].quantity > 0) return true;
    }
    return false;
}
bool Inventory_IsActive() { return Menu_IsActive() && currentMenu == MENU_INVENTORY; }
const char* Inventory_GetItemName(ItemID id) {
    if (id >= 0 && id < ITEM_COUNT) return itemDatabase[id].name;
    return "Unknown";
}