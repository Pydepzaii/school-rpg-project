#include "gameplay.h"
#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include "debug.h"
#include "renderer.h"
#include "interact.h"
#include "ui_style.h"
#include "audio_manager.h"
#include "camera.h"
#include "transition.h"
#include "menu_system.h" 
#include "inventory.h"
static Player mainCharacter;
static GameMap currentMap;
static Npc npcList[MAX_NPCS];
static int npcCount = 0;

// --- 2. CÀI ĐẶT CÁC HÀM ---

void Gameplay_Init() {
    // Setup Objects
    InitPlayer(&mainCharacter, CLASS_STUDENT); 
    mainCharacter.position = (Vector2){ 400, 300 }; 
    currentMap.texture.id = 0; 
    LoadMap(&currentMap, MAP_TOA_ALPHA); 
    
    npcCount = 0;
    Npc_LoadForMap(MAP_THU_VIEN, npcList, &npcCount);
    
    // Đảm bảo nhạc đúng map
    Audio_PlayMusicForMap(MAP_THU_VIEN);
}
// Hàm đổi class nhân vật (Gọi từ Menu)
void Gameplay_SetPlayerClass(int classID) {
    // 1. Xóa ảnh nhân vật cũ khỏi RAM
    UnloadPlayer(&mainCharacter);
    
    // 2. Tạo lại nhân vật mới với ID class mới
    InitPlayer(&mainCharacter, classID);
    
    // 3. Đặt vị trí xuất phát
    mainCharacter.position = (Vector2){ 400, 300 }; 
}

void Gameplay_Update() {
    // Logic debug map (F1, F2...)
    if (IsKeyPressed(KEY_F1)) Transition_StartToMap(MAP_THU_VIEN, (Vector2){200, 200});
    if (IsKeyPressed(KEY_F2)) Transition_StartToMap(MAP_TOA_ALPHA, (Vector2){400, 300});
    if (IsKeyPressed(KEY_F3)) Transition_StartToMap(MAP_NHA_VO, (Vector2){300, 300});

    // Chỉ update khi không chuyển cảnh
    if (!Transition_IsActive()) {
        Inventory_Update();
        if (!Inventory_IsActive()) {
            UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
            
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) UpdateNpc(&npcList[i]);
            }
            Interact_Update(&mainCharacter, npcList, npcCount, &currentMap);
            // Cập nhật Camera bám theo nhân vật
            Camera_Update(&mainCharacter, &currentMap);
        }
    }
    // Cập nhật chuyển cảnh (Transition cần dữ liệu để load map mới nếu có lệnh chuyển)
    Transition_Update(&currentMap, &mainCharacter, npcList, &npcCount);
}

void Gameplay_Draw() {
    // 1. VẼ THẾ GIỚI GAME (Có Camera)
    BeginMode2D(gameCamera); // gameCamera lấy từ camera.h

        DrawMap(&currentMap);
        
        // Renderer (Y-Sorting)
        Render_Clear(); 
        Render_AddPlayer(&mainCharacter);
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == currentMap.currentMapID) Render_AddNpc(&npcList[i]);
        }
        // [NEW] Add Props vào Renderer
        for (int i = 0; i < currentMap.propCount; i++) {
            // Tính toán sortY: Vị trí Y + Chiều cao (Đáy ảnh)
            float sortY = currentMap.props[i].position.y + currentMap.props[i].originY;
            Render_AddProp(&currentMap.props[i]);
        }
        Render_DrawAll(); 

        // Vẽ Debug (Hitbox)
        Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount); 
        Debug_RunPropTool(&currentMap);
    EndMode2D(); 

    // 2. VẼ UI (Không chịu ảnh hưởng Camera)
    Interact_DrawUI(&mainCharacter, npcList, npcCount, &currentMap);
    Inventory_Draw();//vẽ túi đồ
    if (Inventory_IsActive()) {
        Debug_RunMenuTool(); 
    }
    DrawText("F11: full screen", 10, GetScreenHeight() - 30, 20, LIGHTGRAY);
    DrawTextEx(globalFont, "F1: Thư viện | F2: Nhà Võ", (Vector2){10, 10}, 24, 1, PURPLE);
    
    const char* hpText = TextFormat("hp: %d/%d", mainCharacter.stats.hp, mainCharacter.stats.maxHp);
    DrawTextEx(globalFont, hpText, (Vector2){10, 40}, 24, 1, GREEN);
}

void Gameplay_Shutdown() {
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    // Nếu NPC có load texture riêng thì unload ở đây
}
void Gameplay_GetSaveInfo(int *mapID, Vector2 *pos, PlayerStats *stats) {
    *mapID = currentMap.currentMapID;
    *pos = mainCharacter.position;
    *stats = mainCharacter.stats;
}

// Hàm này nhận stats từ file save và áp dụng vào nhân vật
void Gameplay_LoadStats(PlayerStats stats) {
    mainCharacter.stats = stats;
}