#include "gameplay.h"
#include "raylib.h"
#include "settings.h"
#include "combat.h"
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
#include <string.h>
#include <math.h>

static Player mainCharacter;
static GameMap currentMap;
static Npc npcList[MAX_NPCS];
static int npcCount = 0;
// --- BIẾN KỸ XẢO DARK ENDING ---
bool isDarkEndingCutscene = false;
bool isSpawningDarkBoss = false;
float darkBossSpawnTimer = 0.0f;
bool isBlackholeActive = false;
float blackholeTimer = 0.0f;
static Texture2D texAuraBoss = {0};
static Texture2D texBlackhole = {0};

void Gameplay_SpawnDarkEndingNPCs()
{

    // Boss Quốc Trung (ID 11, Ảnh bà già)
    InitNpc(&npcList[npcCount], MAP_LAB, "resources/npc/map_easter_egg/ba_gia.png", (Vector2){400, 160}, "Quốc TrungECHO", 11);
    strcpy(npcList[npcCount].dialogKey, "BEGIN_ENDING_2");
    npcCount++;

    // Bảo vệ 1 (ID 14)
    InitNpc(&npcList[npcCount], MAP_LAB, "resources/npc/map_alpha/baove1.png", (Vector2){300, 150}, "Sy quan ECHO 1", 14);
    strcpy(npcList[npcCount].dialogKey, "DEFAULT");
    npcCount++;

    // Bảo vệ 2 (ID 15)
    InitNpc(&npcList[npcCount], MAP_LAB, "resources/npc/map_alpha/baove2.png", (Vector2){500, 150}, "Sy quan ECHO 2", 15);
    strcpy(npcList[npcCount].dialogKey, "DEFAULT");
    npcCount++;
}

// --- BIẾN ENDING GAME ---
bool playIntroAgain = false; // Tín hiệu báo cho main.c phát lại Intro
static bool isEndingActive = false;
static float endingAlpha = 0.0f;
static float creditsY = 0.0f;
static Music endingMusic = {0};
static EndingType currentEndingType = ENDING_TRUE; // [MỚI] Lưu loại Ending
static bool isWaitingForClick = false;
// --- BIẾN CHO DARK ENDING CUTSCENE ---
static Texture2D texDarkCutscene = {0};
static int darkCutscenePhase = 0; // 0: Tắt, 1: Đang hiện thoại, 2: Xong (chuyển qua Credit)
static int currentDarkLine = 0;   // [MỚI] Cờ chờ click chuột
void Gameplay_StartEnding(EndingType type)
{
    isEndingActive = true;
    currentEndingType = type;
    isWaitingForClick = false;
    endingAlpha = 0.0f;
    creditsY = SCREEN_HEIGHT;
    Audio_StopMusic(MUSIC_LAB);

    // Nạp nhạc
    if (endingMusic.ctxType == 0)
    {
        if (FileExists("resources/intro/ending_theme.mp3"))
        {
            endingMusic = LoadMusicStream("resources/intro/ending_theme.mp3");
            endingMusic.looping = false;
        }
    }
    if (endingMusic.ctxType != 0)
        PlayMusicStream(endingMusic);

    // KÍCH HOẠT CUTSCENE NẾU LÀ DARK ENDING
    if (type == ENDING_DARK)
    {
        darkCutscenePhase = 1; // Bật phase chiếu cutscene
        currentDarkLine = 0;
        // ĐẢM BẢO ĐƯỜNG DẪN ẢNH NÀY ĐÚNG VỚI FOLDER CỦA BẠN NHÉ
        if (texDarkCutscene.id == 0)
        {
            texDarkCutscene = LoadTexture("resources/intro/echo_ending.png");
        }
    }
    else
    {
        darkCutscenePhase = 0; // Ending khác thì không bật
    }
}

