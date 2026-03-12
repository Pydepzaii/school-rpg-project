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

// --- BIẾN ENDING GAME ---
bool playIntroAgain = false; // Tín hiệu báo cho main.c phát lại Intro
static bool isEndingActive = false;
static float endingAlpha = 0.0f;
static float creditsY = 0.0f;
static Music endingMusic = {0};

void Gameplay_StartEnding() {
    isEndingActive = true;
    endingAlpha = 0.0f;
    creditsY = SCREEN_HEIGHT; // Chữ bắt đầu dâng lên từ đáy màn hình
    Audio_StopMusic(MUSIC_LAB);
    // Nạp và phát nhạc Ending
    if (endingMusic.ctxType == 0) {
        if (FileExists("resources/intro/ending_theme.mp3")) {
            endingMusic = LoadMusicStream("resources/intro/ending_theme.mp3"); 
            endingMusic.looping = false;
        }
    }
    if (endingMusic.ctxType != 0) PlayMusicStream(endingMusic);
}

// [MỚI] Hàm vẽ Ending (Sẽ gọi từ main.c)
void Gameplay_DrawEnding() {
    if (isEndingActive) {
        // Phủ đen màn hình
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, endingAlpha));
        
        // Vẽ dòng chữ Credits trôi lên
        if (endingAlpha >= 1.0f) {
           // Vẽ dòng chữ Credits trôi lên
        if (endingAlpha >= 1.0f) {
            const char* creditsText = 
                u8"CẢM ƠN BẠN ĐÃ TRẢI NGHIỆM!\n\n\n\n"
                u8"Một hành trình dài đã khép lại, \nsự thật cuối cùng cũng được phơi bày...\n\n\n\n\n\n\n\n"

                u8"---------------------------------------\n"
                u8"       DỰ ÁN MÔN HỌC OSG202      \n"
                u8"---------------------------------------\n\n"
                u8"Tạo bởi nhóm:\n"
                u8"CÓC THÁCH ĐÂU\n\n\n\n\n\n\n\n"

                u8"------------------------------------------\n"
                u8"      ĐỘI NGŨ PHÁT TRIỂN CHÍNH   \n"
                u8"------------------------------------------\n\n"
                u8"[ NHÓM LẬP TRÌNH - DEV TEAM ]\n\n"
                u8"Hà Quốc Trung (HE204213)\n"
                u8"Hoàng Văn Nam (HE204363)\n"
                u8"Hoàng Tuấn Đạt (HE204119)\n\n\n\n"
                u8"[ NHÓM ĐỒ HỌA - ART TEAM ]\n\n"
                u8"Nguyễn Xuân Dương (HE204027)\n"
                u8"Nghiêm Thị Thu Hoài (HE204322)\n\n\n\n\n\n\n\n"

                u8"-------------------------------------\n"
                u8"      CÔNG NGHỆ NỀN TẢNG         \n"
                u8"-------------------------------------\n\n"
                u8"Ngôn ngữ lập trình: C\n\n"
                u8"Thư viện đồ họa & UI:\n"
                u8"Raylib - Raygui\n\n\n\n\n\n\n\n"

                u8"--------------------------------------\n"
                u8"        LẬP TRÌNH HỆ THỐNG       \n"
                u8"--------------------------------------\n\n"
                u8"Lập trình Cốt lõi & Main Loop\n"
                u8"Hà Quốc Trung\n\n\n"
                u8"Hệ thống Render 2.5D & Animation\n"
                u8"Hà Quốc Trung\n\n\n"
                u8"Logic Kịch Bản & Cốt Truyện\n"
                u8"Hà Quốc Trung\n\n\n"
                u8"Hệ thống Túi Đồ (Inventory System)\n"
                u8"Hà Quốc Trung\n\n\n"
                u8"Lập trình Giao diện UI & Intro/Outro\n"
                u8"Hà Quốc Trung\n\n\n"
                u8"Hệ thống Câu hỏi & Chiến đấu Vấn Đáp\n"
                u8"Hoàng Văn Nam\n\n\n"
                u8"Lập trình Setting Cơ bản\n"
                u8"Hà Quốc Trung - Hoàng Văn Nam\n\n\n"
                u8"Quản lý Nguồn & GitHub Manager\n"
                u8"Hoàng Tuấn Đạt\n\n\n"
                u8"Tối ưu hóa (Fix & Debug)\n"
                u8"Hà Quốc Trung\n\n\n\n\n\n\n\n"

                u8"------------------------------------------\n"
                u8"      THIẾT KẾ GAME & KỊCH BẢN   \n"
                u8"------------------------------------------\n\n"
                u8"Thiết kế Cơ chế Chiến đấu\n"
                u8"Nguyễn Xuân Dương\n\n\n"
                u8"Cân bằng Mức độ & Phân lớp Nhân vật\n"
                u8"Hoàng Tuấn Đạt - Hoàng Văn Nam\n\n\n"
                u8"Biên kịch Cốt truyện chính\n"
                u8"Hoàng Tuấn Đạt - Nguyễn Xuân Dương\n\n\n"
                u8"Xây dựng Lời thoại Nhân vật\n"
                u8"Hoàng Tuấn Đạt - Nguyễn Xuân Dương\n\n\n"
                u8"Sơ đồ luồng Kịch bản & Game Design Doc\n"
                u8"Hoàng Tuấn Đạt\n\n\n\n\n\n\n\n"

                u8"-------------------------------------\n"
                u8"      NGHỆ THUẬT & ĐỒ HỌA        \n"
                u8"-------------------------------------\n\n"
                u8"Thiết kế Môi trường (Map Design)\n"
                u8"Nguyễn Xuân Dương\n\n\n"
                u8"Thiết kế Lớp ảnh Bản đồ (Map Layer)\n"
                u8"Nghiêm Thị Thu Hoài\n\n\n"
                u8"Thiết kế Nhân vật Chính (Player)\n"
                u8"Nguyễn Xuân Dương\n\n\n"
                u8"Thiết kế Nhân vật Phụ (NPC)\n"
                u8"Nguyễn Xuân Dương - Nghiêm Thị Thu Hoài\n\n\n"
                u8"Thiết kế Giao diện UI & Vật phẩm (Item)\n"
                u8"Nguyễn Xuân Dương - Hoàng Tuấn Đạt\n\n\n\n\n\n\n\n"

                u8"-------------------------------------------\n"
                u8"      CÔNG CỤ PHÁT TRIỂN NỘI BỘ  \n"
                u8"-------------------------------------------\n\n"
                u8"Develop Game Debug Tool (DGDT)\n"
                u8"Phát triển bởi: Hà Quốc Trung\n\n\n"
                u8"Tích hợp Va chạm Map qua DGDT\n"
                u8"Nguyễn Xuân Dương\n\n\n"
                u8"Kiến tạo Render 2.5D qua DGDT\n"
                u8"Nguyễn Xuân Dương\n\n\n"
                u8"Triển khai Thoại & Sự kiện qua DGDT\n"
                u8"Hoàng Tuấn Đạt\n\n\n"
                u8"Phát triển Menu In-game qua DGDT\n"
                u8"Hà Quốc Trung\n\n\n\n\n\n\n\n"

                u8"--------------------------------------\n"
                u8"         LỜI CẢM ƠN TỪ NHÓM      \n"
                u8"--------------------------------------\n\n"
                u8"Một lần nữa, nhóm CÓC THÁCH ĐÂU xin gửi\n"
                u8"lời tri ân sâu sắc nhất tới thầy cô\n"
                u8"và các bạn đã ủng hộ tựa game này.\n\n\n\n"
                u8"Dù còn nhiều thiếu sót, nhưng đây là\n"
                u8"tâm huyết và nỗ lực không ngừng nghỉ\n"
                u8"của toàn bộ thành viên trong đội.\n\n\n\n\n\n\n\n\n\n"
                u8"Trân trọng,\n"
                u8"CÓC THÁCH ĐÂU.\n\n\n\n";
                u8"THANK YOU FOR PLAYING.";
            
            // ÉP TỌA ĐỘ VẼ ĐỂ CĂN GIỮA VÀ TRÔI (Sử dụng MeasureText để tự động canh giữa bất chấp độ dài)
            DrawTextEx(globalFont, creditsText, (Vector2){ SCREEN_WIDTH/2 - 190, creditsY }, 26, 1, Fade(WHITE, 0.9f));
            // ========================================================
            // [MỚI] VẼ DÒNG VÉT-ĐÉT "THANK YOU FOR PLAYING" Ở DƯỚI CÙNG
            // ========================================================
            // Đo xem toàn bộ khối chữ cũ cao bao nhiêu
            Vector2 tSize = MeasureTextEx(globalFont, creditsText, 26, 1);
            
            // Tính Tọa độ Y của dòng cuối (Nằm dưới khối chữ cũ + cách ra 150px)
            float lastLineY = creditsY + tSize.y + 150.0f; 
            
            const char* finalLine = u8"THANK YOU FOR PLAYING";
            
            // Đo chiều ngang của dòng cuối để tự động căn chính giữa màn hình
            Vector2 finalSize = MeasureTextEx(globalFont, finalLine, 36, 1);
            float finalLineX = (SCREEN_WIDTH - finalSize.x) / 2.0f;

            // Đổ bóng đen (lùi xuống 2px) tạo độ dày
            DrawTextEx(globalFont, finalLine, (Vector2){ finalLineX + 2, lastLineY + 2 }, 36, 1, Fade(BLACK, 0.8f));
            
            // Vẽ chữ VÀNG sáng chói cỡ 36
            DrawTextEx(globalFont, finalLine, (Vector2){ finalLineX, lastLineY }, 36, 1, YELLOW);
        }
        }
    }
}

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
    // [MỚI] KHÓA TOÀN BỘ GAME ĐỂ CHẠY KỸ XẢO ENDING
    if (isEndingActive) {
        if (endingMusic.ctxType != 0) UpdateMusicStream(endingMusic);

        if (endingAlpha < 1.0f) {
            endingAlpha += GetFrameTime() * 0.5f; // Tối dần màn hình
        } else {
            creditsY -= GetFrameTime() * 25.0f; // Tốc độ trôi chữ
            
            if (creditsY < -6600.0f) { // Khi chữ trôi hết lên đỉnh
                isEndingActive = false;
                if (endingMusic.ctxType != 0) StopMusicStream(endingMusic);
                
                // Gọi Transition để từ từ làm đen màn hình thay vì cắt phụt 1 cái
                extern void Transition_EndingToIntro();
                Transition_EndingToIntro();
            }
        }
        return; // Chặn không cho nhân vật chạy đi đâu nữa
    }
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
    
    const char* hpText = TextFormat("HP: %d/%d", mainCharacter.cbcStats.hp, mainCharacter.cbcStats.maxHp);
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
    
    // Game_Load sẽ đọc file và nạp dữ liệu vào mainCharacter
    if (Game_Load(&savedMapID, &savedPos, &mainCharacter)) {
        
        // 1. [QUAN TRỌNG] LƯU TẠM CHỈ SỐ VỪA LOAD ĐƯỢC
        // Để tránh bị hàm InitPlayer() bên dưới xóa mất
        PlayerStats savedStats = mainCharacter.stats;
        CBC_Stats savedCbc = mainCharacter.cbcStats;
        int savedClass = mainCharacter.pClass;
        
        // 2. Load lại hình ảnh nhân vật (Hàm này sẽ làm reset chỉ số)
        Gameplay_SetPlayerClass(savedClass);
        
        // 3. GÁN NGƯỢC LẠI CHỈ SỐ ĐỂ BẢO TOÀN CỐT TRUYỆN VÀ MÁU
        mainCharacter.stats = savedStats;
        mainCharacter.cbcStats = savedCbc;
        mainCharacter.pClass = savedClass;
        
        // 4. Chuyển Map và đặt nhân vật về tọa độ cũ
        Transition_StartToMap(savedMapID, savedPos);
    }
}
// [MỚI] Hàm kiểm tra xem có đang chạy Ending không
bool Gameplay_IsEnding() {
    extern bool isEndingActive; // Khai báo lấy biến cục bộ
    return isEndingActive;
}