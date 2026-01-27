// FILE: src/renderer.c
#include "renderer.h"
#include <stdlib.h> 

// --- TYPE DEFINITIONS ---
typedef enum {
    TYPE_PLAYER,
    TYPE_NPC,
    TYPE_PROP 
} RenderType;

// [STRUCT] RenderItem
// Wrapper chứa thông tin cần thiết để sắp xếp thứ tự vẽ
typedef struct {
    RenderType type;
    void *data;      // Con trỏ void trỏ đến object gốc (Player*, Npc*...)
    float sortY;     // [CRITICAL] Tọa độ Y dùng để so sánh (Depth Key)
} RenderItem;

// --- RENDER QUEUE ---
#define MAX_RENDER_ITEMS 1000
static RenderItem renderList[MAX_RENDER_ITEMS];
static int renderCount = 0;

void InitRenderer() {
    renderCount = 0;
}

// Hàm helper thêm Player vào hàng đợi vẽ
void Render_AddPlayer(Player *player) {
    if (renderCount >= MAX_RENDER_ITEMS) return;
    
    renderList[renderCount].type = TYPE_PLAYER;
    renderList[renderCount].data = (void*)player;
    // Pivot Y: Chân nhân vật (đáy ảnh)
    renderList[renderCount].sortY = player->position.y + player->spriteHeight;
    renderCount++;
}

// Hàm helper thêm NPC
void Render_AddNpc(Npc *npc) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_NPC;
    renderList[renderCount].data = (void*)npc;
    
    // [ADJUSTMENT] NPC Sort Y
    // Đồng bộ logic này với Hitbox trong debug.c và player.c
    // Trừ đi paddingBottom để điểm sort nằm đúng chân "vật lý"
    float paddingBottom = 17.0f;
    renderList[renderCount].sortY = npc->position.y + npc->texture.height - paddingBottom;
    
    renderCount++;
}

// Hàm helper thêm vật thể tĩnh (Cột nhà, cây cối...)
void Render_AddProp(GameProp *prop) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_PROP;
    renderList[renderCount].data = (void*)prop;
    // Props thường có điểm neo (originY) tùy chỉnh
    renderList[renderCount].sortY = prop->position.y + prop->originY;
    renderCount++;
}

// [ALGORITHM] Comparator for qsort
// So sánh Y: Y nhỏ (xa, trên cao) vẽ trước. Y lớn (gần, dưới thấp) vẽ sau.
int CompareRenderItems(const void *a, const void *b) {
    RenderItem *itemA = (RenderItem *)a;
    RenderItem *itemB = (RenderItem *)b;

    if (itemA->sortY < itemB->sortY) return -1; 
    if (itemA->sortY > itemB->sortY) return 1;  
    return 0;
}

void Render_DrawAll() {
    // 1. Sorting: Sắp xếp danh sách dựa trên Y
    qsort(renderList, renderCount, sizeof(RenderItem), CompareRenderItems);

    // 2. Painting: Duyệt danh sách đã sắp xếp và vẽ từ dưới lên trên
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
    renderCount = 0; // Reset hàng đợi mỗi frame
}