void Gameplay_DrawEnding()
{
    if (isEndingActive)
    {
        // Phủ đen màn hình
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, endingAlpha));

        // [MỚI] VẼ MÀN HÌNH CHỜ CLICK SAU KHI CREDIT CHẠY XONG
        if (isWaitingForClick)
        {
            const char *endText = "ENDING";
            Color endColor = WHITE;

            if (currentEndingType == ENDING_BAD)
            {
                endText = "BAD ENDING";
                endColor = RED;
            }
            else if (currentEndingType == ENDING_TRUE)
            {
                endText = "GOOD ENDING";
                endColor = GOLD;
            }
            else if (currentEndingType == ENDING_DARK)
            {
                endText = "DARK ENDING\nTRUE ENDING";
                endColor = PURPLE;
            }

            // Vẽ tiêu đề Ending bự ở giữa
            Vector2 tSize = MeasureTextEx(globalFont, endText, 60, 1);
            DrawTextEx(globalFont, endText, (Vector2){(SCREEN_WIDTH - tSize.x) / 2.0f, SCREEN_HEIGHT / 2.0f - 50.0f}, 60, 1, endColor);

            // Vẽ dòng nhắc nhở nhấp nháy
            const char *promptText = u8"Nhấp chuột trái để trở về Menu...";
            Vector2 pSize = MeasureTextEx(globalFont, promptText, 24, 1);
            if ((int)(GetTime() * 2) % 2 == 0)
            {
                DrawTextEx(globalFont, promptText, (Vector2){(SCREEN_WIDTH - pSize.x) / 2.0f, SCREEN_HEIGHT / 2.0f + 50.0f}, 24, 1, GRAY);
            }
            return; // Dừng lại không vẽ Credit nữa
        }

        // [MỚI] VẼ CUTSCENE DARK ENDING TRƯỚC KHI CREDIT TRÔI
        if (endingAlpha >= 1.0f && currentEndingType == ENDING_DARK && darkCutscenePhase == 1)
        {
            // 1. Vẽ bức ảnh ECHO logo ở giữa màn hình
            if (texDarkCutscene.id != 0)
            {
                // Tỉ lệ scale ảnh cho vừa màn hình (tùy bạn chỉnh)
                float scale = 0.8f;
                Vector2 imgPos = {
                    (SCREEN_WIDTH - texDarkCutscene.width * scale) / 2.0f,
                    (SCREEN_HEIGHT - texDarkCutscene.height * scale) / 2.0f - 50};
                DrawTextureEx(texDarkCutscene, imgPos, 0.0f, scale, WHITE);
            }

            // 2. Các dòng thoại kịch bản
            const char *lines[] = {
                u8"Id cơ sở dẫ biến mất",
                u8"Xác nhận GP11642561 đã bị lô",
                u8"Chuyển thông báo về toàn bộ sở chỉ huy khu vực",
                u8"Chiến dịch ECHO The NEW WORLD BẮT ĐẦU"};

            // Vẽ hộp thoại nền đen mờ ở dưới đáy
            DrawRectangle(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120, Fade(BLACK, 0.8f));

            // Vẽ dòng thoại hiện tại căn giữa
            Vector2 textSize = MeasureTextEx(globalFont, lines[currentDarkLine], 28, 1);
            DrawTextEx(globalFont, lines[currentDarkLine], (Vector2){(SCREEN_WIDTH - textSize.x) / 2.0f, SCREEN_HEIGHT - 80}, 28, 1, WHITE);

            // Nhắc nhở click chuột
            DrawTextEx(globalFont, u8"Click để tiếp tục...", (Vector2){SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30}, 20, 1, GRAY);

            return; // Đang chiếu thoại thì không vẽ chữ Credit
        }

        // NẾU CHƯA CHỜ CLICK VÀ KHÔNG CÓ CUTSCENE -> VẼ CREDIT TRÔI NHƯ CŨ
        if (endingAlpha >= 1.0f)
        {
            const char *creditsText =
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
            DrawTextEx(globalFont, creditsText, (Vector2){SCREEN_WIDTH / 2 - 190, creditsY}, 26, 1, Fade(WHITE, 0.9f));
            // ========================================================
            // [MỚI] VẼ DÒNG VÉT-ĐÉT "THANK YOU FOR PLAYING" Ở DƯỚI CÙNG
            // ========================================================
            // Đo xem toàn bộ khối chữ cũ cao bao nhiêu
            Vector2 tSize = MeasureTextEx(globalFont, creditsText, 26, 1);

            // Tính Tọa độ Y của dòng cuối (Nằm dưới khối chữ cũ + cách ra 150px)
            float lastLineY = creditsY + tSize.y + 150.0f;

            const char *finalLine = u8"THANK YOU FOR PLAYING";

            // Đo chiều ngang của dòng cuối để tự động căn chính giữa màn hình
            Vector2 finalSize = MeasureTextEx(globalFont, finalLine, 36, 1);
            float finalLineX = (SCREEN_WIDTH - finalSize.x) / 2.0f;

            // Đổ bóng đen (lùi xuống 2px) tạo độ dày
            DrawTextEx(globalFont, finalLine, (Vector2){finalLineX + 2, lastLineY + 2}, 36, 1, Fade(BLACK, 0.8f));

            // Vẽ chữ VÀNG sáng chói cỡ 36
            DrawTextEx(globalFont, finalLine, (Vector2){finalLineX, lastLineY}, 36, 1, YELLOW);
        }
    }
}

