// FILE: src/renderer.c
#include "renderer.h"
#include <stdlib.h> 
#include <player.h>
#include "map.h" // [QUAN TRỌNG] Include map để hiểu GameProp

// --- TYPE DEFINITIONS ---
typedef enum {
    TYPE_PLAYER,
    TYPE_NPC,
    TYPE_PROP 
} RenderType;

// [STRUCT] RenderItem
// Wrapper chứa thông tin cần thiết để sắp xếp thứ tự vẽ
// [GIẢI THÍCH]: Struct này dùng để gói mọi thứ cần vẽ lại thành 1 cục chung để dễ sắp xếp.
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
    // [GIẢI THÍCH]: Logic quan trọng nhất: Lấy chân làm điểm mốc để so sánh.
    renderList[renderCount].sortY = player->position.y + player->drawHeight-2.0f;
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
    renderList[renderCount].sortY = npc->position.y + npc->texture.height - npc->paddingBottom;
    renderCount++;
}

void Render_AddProp(GameProp *prop) {
    if (renderCount >= MAX_RENDER_ITEMS) return;

    renderList[renderCount].type = TYPE_PROP;
    
    // Lưu con trỏ trỏ tới vật thể gốc trong Map
    renderList[renderCount].data = (void*)prop; 
    
    // Tính toán độ sâu: Y vị trí + Chiều cao vật thể (Chân đế)
    renderList[renderCount].sortY = prop->position.y + prop->originY;
    
    renderCount++;
}

// [ALGORITHM] Comparator for qsort
// So sánh Y: Y nhỏ (xa, trên cao) vẽ trước. Y lớn (gần, dưới thấp) vẽ sau.
// [GIẢI THÍCH]: Hàm so sánh dùng cho thuật toán Quick Sort của C.
int CompareRenderItems(const void *a, const void *b) {
    RenderItem *itemA = (RenderItem *)a;
    RenderItem *itemB = (RenderItem *)b;

    if (itemA->sortY < itemB->sortY) return -1; 
    if (itemA->sortY > itemB->sortY) return 1;  
    return 0;
}

void Render_DrawAll() {
    // 1. Sorting: Sắp xếp danh sách dựa trên Y
    // [GIẢI THÍCH]: Gọi hàm qsort có sẵn trong thư viện stdlib.h
    qsort(renderList, renderCount, sizeof(RenderItem), CompareRenderItems);

    // 2. Painting: Duyệt danh sách đã sắp xếp và vẽ từ dưới lên trên
    // Object nào ở xa (Y nhỏ) vẽ trước, object ở gần (Y lớn) vẽ sau đè lên.
    for (int i = 0; i < renderCount; i++) {
        RenderItem item = renderList[i];

        if (item.type == TYPE_PLAYER) {
            DrawPlayer((Player*)item.data);
        }
        else if (item.type == TYPE_NPC) {
            DrawNpc((Npc*)item.data);
        }
        else if (item.type == TYPE_PROP) {
            // Bây giờ GameProp đã được hiểu nhờ include map.h
            GameProp *p = (GameProp*)item.data;
            
            if (p->texRef != NULL) {
                // --- [BẮT ĐẦU SỬA] ---
                
                // CODE CŨ (Đã comment lại để backup như yêu cầu):
                // DrawTextureRec(*p->texRef, p->sourceRec, p->position, WHITE);

                // CODE MỚI (Fix lỗi Scale 2.0):
                // Sử dụng DrawTexturePro để có thể phóng to ảnh cắt được
                float scale = 2.0f; // Scale này phải khớp với map->scale bên map.c
                
                Rectangle source = p->sourceRec; // Vùng cắt trên ảnh gốc (nhỏ)
                
                Rectangle dest = {
                    p->position.x,
                    p->position.y,
                    source.width * scale,   // Phóng to chiều ngang gấp đôi
                    source.height * scale   // Phóng to chiều dọc gấp đôi
                };
                
                DrawTexturePro(*p->texRef, source, dest, (Vector2){0,0}, 0.0f, WHITE);
                
                // --- [KẾT THÚC SỬA] ---
            }
        }
    }
}

void Render_Clear() {
    renderCount = 0; // Reset hàng đợi mỗi frame
}