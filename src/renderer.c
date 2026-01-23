// FILE: src/renderer.c
#include "renderer.h"
#include <stdlib.h> // Dùng cho qsort (sắp xếp)

// --- ĐỊNH NGHĨA CÁC LOẠI ĐỐI TƯỢNG ---
typedef enum {
    TYPE_PLAYER,
    TYPE_NPC,
    TYPE_PROP // Cây cối, bàn ghế, cột...
} RenderType;

// Một "Gói hàng" chứa thông tin để vẽ
typedef struct {
    RenderType type;
    void *data;      // Con trỏ trỏ tới dữ liệu gốc (Player* hoặc Npc*)
    float sortY;     // Tọa độ Y tại CHÂN của đối tượng (Dùng để so sánh)
} RenderItem;

// --- BỘ NHỚ ---
#define MAX_RENDER_ITEMS 1000
static RenderItem renderList[MAX_RENDER_ITEMS];
static int renderCount = 0;

void InitRenderer() {
    renderCount = 0;
}

// Hàm thêm Player vào danh sách
void Render_AddPlayer(Player *player) {
    if (renderCount >= MAX_RENDER_ITEMS) return;
    
    renderList[renderCount].type = TYPE_PLAYER;
    renderList[renderCount].data = (void*)player;
    // Quan trọng: sortY là vị trí chân (Y + chiều cao)
    renderList[renderCount].sortY = player->position.y + player->spriteHeight;
    renderCount++;
}

// Hàm thêm NPC vào danh sách
void Render_AddNpc(Npc *npc) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_NPC;
    renderList[renderCount].data = (void*)npc;
    // Tính chiều cao NPC (Texture height)
    renderList[renderCount].sortY = npc->position.y + npc->texture.height;
    renderCount++;
}

// Hàm thêm Vật cản tĩnh (Cây, Tủ...) vào danh sách
void Render_AddProp(GameProp *prop) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_PROP;
    renderList[renderCount].data = (void*)prop;
    renderList[renderCount].sortY = prop->position.y + prop->originY;
    renderCount++;
}

// --- LOGIC SẮP XẾP ---
// Hàm so sánh để dùng cho qsort
// Nếu A ở thấp hơn B (Y lớn hơn) -> A vẽ sau -> A lớn hơn
int CompareRenderItems(const void *a, const void *b) {
    RenderItem *itemA = (RenderItem *)a;
    RenderItem *itemB = (RenderItem *)b;

    if (itemA->sortY < itemB->sortY) return -1; // A ở xa hơn, vẽ trước
    if (itemA->sortY > itemB->sortY) return 1;  // A ở gần hơn, vẽ sau
    return 0;
}

// --- LOGIC VẼ ---
void Render_DrawAll() {
    // 1. Sắp xếp danh sách từ Y nhỏ -> Y lớn (Xa -> Gần)
    qsort(renderList, renderCount, sizeof(RenderItem), CompareRenderItems);

    // 2. Duyệt danh sách và vẽ từng món
    for (int i = 0; i < renderCount; i++) {
        RenderItem item = renderList[i];

        if (item.type == TYPE_PLAYER) {
            DrawPlayer((Player*)item.data);
        }
        else if (item.type == TYPE_NPC) {
            DrawNpc((Npc*)item.data);
        }
        else if (item.type == TYPE_PROP) {
            // Logic vẽ Prop (Vật tĩnh)
            GameProp *p = (GameProp*)item.data;
            DrawTextureRec(p->texture, p->sourceRec, p->position, WHITE);
        }
    }
}

// Xóa danh sách để chuẩn bị cho khung hình tiếp theo
void Render_Clear() {
    renderCount = 0;
}