// --- 2. CÀI ĐẶT CÁC HÀM ---

void Gameplay_Init()
{
    // Setup Objects
    InitPlayer(&mainCharacter, CLASS_STUDENT);
    mainCharacter.position = (Vector2){400, 300};
    currentMap.texture.id = 0;
    LoadMap(&currentMap, MAP_TOA_ALPHA);

    npcCount = 0;
    Npc_LoadForMap(MAP_TOA_ALPHA, npcList, &npcCount);

    // Đảm bảo nhạc đúng map
    Audio_PlayMusicForMap(MAP_TOA_ALPHA);
}
// Hàm đổi class nhân vật (Gọi từ Menu)
void Gameplay_SetPlayerClass(int classID)
{
    // 1. Xóa ảnh nhân vật cũ khỏi RAM
    UnloadPlayer(&mainCharacter);

    // 2. Tạo lại nhân vật mới với ID class mới
    InitPlayer(&mainCharacter, classID);

    // 3. Đặt vị trí xuất phát
    mainCharacter.position = (Vector2){400, 300};
    // [FIX]: Xóa sạch đồ ván cũ và sinh lại đồ ván mới
    Inventory_Reset(); 
    Inventory_SpawnItem(ITEM_BOOK_1, (Vector2){200, 65}, MAP_TOA_ALPHA);
    Inventory_SpawnItem(ITEM_BOOK_2, (Vector2){20, 400}, MAP_NHA_VO);
    Inventory_SpawnItem(ITEM_BOOK_3, (Vector2){280, 140}, MAP_NHA_AN);
    Inventory_SpawnItem(ITEM_BOOK_4, (Vector2){170, 50}, MAP_THU_VIEN);
    Inventory_SpawnItem(ITEM_BOOK_5, (Vector2){480, 225}, MAP_BETA);
    Inventory_SpawnItem(ITEM_BOOK_6, (Vector2){732, 220}, MAP_LAB);
}

