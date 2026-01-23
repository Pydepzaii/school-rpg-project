// FILE: src/renderer.c
#include "renderer.h"
#include <stdlib.h> 

// --- ĐỊNH NGHĨA CÁC LOẠI ĐỐI TƯỢNG ---
typedef enum {
    TYPE_PLAYER,
    TYPE_NPC,
    TYPE_PROP 
} RenderType;

// Một "Gói hàng" chứa thông tin để vẽ
typedef struct {
    RenderType type;
    void *data;      
    float sortY;     // Tọa độ Y tại CHÂN (Dùng để so sánh trước sau)
} RenderItem;

// --- BỘ NHỚ ---
#define MAX_RENDER_ITEMS 1000
static RenderItem renderList[MAX_RENDER_ITEMS];
static int renderCount = 0;

void InitRenderer() {
    renderCount = 0;
}

void Render_AddPlayer(Player *player) {
    if (renderCount >= MAX_RENDER_ITEMS) return;
    
    renderList[renderCount].type = TYPE_PLAYER;
    renderList[renderCount].data = (void*)player;
    // Player chân sát đáy ảnh
    renderList[renderCount].sortY = player->position.y + player->spriteHeight;
    renderCount++;
}

void Render_AddNpc(Npc *npc) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_NPC;
    renderList[renderCount].data = (void*)npc;
    
    // --- [FIX] ĐỒNG BỘ VỚI HITBOX ---
    // Vì hitbox NPC cách đáy ảnh 4px (paddingBottom = 4.0f)
    // Nên điểm so sánh Y cũng phải lùi lên 4px.
    float paddingBottom = 17.0f;
    renderList[renderCount].sortY = npc->position.y + npc->texture.height - paddingBottom;
    
    renderCount++;
}

void Render_AddProp(GameProp *prop) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_PROP;
    renderList[renderCount].data = (void*)prop;
    renderList[renderCount].sortY = prop->position.y + prop->originY;
    renderCount++;
}

// Hàm so sánh cho qsort
int CompareRenderItems(const void *a, const void *b) {
    RenderItem *itemA = (RenderItem *)a;
    RenderItem *itemB = (RenderItem *)b;

    if (itemA->sortY < itemB->sortY) return -1; // A ở xa hơn (Y nhỏ) -> vẽ trước
    if (itemA->sortY > itemB->sortY) return 1;  // A ở gần hơn (Y to) -> vẽ sau
    return 0;
}

void Render_DrawAll() {
    // 1. Sắp xếp danh sách 
    qsort(renderList, renderCount, sizeof(RenderItem), CompareRenderItems);

    // 2. Duyệt danh sách và vẽ
    for (int i = 0; i < renderCount; i++) {
        RenderItem item = renderList[i];

        if (item.type == TYPE_PLAYER) {
            DrawPlayer((Player*)item.data);
        }
        else if (item.type == TYPE_NPC) {
            DrawNpc((Npc*)item.data);
        }
        else if (item.type == TYPE_PROP) {
            GameProp *p = (GameProp*)item.data;
            DrawTextureRec(p->texture, p->sourceRec, p->position, WHITE);
        }
    }
}

void Render_Clear() {
    renderCount = 0;
}