#include "gameplay.h"
#include "raylib.h"
#include "settings.h"
#include "player.h"
#include "map.h"
#include "npc.h"
#include "debug.h"
#include "renderer.h"
#include "interact.h"
#include "story_manager.h"
#include "save_system.h"
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
    Npc_LoadForMap(MAP_TOA_ALPHA, npcList, &npcCount);
    
    // Đảm bảo nhạc đúng map
    Audio_PlayMusicForMap(MAP_TOA_ALPHA);
    // --- ĐẶT SẴN VẬT PHẨM TRÊN MAP ALPHA ---
    // Đặt Book 1 ở khu vực phía trên bên phải. Thay (Vector2){600, 100} bằng tọa độ thật.
    Inventory_SpawnItem(ITEM_BOOK_1, (Vector2){200, 65}, MAP_TOA_ALPHA);
    Inventory_SpawnItem(ITEM_BOOK_2, (Vector2){20, 400}, MAP_NHA_VO);  
    Inventory_SpawnItem(ITEM_BOOK_3, (Vector2){280, 140}, MAP_NHA_AN);
    Inventory_SpawnItem(ITEM_BOOK_4, (Vector2){170, 50}, MAP_THU_VIEN);
    Inventory_SpawnItem(ITEM_BOOK_5, (Vector2){ 480, 225 }, MAP_BETA);
    Inventory_SpawnItem(ITEM_BOOK_6, (Vector2){ 732, 220 }, MAP_LAB);
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
    if (IsKeyPressed(KEY_F4)) Transition_StartToMap(MAP_BETA, (Vector2){300, 300});
    if (IsKeyPressed(KEY_F5)) Transition_StartToMap(MAP_LAB, (Vector2){300, 300});
    if (IsKeyPressed(KEY_F6)) Transition_StartToMap(MAP_NHA_AN, (Vector2){300, 300});

    // Chỉ update khi không chuyển cảnh
    if (!Transition_IsActive()) {
        Inventory_Update();
        if (!Inventory_IsActive() && !IsDialogDebugActive() && !isShowingSecretMap ) {
            UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);
            
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].mapID == currentMap.currentMapID) UpdateNpc(&npcList[i]);
            }
            Interact_Update(&mainCharacter, npcList, npcCount, &currentMap);
            Story_Update(&mainCharacter, &currentMap, npcList, npcCount);
            // Cập nhật Camera bám theo nhân vật
            Camera_Update(&mainCharacter, &currentMap);
        }
    }
    // Cập nhật chuyển cảnh (Transition cần dữ liệu để load map mới nếu có lệnh chuyển)
    Transition_Update(&currentMap, &mainCharacter, npcList, &npcCount);
//Item 
// --- LOGIC TEST RỚT ĐỒ ---
    if (IsKeyPressed(KEY_J)) {
        Inventory_SpawnItem(ITEM_2, (Vector2){ mainCharacter.position.x + 50, mainCharacter.position.y }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_KEY_ALPHA, (Vector2){ mainCharacter.position.x - 50, mainCharacter.position.y }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_KEY_BETA, (Vector2){ mainCharacter.position.x - 75, mainCharacter.position.y }, currentMap.currentMapID);
        Inventory_SpawnItem(ITEM_KEY_DELTA, (Vector2){ mainCharacter.position.x + 75, mainCharacter.position.y }, currentMap.currentMapID); \
        Inventory_SpawnItem(ITEM_1, (Vector2){ mainCharacter.position.x + 50, mainCharacter.position.y +50 }, currentMap.currentMapID);
        Inventory_SpawnItem(ITEM_BOOK_1, (Vector2){ mainCharacter.position.x + 150, mainCharacter.position.y }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_BOOK_2, (Vector2){ mainCharacter.position.x + 150, mainCharacter.position.y +50 }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_BOOK_3, (Vector2){ mainCharacter.position.x + 150, mainCharacter.position.y +100 }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_BOOK_4, (Vector2){ mainCharacter.position.x + 200, mainCharacter.position.y }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_BOOK_5, (Vector2){ mainCharacter.position.x + 200, mainCharacter.position.y +50 }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_BOOK_6, (Vector2){ mainCharacter.position.x + 200, mainCharacter.position.y +100 }, currentMap.currentMapID);
        Inventory_SpawnItem(ITEM_NOTE_PAPER, (Vector2){ mainCharacter.position.x + 250, mainCharacter.position.y }, currentMap.currentMapID); 
        Inventory_SpawnItem(ITEM_KEY_SPECIAL, (Vector2){ mainCharacter.position.x + 250, mainCharacter.position.y + 50 }, currentMap.currentMapID); 
    }

    // Cập nhật logic nhặt đồ (Tính toán hitbox chuẩn từ Player)
    Rectangle playerHitbox = { 
        mainCharacter.position.x + (mainCharacter.drawWidth - mainCharacter.hitWidth) / 2.0f,  
        mainCharacter.position.y + mainCharacter.drawHeight - mainCharacter.hitHeight - 2.0f,    
        mainCharacter.hitWidth,                                        
        mainCharacter.hitHeight                                     
    };
    Inventory_UpdateItemsOnMap(playerHitbox, currentMap.currentMapID);

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
        Inventory_DrawItemsOnMap(currentMap.currentMapID);
    EndMode2D(); 

    // 2. VẼ UI (Không chịu ảnh hưởng Camera)
    Interact_DrawUI(&mainCharacter, npcList, npcCount, &currentMap);
    
    if (Inventory_IsActive()) { 
        Inventory_Draw(); 
        Menu_Draw();
        Debug_RunMenuTool(); // Tool debug chạy kèm khi mở túi
    }

    if (Inventory_IsActive()) {
        Debug_RunMenuTool(); 
    }
    DrawTextEx(globalFont, "F11: Toàn màn hình", (Vector2){10, (float)GetScreenHeight() - 30}, 20, 1, LIGHTGRAY);
    DrawTextEx(globalFont, "F1: Thư viện | F2: Nhà Võ", (Vector2){10, 10}, 24, 1, PURPLE);
    
    const char* hpText = TextFormat("hp: %d/%d", mainCharacter.stats.currentHp, mainCharacter.stats.maxHp);
    DrawTextEx(globalFont, hpText, (Vector2){10, 40}, 24, 1, GREEN);
    //vẽ chữ nhặt item
    BeginMode2D(gameCamera);
    Inventory_DrawNotifications();
    EndMode2D();
}

void Gameplay_Shutdown() {
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    // Nếu NPC có load texture riêng thì unload ở đây
}
// Thay thế 2 hàm Get/Load cũ bằng 2 hàm này ở cuối file gameplay.c

void Gameplay_SaveGame() {
    // Gọi trực tiếp từ save_system
    Game_Save(currentMap.currentMapID, mainCharacter.position, &mainCharacter);
}

void Gameplay_LoadGame() {
    int savedMapID;
    Vector2 savedPos;
    
    // Game_Load sẽ tự động nạp lại stats, cbcStats và pClass vào mainCharacter
    if (Game_Load(&savedMapID, &savedPos, &mainCharacter)) {
        
        // 1. Load lại hình ảnh nhân vật theo đúng Class đã lưu
        Gameplay_SetPlayerClass(mainCharacter.pClass);
        
        // 2. Chuyển Map và đặt nhân vật về tọa độ cũ
        Transition_StartToMap(savedMapID, savedPos);
    }
}