void Gameplay_Update()
{
    // [MỚI] NẾU ĐANG TRONG TRẬN ĐÁNH THÌ KHÓA MỌI THỨ VÀ CHẠY COMBAT
    extern bool Combat_IsActive();
    extern void Combat_Update();
    if (Combat_IsActive())
    {
        Combat_Update();
        return;
    }
    // [MỚI] KHÓA TOÀN BỘ GAME ĐỂ CHẠY KỸ XẢO ENDING
    if (isEndingActive)
    {
        if (endingMusic.ctxType != 0)
            UpdateMusicStream(endingMusic);

       // [MỚI] NẾU ĐÃ TRÔI XONG CREDIT VÀ ĐANG CHỜ CLICK CHUỘT
        if (isWaitingForClick) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isEndingActive = false;
                isWaitingForClick = false;
                
                // --- DỌN RÁC BỘ NHỚ KHI XONG ENDING ---
                if (endingMusic.ctxType != 0) {
                    StopMusicStream(endingMusic);
                    UnloadMusicStream(endingMusic); // Giải phóng nhạc
                    endingMusic.ctxType = 0;        // Reset id
                }
                if (texDarkCutscene.id != 0) {
                    UnloadTexture(texDarkCutscene); // Giải phóng ảnh Cutscene
                    texDarkCutscene.id = 0;         // Reset id
                }
                
                extern void Transition_EndingToIntro();
                Transition_EndingToIntro();
            }
            return; // Khóa vòng lặp tại đây
        }

        if (endingAlpha < 1.0f)
        {
            endingAlpha += GetFrameTime() * 0.5f; // Tối dần màn hình
        }
        else
        {
            // [MỚI] NẾU LÀ DARK ENDING VÀ ĐANG Ở PHASE CUTSCENE THÌ CHẶN CREDIT LẠI
            if (currentEndingType == ENDING_DARK && darkCutscenePhase == 1)
            {
                // Chờ click chuột để chuyển dòng thoại
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    currentDarkLine++;
                    if (currentDarkLine > 2)
                    {                          // Giả sử có 3 dòng thoại (0, 1, 2)
                        darkCutscenePhase = 2; // Xong thoại, cho phép trôi Credit
                    }
                }
                return; // KHÓA VÒNG LẶP: Không cho chữ trôi lúc đang chiếu thoại
            }

            // NẾU KHÔNG PHẢI CUTSCENE THÌ TRÔI CHỮ BÌNH THƯỜNG
            creditsY -= GetFrameTime() * 25.0f; // Tốc độ trôi chữ

            if (creditsY < -6600.0f)
            { // Khi chữ trôi hết lên đỉnh
                // [SỬA] BẬT CỜ CHỜ CLICK THAY VÌ CHUYỂN CẢNH VỀ MENU NGAY
                isWaitingForClick = true;
            }
        }
        return; // Chặn không cho nhân vật chạy đi đâu nữa
    }
    // Chỉ update khi không chuyển cảnh
    if (!Transition_IsActive())
    {
        Inventory_Update();
        if (!Inventory_IsActive() && !IsDialogDebugActive() && !isShowingSecretMap)
        {
            UpdatePlayer(&mainCharacter, &currentMap, npcList, npcCount);

            for (int i = 0; i < npcCount; i++)
            {
                if (npcList[i].mapID == currentMap.currentMapID)
                    UpdateNpc(&npcList[i]);
            }
            Interact_Update(&mainCharacter, npcList, npcCount, &currentMap);
            Story_Update(&mainCharacter, &currentMap, npcList, npcCount);
            // Cập nhật Camera bám theo nhân vật
            Camera_Update(&mainCharacter, &currentMap);
        }
    }
    // Cập nhật chuyển cảnh (Transition cần dữ liệu để load map mới nếu có lệnh chuyển)
    Transition_Update(&currentMap, &mainCharacter, npcList, &npcCount);
    // Item
    //  Cập nhật logic nhặt đồ (Tính toán hitbox chuẩn từ Player)
    Rectangle playerHitbox = {
        mainCharacter.position.x + (mainCharacter.drawWidth - mainCharacter.hitWidth) / 2.0f,
        mainCharacter.position.y + mainCharacter.drawHeight - mainCharacter.hitHeight - 2.0f,
        mainCharacter.hitWidth,
        mainCharacter.hitHeight};
    Inventory_UpdateItemsOnMap(playerHitbox, currentMap.currentMapID);
}

void Gameplay_Draw()
{
    // 1. VẼ THẾ GIỚI GAME (Có Camera)
    BeginMode2D(gameCamera); // gameCamera lấy từ camera.h

    DrawMap(&currentMap);

    // Renderer (Y-Sorting)
    Render_Clear();
    Render_AddPlayer(&mainCharacter);
    for (int i = 0; i < npcCount; i++)
    {
        if (npcList[i].mapID == currentMap.currentMapID)
            Render_AddNpc(&npcList[i]);
    }
    // [NEW] Add Props vào Renderer
    for (int i = 0; i < currentMap.propCount; i++)
    {
        // Tính toán sortY: Vị trí Y + Chiều cao (Đáy ảnh)
        float sortY = currentMap.props[i].position.y + currentMap.props[i].originY;
        Render_AddProp(&currentMap.props[i]);
    }
    Render_DrawAll();
    // --- [MỚI] KỸ XẢO DARK ENDING ĐÈ LÊN MAP ---
    if (isSpawningDarkBoss)
    {
        if (texAuraBoss.id == 0)
            texAuraBoss = LoadTexture("resources/menu/aura.png");
        darkBossSpawnTimer += GetFrameTime();

        // Tính toán tiến trình (0.0 đến 1.0) trong 1.5 giây
        float p = darkBossSpawnTimer / 1.5f;
        if (p > 1.0f)
            p = 1.0f;

        float scale = p * 4.0f; // Vòng phép phình to cực nhanh (không dùng sin nữa)

        Vector2 spawnPos = {396, 130};
        float drawSize = 150.0f * scale;
        Rectangle destAura = {spawnPos.x + 25, spawnPos.y + 40, drawSize, drawSize};
        Vector2 origin = {drawSize / 2.0f, drawSize / 2.0f};

        // Tính toán độ chói lóa ở những frame cuối (Flash)
        Color auraColor = Fade(PURPLE, 1.0f - p);                              // Hạt Aura mờ dần
        Color flashColor = Fade(WHITE, (p > 0.8f) ? (1.0f - p) * 5.0f : 0.0f); // Nổ chói lóa trắng xóa

        BeginBlendMode(BLEND_ADDITIVE);
        // Vẽ vòng xoáy
        DrawTexturePro(texAuraBoss, (Rectangle){0, 0, texAuraBoss.width, texAuraBoss.height}, destAura, origin, darkBossSpawnTimer * 1200.0f, auraColor);

        // Cú nổ sáng trắng ở 0.3s cuối
        if (p > 0.8f)
        {
            DrawCircleV((Vector2){spawnPos.x + 25, spawnPos.y + 40}, drawSize / 1.5f, flashColor);
        }
        EndBlendMode();

        if (darkBossSpawnTimer > 1.5f)
        { // Triệu hồi hoàn tất nhanh gọn
            isSpawningDarkBoss = false;
            Gameplay_SpawnDarkEndingNPCs();
        }
    }

    // --- KỸ XẢO HỐ ĐEN HỦY DIỆT ---
    if (isBlackholeActive)
    {
        if (texBlackhole.id == 0)
            texBlackhole = LoadTexture("resources/blackhole.png");
        blackholeTimer += GetFrameTime();
        float p = blackholeTimer;

        Vector2 htPos = {396, 231};                    // Vị trí Hiệu Trưởng
        Vector2 center = {htPos.x + 25, htPos.y + 25}; // Tâm vụ nổ

        // 1. Giai đoạn Hố đen (0.0s -> 2.0s)
        if (p < 2.0f)
        {
            float scale = (p < 1.0f) ? (p * 2.5f) : ((2.0f - p) * 2.5f);
            if (scale < 0)
                scale = 0;

            float drawSize = 180.0f * scale;
            Rectangle destBH = {center.x, center.y, drawSize, drawSize};
            Vector2 origin = {drawSize / 2.0f, drawSize / 2.0f};

            // Vẽ hố đen xoay tròn cực mạnh
            DrawTexturePro(texBlackhole, (Rectangle){0, 0, texBlackhole.width, texBlackhole.height}, destBH, origin, p * 1500.0f, WHITE);
        }

        // 2. Chốt Tàng hình Hiệu Trưởng lúc hố đen đạt đỉnh (1.0s)
        if (p >= 1.0f && p < 1.1f)
        {
            for (int i = 0; i < npcCount; i++)
            {
                if (npcList[i].id == 12 && npcList[i].texture.width != 1)
                {
                    Image blankImg = GenImageColor(1, 1, BLANK);
                    npcList[i].texture = LoadTextureFromImage(blankImg);
                    UnloadImage(blankImg);
                }
            }
        }

        // 3. Giai đoạn 4 Thanh Đỏ đâm vào tâm (1.0s -> 2.0s - Lúc hố đen đang thu nhỏ)
        if (p >= 1.0f && p < 2.0f)
        {
            float t = p - 1.0f;               // t chạy từ 0.0 -> 1.0
            float dist = 400.0f * (1.0f - t); // Khoảng cách từ ngoài màn hình lao vào tâm
            float length = 120.0f;            // Chiều dài tia laser đỏ
            float thickness = 6.0f;           // Độ dày

            // 4 góc chéo: 45, 135, 225, 315 độ (Tính bằng Radian)
            float angles[4] = {PI / 4, 3 * PI / 4, 5 * PI / 4, 7 * PI / 4};
            for (int i = 0; i < 4; i++)
            {
                Vector2 startPos = {center.x + cos(angles[i]) * (dist + length), center.y + sin(angles[i]) * (dist + length)};
                Vector2 endPos = {center.x + cos(angles[i]) * dist, center.y + sin(angles[i]) * dist};

                // Vẽ tia sáng đỏ rực
                DrawLineEx(startPos, endPos, thickness, RED);
                // Vẽ thêm viền sáng trắng bên trong tia đỏ cho ngầu
                DrawLineEx(startPos, endPos, thickness * 0.4f, WHITE);
            }
        }

        // 4. Giai đoạn Vụ Nổ Màu Đen Hủy Diệt (2.0s -> 2.5s)
        if (p >= 2.0f && p < 2.5f)
        {
            float t = p - 2.0f;        // t chạy từ 0.0 -> 0.5
            float progress = t / 0.5f; // Chuẩn hóa về 0.0 -> 1.0

            // Vụ nổ bùng to ra 800 pixel bao trùm bản đồ
            float explosionRadius = progress * 800.0f;
            Color explosionColor = Fade(BLACK, 1.0f - progress); // Đậm màu rồi nhạt dần

            DrawCircleV(center, explosionRadius, explosionColor);
            // Một luồng sóng xung kích viền đỏ bùng ra theo
            DrawCircleLines((int)center.x, (int)center.y, explosionRadius * 1.1f, Fade(RED, 1.0f - progress));
        }

        // Kết thúc toàn bộ chuỗi kỹ xảo
        if (p > 2.5f)
            isBlackholeActive = false;
    }
    // ---------------------------------
    // Vẽ Debug (Hitbox)
    Debug_UpdateAndDraw(&currentMap, &mainCharacter, npcList, npcCount);
    Debug_RunPropTool(&currentMap);
    Inventory_DrawItemsOnMap(currentMap.currentMapID);
    EndMode2D();

    // 2. VẼ UI (Không chịu ảnh hưởng Camera)
    Interact_DrawUI(&mainCharacter, npcList, npcCount, &currentMap);

    if (Inventory_IsActive())
    {
        Inventory_Draw();
        Menu_Draw();
        Debug_RunMenuTool(); // Tool debug chạy kèm khi mở túi
    }

    if (Inventory_IsActive())
    {
        Debug_RunMenuTool();
    }

    const char *hpText = TextFormat("HP: %d/%d", mainCharacter.cbcStats.hp, mainCharacter.cbcStats.maxHp);
    DrawTextEx(globalFont, hpText, (Vector2){10, 40}, 24, 1, GREEN);
    // vẽ chữ nhặt item
    BeginMode2D(gameCamera);
    Inventory_DrawNotifications();
    // [MỚI] VẼ MÀN HÌNH COMBAT ĐÈ LÊN TRÊN CÙNG
    extern bool Combat_IsActive();
    extern void Combat_Draw();
    if (Combat_IsActive())
    {
        Combat_Draw();
    }
    EndMode2D();
}

void Gameplay_Shutdown()
{
    UnloadPlayer(&mainCharacter);
    UnloadMap(&currentMap);
    // Nếu NPC có load texture riêng thì unload ở đây
    // Quét dọn bộ nhớ Ending nếu lỡ tắt game đột ngột
    if (endingMusic.ctxType != 0) UnloadMusicStream(endingMusic);
    if (texDarkCutscene.id != 0) UnloadTexture(texDarkCutscene);
}
// Thay thế 2 hàm Get/Load cũ bằng 2 hàm này ở cuối file gameplay.c

void Gameplay_SaveGame()
{
    // Gọi trực tiếp từ save_system
    Game_Save(currentMap.currentMapID, mainCharacter.position, &mainCharacter);
}

void Gameplay_LoadGame()
{
    int savedMapID;
    Vector2 savedPos;

    // Game_Load sẽ đọc file và nạp dữ liệu vào mainCharacter
    if (Game_Load(&savedMapID, &savedPos, &mainCharacter))
    {
        // [THÊM MỚI] DỌN SẠCH TÀN DƯ KỸ XẢO NẾU NGƯỜI CHƠI LOAD LẠI GAME
        // =========================================================
        extern bool isEndingActive; 
        isEndingActive = false;
        isDarkEndingCutscene = false;
        isSpawningDarkBoss = false;
        darkBossSpawnTimer = 0.0f;
        isBlackholeActive = false;
        blackholeTimer = 0.0f;
        // =========================================================

        // 1. [QUAN TRỌNG] LƯU TẠM TOÀN BỘ DỮ LIỆU VỪA LOAD ĐƯỢC TỪ FILE SAVE
        PlayerStats savedStats = mainCharacter.stats;
        CBC_Stats savedCbc = mainCharacter.cbcStats;
        int savedClass = mainCharacter.pClass;
        
        // Lưu tạm cả 4 kỹ năng để không bị reset
        Skill savedSkills[4];
        for (int i = 0; i < 4; i++) {
            savedSkills[i] = mainCharacter.skills[i];
        }

        // 2. [FIX TRIỆT ĐỂ]: Gọi InitPlayer để nạp lại ảnh và khung Animation chuẩn
        UnloadPlayer(&mainCharacter); 
        InitPlayer(&mainCharacter, savedClass); // Hàm này sẽ tự động lo việc chọn đúng ảnh Class

        // 3. GÁN NGƯỢC LẠI CHỈ SỐ ĐỂ BẢO TOÀN CỐT TRUYỆN, MÁU VÀ SKILL
        mainCharacter.stats = savedStats;
        mainCharacter.cbcStats = savedCbc;
        // (Không cần gán lại pClass vì InitPlayer đã làm rồi)
        
        for (int i = 0; i < 4; i++) {
            mainCharacter.skills[i] = savedSkills[i];
        }

        // 4. Chuyển Map và đặt nhân vật về tọa độ cũ
        Transition_StartToMap(savedMapID, savedPos);
    }
}
// [MỚI] Hàm kiểm tra xem có đang chạy Ending không
bool Gameplay_IsEnding()
{
    extern bool isEndingActive; // Khai báo lấy biến cục bộ
    return isEndingActive;
}