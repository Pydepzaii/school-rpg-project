// FILE: src/combat.c
#include "combat.h"
#include "raylib.h"
#include "settings.h"
#include "audio_manager.h"
#include "ui_style.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "raymath.h"

// ========================================================================
// [CẤU HÌNH TỌA ĐỘ ẢNH TỪ UI.PNG]
// ========================================================================
static Rectangle SRC_HEART[] = {
    {146, 579, 13, 10}, {130, 579, 13, 10}, {114, 579, 13, 10}, {98, 579, 13, 10}, {82, 579, 13, 10}, {66, 579, 13, 10}, {50, 579, 13, 10}, {34, 579, 13, 10}, {34, 563, 13, 10}};
static Rectangle SRC_STAMINA[] = {
    {114, 593, 12, 14}, {98, 593, 12, 14}, {82, 593, 12, 14}, {66, 593, 12, 14}, {50, 593, 12, 14}, {34, 593, 12, 14}, {66, 561, 12, 14}};
// NÚT BẤM
static Rectangle SRC_BTN_NORMAL = {97, 337, 62, 29};
static Rectangle SRC_BTN_HOVER = {161, 337, 62, 29};
static Rectangle SRC_BTN_LOCK = {161, 305, 62, 29};
static Rectangle SRC_SLOT_NORMAL = {65, 273, 30, 30};
static Rectangle SRC_SLOT_HOVER = {225, 273, 30, 30};

// ========================================================================

static bool combatActive = false;
static CombatState combatState;
static int currentPhase = 0;
static int combatResult = 0;
static bool hasUsedRevive = false; // Cờ đánh dấu Học Bá đã dùng quyền sửa sai chưa
static float reviveVFXTimer = 0.0f; // Bộ đếm thời gian cho hiệu ứng tỏa sáng hồi sinh
static float stateTimer = 0.0f;
static float fadeAlpha = 0.0f;

static Texture2D texBackground;
static Texture2D texIconUpCrit; // [MỚI] Icon tăng Chí mạng
static Texture2D texCombatUI;
static Texture2D texTitleFrame;
static bool resourcesLoaded = false;
static char messageLog[128] = "";

// --- [NÂNG CẤP] BIẾN QUẢN LÝ LỊCH SỬ LOG ---
#define MAX_LOG_HISTORY 30
static char logHistory[MAX_LOG_HISTORY][128];
static int logCount = 0;
static bool showLogWindow = false;

// --- BIẾN DÀNH CHO KỸ XẢO (VFX) ---
static Texture2D texVoidPunch; // Dành cho chiêu Đột Kích Phân Rã
static Texture2D texPhase2Aura;
static Texture2D texHaoQuangHuyetSac;
static Texture2D texThuHoiVonCoin;
static Texture2D texHuyetTien;
static Texture2D texDauAnKiSinh;
static Texture2D texDoTimConMoi;
static Texture2D texGiaoKeoAcQuy;
static Texture2D texBookVFX;
static Texture2D texBlackHoleVFX, texTelegateVFX, texLaserVFX;
static Texture2D texExplosionVFX;
static Texture2D texPunchVFX;
static Texture2D texBowVFX;
static Texture2D texArrowVFX;
static float screenShakeTimer = 0.0f;
static bool hasDealtDamage = false;
static Texture2D texCoinVFX;
static Texture2D texPhaGia;
static Texture2D texTrietHa;
// biến cho animation skill1 hocba
static Texture2D texPhanUngHoaHoc; // Dành cho chiêu ném độc của Học Bá
static Texture2D texKhienAnimation, texTangCong, texIconTapTrung;
static Texture2D texCauTuTruong;
static Texture2D texTuDuyChienThuat;
static Texture2D texDinhLiCuoiCung;
// biến cho animation skill1 đầu gấu
//chiêu 1
static Texture2D texThornVFX;
static float thornRotation = 0.0f;
static float thornAnimTimer = 0.0f;
static bool isThornAnimating = false;
//chiêu 2
static Texture2D texKeChiuDonIcon;
static Texture2D texKeChiuDonVFX;
//chiêu 3
static Texture2D texLightningIcon;
static Texture2D texLightningVFX;
//chiêu 4
static Texture2D texRungChanIcon;
//chiêu cuối
static Texture2D texUltiDauGau;

#define MAX_FLOATING_TEXTS 15
typedef struct
{
    char text[32];
    Vector2 pos;
    Color color;
    float lifetime;
    float maxLifetime;
    float velocityY;
    bool isCrit;
} FloatingText;

static FloatingText fTexts[MAX_FLOATING_TEXTS];
static Texture2D texIconPoison, texIconSilence, texIconUpHp, texIconUpDef, texIconUpAtk, texIconStun;

// --- [MỚI TẠO] BỘ MÁY HOẠT ẢNH ICON HIỆU ỨNG ---
typedef struct
{
    EffectType type;
    bool isPlayerTarget;
    float timer;
    float maxTime;
    bool active;
} IconAnim;
static IconAnim iconAnims[10];

void SpawnIconAnim(EffectType type, bool isPlayerTarget)
{
    for (int i = 0; i < 10; i++)
    {
        if (!iconAnims[i].active)
        {
            iconAnims[i].type = type;
            iconAnims[i].isPlayerTarget = isPlayerTarget;
            iconAnims[i].timer = 0.8f; // Tổng thời gian bay (0.8s)
            iconAnims[i].maxTime = 0.8f;
            iconAnims[i].active = true;
            break;
        }
    }
}


// Hàm bắn chữ nổi [ĐÃ NÂNG CẤP CHỐNG ĐÈ CHỮ]
void SpawnFloatingText(const char *text, Vector2 pos, Color color, bool isCrit)
{
    float offsetY = 0.0f;
    // Radar quét: Nếu có text nào vừa sinh ra (lifetime > 0.8) ở cùng vị trí X, đẩy text này lên cao hơn
    for (int k = 0; k < MAX_FLOATING_TEXTS; k++) {
        if (fTexts[k].lifetime > 0.8f && fabsf(fTexts[k].pos.x - pos.x) < 50.0f) {
            offsetY -= 35.0f; // Xếp chồng lên nhau cách 35 pixel
        }
    }

    for (int i = 0; i < MAX_FLOATING_TEXTS; i++)
    {
        if (fTexts[i].lifetime <= 0)
        {
            strcpy(fTexts[i].text, text);
            // Áp dụng offsetY vừa tính toán
            fTexts[i].pos = (Vector2){pos.x + GetRandomValue(-15, 15), pos.y + offsetY};
            fTexts[i].color = color;
            fTexts[i].maxLifetime = 1.2f;
            fTexts[i].lifetime = 1.2f;
            fTexts[i].velocityY = -60.0f; // Trôi chậm lại một chút để dễ đọc hơn
            fTexts[i].isCrit = isCrit;
            break;
        }
    }
}

// --- CÁC GIAI ĐOẠN HÀNH ĐỘNG CỦA ĐẦU GẤU ---
typedef enum
{
    ACTION_PHASE_IDLE = 0,
    ACTION_PHASE_DASH,
    ACTION_PHASE_PUNCH,
    ACTION_PHASE_EXPLODE,
    ACTION_PHASE_RETURN
} ActionPhase;

static ActionPhase currentActionPhase = ACTION_PHASE_IDLE;

static CombatEntity *activeAttacker = NULL;
static CombatEntity *activeDefender = NULL;
static CombatSkill *activeSkill = NULL;

static char turnText[64] = "";
static Color turnColor = WHITE;
static float turnAnimTimer = 0.0f;
static bool showTurnAnim = false;

static Texture2D texPhase1Trung, texPhase1BV1, texPhase1BV2;
static bool phase1TexLoaded = false;

static CombatEntity entPlayer, entBoss;
static Player *refPlayer;
static Npc *refBoss;

// --- BIẾN QUẢN LÝ SÁCH KỸ NĂNG 3D ---
typedef enum
{
    CB_BOOK_IDLE = 0,
    CB_BOOK_DROPPING,
    CB_BOOK_PAUSE,
    CB_BOOK_OPEN_RIGHT,
    CB_BOOK_OPEN_LEFT,
    CB_BOOK_FULLY_OPEN,
    CB_BOOK_CLOSE_LEFT,
    CB_BOOK_CLOSE_RIGHT,
    CB_BOOK_EXITING
} CombatBookState;

static CombatBookState cb_bookState = CB_BOOK_IDLE;
static int cb_bookMode = 0;
static float cb_bookProgress = 0.0f;
static float cb_bookDropY = -1000.0f;
static float cb_bookVelocity = 0.0f;
static float cb_pauseTimer = 0.0f;
static float cb_auraRot = 0.0f;

static int cb_titleState = 0;
static float cb_titleYOffset = -600.0f;
static float cb_titleVelocity = 0.0f;
static bool cb_waitingToCloseBook = false;


static Texture2D texAura;
static int selectedBookSkill = -1;

Color CB_GetPageFlipTint(float progress)
{
    unsigned char b = (progress < 0.5f) ? (unsigned char)(255 - (progress * 2.0f * 95)) : (unsigned char)(160 + ((progress - 0.5f) * 2.0f * 95));
    return (Color){b, b, b, 255};
}

Vector2 GetMouseScaled()
{
    float scale = (float)GetScreenWidth() / SCREEN_WIDTH;
    if ((float)GetScreenHeight() / SCREEN_HEIGHT < scale)
        scale = (float)GetScreenHeight() / SCREEN_HEIGHT;
    Vector2 mouse = GetMousePosition();
    return (Vector2){(mouse.x - (GetScreenWidth() - (SCREEN_WIDTH * scale)) * 0.5f) / scale,
                     (mouse.y - (GetScreenHeight() - (SCREEN_HEIGHT * scale)) * 0.5f) / scale};
}

void LogMsg(const char *msg) { 
    // 1. Vẫn hiện popup 1 dòng ngoài màn hình
    strcpy(messageLog, msg); 
    
    // 2. Đẩy vào Lịch sử
    if (logCount < MAX_LOG_HISTORY) {
        strcpy(logHistory[logCount], msg);
        logCount++;
    } else {
        // Đầy bộ nhớ -> Đẩy mảng lên 1 nấc (Xóa dòng cũ nhất ở index 0)
        for (int i = 0; i < MAX_LOG_HISTORY - 1; i++) {
            strcpy(logHistory[i], logHistory[i + 1]);
        }
        strcpy(logHistory[MAX_LOG_HISTORY - 1], msg);
    }
}

float EaseOutBounce(float t)
{
    if (t < (1 / 2.75f))
        return (7.5625f * t * t);
    else if (t < (2 / 2.75f))
    {
        t -= (1.5f / 2.75f);
        return (7.5625f * t * t + 0.75f);
    }
    else if (t < (2.5 / 2.75f))
    {
        t -= (2.25f / 2.75f);
        return (7.5625f * t * t + 0.9375f);
    }
    else
    {
        t -= (2.625f / 2.75f);
        return (7.5625f * t * t + 0.984375f);
    }
}

void SetupPlayerSkills()
{
    for (int i = 0; i < 7; i++)
    {
        entPlayer.skills[i].scaleWithDef = false;
        entPlayer.skills[i].scaleWithMaxHp = false;
        entPlayer.skills[i].applyEffect = EFFECT_NONE;
        entPlayer.skills[i].effectDuration = 0;
        entPlayer.skills[i].effectValue = 0;
        entPlayer.skills[i].currentCooldown = 0;
    }

    strcpy(entPlayer.skills[0].name, u8"Đánh Thường");
    strcpy(entPlayer.skills[0].desc, u8"Tấn công cơ bản, không tiêu hao thể lực.");
    entPlayer.skills[0].damageBase = 50;
    entPlayer.skills[0].staminaCost = 0;
    entPlayer.skills[0].actionDelay = 1.0f;
    entPlayer.skills[0].isUltimate = false;
    strcpy(entPlayer.skills[1].name, u8"Phòng Thủ");
    strcpy(entPlayer.skills[1].desc, u8"Giảm sát thương nhận vào, hồi phục 40 Thể lực.");
    entPlayer.skills[1].damageBase = 0;
    entPlayer.skills[1].staminaCost = -40;//THỂ LỰC TIÊU HAO
    entPlayer.skills[1].actionDelay = 0.6f;
    entPlayer.skills[1].isUltimate = false;
    entPlayer.skills[1].applyEffect = EFFECT_DEF_UP; // Cấp hiệu ứng Tăng Giáp
    entPlayer.skills[1].effectDuration = 1;          // Duy trì trong 1 lượt của Boss
    entPlayer.skills[1].effectValue = 80;            // Cộng thẳng 50 Giáp (bạn có thể tự chỉnh)
    entPlayer.skills[1].effectChance = 100;          // 100% kích hoạt thành công
    strcpy(entPlayer.skills[2].name, u8"Tuyệt Kỹ");
    strcpy(entPlayer.skills[2].desc, u8"Đòn đánh chí mạng, cướp lượt nhưng tụt hậu cực sâu.");
    entPlayer.skills[2].damageBase = 450;
    entPlayer.skills[2].staminaCost = 0;
    entPlayer.skills[2].actionDelay = 4.0f;
    entPlayer.skills[2].isUltimate = true;

    switch (refPlayer->pClass)
    {
    case CLASS_DAU_GAU:
    case CLASS_STUDENT:
        strcpy(entPlayer.skills[3].name, u8"Giáp Gai");
        strcpy(entPlayer.skills[3].desc, u8"Nhận 70% Giáp.\n Phản đòn khi bị đánh.");
        entPlayer.skills[3].damageBase = 0;
        entPlayer.skills[3].staminaCost = 15;
        entPlayer.skills[3].actionDelay = 1.1f;
        entPlayer.skills[3].applyEffect = EFFECT_THORN_ARMOR;
        entPlayer.skills[3].effectDuration = 2;
        entPlayer.skills[3].effectValue = 70;
        entPlayer.skills[3].effectChance = 100;
        strcpy(entPlayer.skills[4].name, u8"Kẻ Chịu Đòn");
        strcpy(entPlayer.skills[4].desc, u8"Hồi máu bằng 350% Giáp.");
        entPlayer.skills[4].damageBase =0;
        entPlayer.skills[4].staminaCost = 25;
        entPlayer.skills[4].actionDelay = 0.9f;
        entPlayer.skills[4].healByDefPercent = 3.5f;
        entPlayer.skills[4].effectChance = 100;
        strcpy(entPlayer.skills[5].name, u8"Cú Đấm Sấm Sét");
        strcpy(entPlayer.skills[5].desc, u8"Gây 300 DMG. 30% tỷ lệ gây Choáng.");
        entPlayer.skills[5].damageBase = 350;
        entPlayer.skills[5].staminaCost = 10;
        entPlayer.skills[5].actionDelay = 1.0f;
        entPlayer.skills[5].applyEffect = EFFECT_STUN;
        entPlayer.skills[5].effectDuration = 2;
        entPlayer.skills[5].effectChance = 30;
        strcpy(entPlayer.skills[6].name, u8"Rung Chấn");
        strcpy(entPlayer.skills[6].desc, u8"Gây 300 DMG. Tăng 150 Giáp trong 3 lượt.");
        entPlayer.skills[6].damageBase = 300;
        entPlayer.skills[6].staminaCost = 35;
        entPlayer.skills[6].actionDelay = 0.8f;
        entPlayer.skills[6].applyEffect = EFFECT_DEF_UP;
        entPlayer.skills[6].effectDuration = 3;
        entPlayer.skills[6].effectValue = 150;
        entPlayer.skills[6].lifestealDmgPercent = 0.0f;
        entPlayer.skills[6].effectChance = 100;

        strcpy(entPlayer.skills[2].name, u8"Lấy Thủ Bù Công");
        strcpy(entPlayer.skills[2].desc, u8"Sát thương tức thì bạo lực bằng 600$ MAX DEF\nHồi chiêu 5 lượt.");
        entPlayer.skills[2].damageBase = 0;
        entPlayer.skills[2].staminaCost = 0;
        entPlayer.skills[2].actionDelay = 3.5f;
        entPlayer.skills[2].damageByDefPercent = 6.0f;
        entPlayer.skills[2].maxCooldown = 5;
        entPlayer.skills[2].currentCooldown = 0;
        break;
    case CLASS_HOC_BA:
    case CLASS_WARRIOR:
        // 1. Phản ứng hóa học (Gây Độc + Lùi lượt Boss)
        strcpy(entPlayer.skills[3].name, u8"Phản Ứng Hóa Học");
        strcpy(entPlayer.skills[3].desc, u8"Gây 150 DMG. Bỏ Độc (200 máu/lượt)\nvà đẩy lùi thanh hành động của Boss.");
        entPlayer.skills[3].damageBase = 150;
        entPlayer.skills[3].staminaCost = 20;
        entPlayer.skills[3].actionDelay = 0.9f;
        entPlayer.skills[3].applyEffect = EFFECT_POISON; // Hiệu ứng chính: Độc
        entPlayer.skills[3].effectDuration = 3;
        entPlayer.skills[3].effectValue = 200; // Trừ 120 máu
        entPlayer.skills[3].effectChance = 100;

        // 2. Tập trung cao độ (Buff ATK + DEF)
        strcpy(entPlayer.skills[4].name, u8"Tập Trung Cao Độ");
        strcpy(entPlayer.skills[4].desc, u8"Tăng 85% ATK và 160 Giáp trong 3 lượt.");
        entPlayer.skills[4].damageBase = 0;
        entPlayer.skills[4].staminaCost = 30;
        entPlayer.skills[4].actionDelay = 0.8f;
        entPlayer.skills[4].applyEffect = EFFECT_ATK_UP; // Hiệu ứng chính: ATK (DEF sẽ code cứng)
        entPlayer.skills[4].effectDuration = 3;
        entPlayer.skills[4].effectValue = 85; 
        entPlayer.skills[4].effectChance = 100;

        // 3. Cầu từ trường (Choáng + Giảm Giáp)
        strcpy(entPlayer.skills[5].name, u8"Cầu Từ Trường");
        strcpy(entPlayer.skills[5].desc, u8"Gây 400 DMG. -35% Giáp địch\n 50% gây Choáng.");
        entPlayer.skills[5].damageBase = 400;
        entPlayer.skills[5].staminaCost = 15;
        entPlayer.skills[5].actionDelay = 1.2f;
        entPlayer.skills[5].applyEffect = EFFECT_DEF_DOWN; // Hiệu ứng chính: Giảm giáp (Choáng code cứng)
        entPlayer.skills[5].effectDuration = 4;
        entPlayer.skills[5].effectValue = 35;
        entPlayer.skills[5].effectChance = 100;

        // 4. Chiến thuật tư duy (Hồi máu + Lập tức thêm lượt)
        strcpy(entPlayer.skills[6].name, u8"Chiến Thuật Tư Duy");
        strcpy(entPlayer.skills[6].desc, u8"Hồi 300 HP\n+250/buff hoạc debuff");
        entPlayer.skills[6].damageBase = 0;
        entPlayer.skills[6].staminaCost = 45;
        entPlayer.skills[6].actionDelay = 0.3f; // Sẽ ép actionValue = 0 ở ExecuteAction
        entPlayer.skills[6].applyEffect = EFFECT_HEAL; 
        entPlayer.skills[6].effectDuration = 1;
        entPlayer.skills[6].effectValue = 300;
        entPlayer.skills[6].effectChance = 100;

        // TUYỆT KỸ: Định Lý Cuối Cùng
        strcpy(entPlayer.skills[2].name, u8"Định Lý Cuối Cùng");
        strcpy(entPlayer.skills[2].desc, u8"Xuyên 100% Giáp\n+80%/bbuff toàn sân");
        entPlayer.skills[2].damageBase = 300; // DMG gốc (sẽ nhân lên theo Debuff)
        entPlayer.skills[2].staminaCost = 0;
        entPlayer.skills[2].actionDelay = 4.0f; // Bị tụt lượt nặng sau khi dùng
        entPlayer.skills[2].isUltimate = true;
        entPlayer.skills[2].maxCooldown = 4;

        entPlayer.skills[2].applyEffect = EFFECT_ATK_DOWN; // Hiệu ứng: Giảm ATK
        entPlayer.skills[2].effectDuration = 2;              // Tồn tại: 2 lượt
        entPlayer.skills[2].effectValue = 65;               // Giá trị: Giảm 20% (Bạn có thể tăng/giảm số này)
        entPlayer.skills[2].effectChance = 100;
        break;
    case CLASS_SOAI_CA:
    case CLASS_MAGE:
        // 1. Hào Quang Chân Huyết (W1)
        strcpy(entPlayer.skills[3].name, u8"Hào Quang Huyết Sắc");
        strcpy(entPlayer.skills[3].desc, u8"Hồi 10% Máu Tối Đa mỗi lượt\ntrong 3 lượt.");
        entPlayer.skills[3].damageBase = 0;
        entPlayer.skills[3].staminaCost = 25;
        entPlayer.skills[3].actionDelay = 1.0f;
        entPlayer.skills[3].applyEffect = EFFECT_HEAL; 
        entPlayer.skills[3].effectDuration = 3;
        entPlayer.skills[3].effectValue = (int)(entPlayer.maxHp * 0.1f); 
        entPlayer.skills[3].effectChance = 100;

        // 2. Huyết Tiễn (W2)
        strcpy(entPlayer.skills[4].name, u8"Huyết Tiễn");
        strcpy(entPlayer.skills[4].desc, u8"Bắn mũi tên máu (ST = DMG + 10% Max HP).\n Hồi ngay 200 HP.");
        entPlayer.skills[4].damageBase = 350; 
        entPlayer.skills[4].staminaCost = 25;
        entPlayer.skills[4].actionDelay = 1.0f;
        entPlayer.skills[4].scaleWithMaxHp = true; // Sát thương theo máu
        entPlayer.skills[4].applyEffect = EFFECT_NONE;

        // 3. Dấu Ấn Ký Sinh (W3)
        strcpy(entPlayer.skills[5].name, u8"Dấu Ấn Ký Sinh");
        strcpy(entPlayer.skills[5].desc, u8"Hút Máu 150%.\nGây hiệu ứng ĐỘC 3 lượt.");
        entPlayer.skills[5].damageBase = 300;
        entPlayer.skills[5].staminaCost = 30;
        entPlayer.skills[5].actionDelay = 1.0f;
        entPlayer.skills[5].applyEffect = EFFECT_POISON;
        entPlayer.skills[5].effectDuration = 3;
        entPlayer.skills[5].effectValue = 100; // Độc trừ 40 máu/turn
        entPlayer.skills[5].effectChance = 100;
        entPlayer.skills[5].lifestealDmgPercent = 1.5f; // Hút 100% sát thương

        // 4. Giao Kèo Ác Quỷ (W4)
        strcpy(entPlayer.skills[6].name, u8"Giao Kèo Ác Quỷ");
        strcpy(entPlayer.skills[6].desc, u8"-20% HP +100SPD.\n+5% HP trong 2 lượt.");
        entPlayer.skills[6].damageBase = 0;
        entPlayer.skills[6].staminaCost = 0; // Không tốn thể lực
        entPlayer.skills[6].actionDelay = 0.2f; // Tốn rất ít turn, gần như cướp lượt
        entPlayer.skills[6].applyEffect = EFFECT_SPD_UP; 
        entPlayer.skills[6].effectDuration = 2;
        entPlayer.skills[6].effectValue = 100; // Tăng SPD
        entPlayer.skills[6].effectChance = 100;
        entPlayer.skills[6].maxCooldown = 6;
        // Tuyệt Kỹ: Vạn Tiễn Xuyên Tâm (R)
        strcpy(entPlayer.skills[2].name, u8"Vạn Tiễn Xuyên Tâm");
        strcpy(entPlayer.skills[2].desc, u8"Xuyên 100% Giáp. ST = 50% Máu Tối Đa. Hút Máu 30%.");
        entPlayer.skills[2].damageBase = 0; 
        entPlayer.skills[2].staminaCost = 0;
        entPlayer.skills[2].actionDelay = 4.0f; 
        entPlayer.skills[2].isUltimate = true;
        entPlayer.skills[2].maxCooldown = 4;
        entPlayer.skills[2].lifestealDmgPercent = 0.3f; // Hút 100%
        break;
    case CLASS_PHU_NHI_DAI:
    case CLASS_ARCHER:
        // Đánh thường (Ghi đè skill 0)
        strcpy(entPlayer.skills[0].name, u8"Cổ Tức Đều Đặn");
        strcpy(entPlayer.skills[0].desc, u8"ST = DMG + 30% ATK.\nĐánh trúng +10 ATK vĩnh viễn.");
        entPlayer.skills[0].damageBase = 15;
        entPlayer.skills[0].staminaCost = 0;
        entPlayer.skills[0].actionDelay = 1.0f;
        
        // W1: Thu Hồi Vốn
        strcpy(entPlayer.skills[3].name, u8"Thu Hồi Vốn");
        strcpy(entPlayer.skills[3].desc, u8"Gây 250 DMG.\nHút máu 75% sát thương.");
        entPlayer.skills[3].damageBase = 250;
        entPlayer.skills[3].staminaCost = 20;
        entPlayer.skills[3].actionDelay = 0.9f;
        entPlayer.skills[3].lifestealDmgPercent = 0.75f;
        
        // W2: Phá Giá Thị Trường
        strcpy(entPlayer.skills[4].name, u8"Phá Giá Thị Trường");
        strcpy(entPlayer.skills[4].desc, u8"Gây 200 DMG.\nGiảm 55% Giáp địch trong 3 lượt.");
        entPlayer.skills[4].damageBase = 200;
        entPlayer.skills[4].staminaCost =30;
        entPlayer.skills[4].actionDelay = 1.0f;
        entPlayer.skills[4].applyEffect = EFFECT_DEF_DOWN;
        entPlayer.skills[4].effectDuration = 3;
        entPlayer.skills[4].effectValue = 55;
        
        // W3: Giao Dịch Nội Gián
        strcpy(entPlayer.skills[5].name, u8"Dò Tìm Con Mồi");
        strcpy(entPlayer.skills[5].desc, u8"Tăng 40% Chí mạng và\n50 Tốc độ (SPD) trong 3 lượt.");
        entPlayer.skills[5].damageBase = 0;
        entPlayer.skills[5].staminaCost = 25;
        entPlayer.skills[5].actionDelay = 0.5f; 
        entPlayer.skills[5].applyEffect = EFFECT_CRIT_UP; // Ta sẽ xử lý ghép buff SPD ở ExecuteAction
        entPlayer.skills[5].effectDuration = 3;
        entPlayer.skills[5].effectValue = 40;
        
        // W4: Lũng Đoạn Cung Cầu
        strcpy(entPlayer.skills[6].name, u8"Triệt Hạ Con Mồi");
        strcpy(entPlayer.skills[6].desc, u8"Gây 350 DMG.\nTăng 60% ATK trong 3 lượt.");
        entPlayer.skills[6].damageBase = 350;
        entPlayer.skills[6].staminaCost = 35; // Hơi tốn thể lực
        entPlayer.skills[6].actionDelay = 1.1f; // Trễ xíu
        entPlayer.skills[6].applyEffect = EFFECT_ATK_UP;
        entPlayer.skills[6].effectDuration = 3;
        entPlayer.skills[6].effectValue = 60;
        
        // R: Canh Bạc Tất Tay
        strcpy(entPlayer.skills[2].name, u8"Canh Bạc Tất Tay");
        strcpy(entPlayer.skills[2].desc, u8"-99% HP & Giáp.\nST = 200% ATK + Xuyên 100% Giáp.");
        entPlayer.skills[2].damageBase = 0; 
        entPlayer.skills[2].staminaCost = 0;
        entPlayer.skills[2].actionDelay = 4.0f;
        entPlayer.skills[2].isUltimate = true;
        entPlayer.skills[2].maxCooldown = 5;
        break;
    }
}

void SetupBossSkills()
{
    for (int i = 0; i < 7; i++)
    {
        entBoss.skills[i].staminaCost = 0;
        entBoss.skills[i].actionDelay = 1.0f;
        entBoss.skills[i].currentCooldown = 0;
    }

   if (currentPhase == 1)
    {
        // 1. Lazer Hủy Diệt: Giảm 20% Giáp trong 2 lượt (Tận dụng logic chia 100 có sẵn trong GetTotalDef)
        strcpy(entBoss.skills[0].name, u8"Lazer Hủy Diệt");
        entBoss.skills[0].damageBase = 45;
        entBoss.skills[0].actionDelay = 1.5f;
        entBoss.skills[0].applyEffect = EFFECT_DEF_DOWN; 
        entBoss.skills[0].effectDuration = 2;
        entBoss.skills[0].effectValue = 20;  // 20 tương đương 20%
        entBoss.skills[0].effectChance = 100;

        // 2. Dịch Chuyển Thời Không: Thuần sát thương, ta sẽ xử lý đẩy lùi lượt ở hàm ExecuteAction
        strcpy(entBoss.skills[1].name, u8"Dịch Chuyển Thời Không");
        entBoss.skills[1].damageBase = 60;
        entBoss.skills[1].actionDelay = 2.0f;

        // 3. Pháo Hố Đen (Chiêu Cuối): Câm Lặng (Khóa sách kỹ năng) trong 2 lượt
        strcpy(entBoss.skills[2].name, u8"Pháo Hố Đen");
        entBoss.skills[2].damageBase = 250;
        entBoss.skills[2].actionDelay = 3.0f;
        entBoss.skills[2].applyEffect = EFFECT_SILENCE;
        entBoss.skills[2].effectDuration = 2;
        entBoss.skills[2].effectChance = 100;
    }
    // Thêm khối này vào trong hàm SetupBossSkills()
    else if (currentPhase == 2)
    {
        // 1. Đột Kích Phân Rã (Thay cho Lazer) - Gắn hiệu ứng ĐỘC (Mất máu)
        strcpy(entBoss.skills[0].name, u8"Đột Kích Phân Rã");
        entBoss.skills[0].damageBase = 100;
        entBoss.skills[0].actionDelay = 1.2f;
        entBoss.skills[0].applyEffect = EFFECT_POISON;
        entBoss.skills[0].effectDuration = 3;
        entBoss.skills[0].effectValue = 50; // Trừ 20 máu mỗi turn
        entBoss.skills[0].effectChance = 60; // 70% trúng độc

        // 2. Quá Tải Huyết Thanh - Buff ATK
        strcpy(entBoss.skills[1].name, u8"Quá Tải Huyết Thanh");
        entBoss.skills[1].damageBase = 0;
        entBoss.skills[1].actionDelay = 1.1f;
        entBoss.skills[1].applyEffect = EFFECT_ATK_UP;
        entBoss.skills[1].effectDuration = 2;
        entBoss.skills[1].effectValue = 50; // +50% ATK
        entBoss.skills[1].effectChance = 100;

        // 3. Nghịch Lý Hủy Diệt (Chiêu cuối) - Giảm Tốc + Hút máu
        strcpy(entBoss.skills[2].name, u8"Nghịch Lý Hủy Diệt");
        entBoss.skills[2].damageBase = 250;
        entBoss.skills[2].actionDelay = 3.0f;
        entBoss.skills[2].applyEffect = EFFECT_SPD_DOWN;
        entBoss.skills[2].effectDuration = 2;
        entBoss.skills[2].effectValue = 30; // Giảm 30 Speed
        entBoss.skills[2].effectChance = 100;
        entBoss.skills[2].lifestealDmgPercent = 0.2f; // Hút 20% sát thương thành máu

        // 4. Lazer Hủy Diệt (Skill 3 - Kế thừa Phase 1 nhưng đấm đau hơn xíu)
        strcpy(entBoss.skills[3].name, u8"Lazer Hủy Diệt");
        entBoss.skills[3].damageBase = 60; // Gốc Phase 1 là 45
        entBoss.skills[3].actionDelay = 1.5f;
        entBoss.skills[3].applyEffect = EFFECT_DEF_DOWN; 
        entBoss.skills[3].effectDuration = 2;
        entBoss.skills[3].effectValue = 20;  
        entBoss.skills[3].effectChance = 100;

        // 5. Dịch Chuyển Thời Không (Skill 4 - Kế thừa Phase 1)
        strcpy(entBoss.skills[4].name, u8"Dịch Chuyển Thời Không");
        entBoss.skills[4].damageBase = 70; // Gốc Phase 1 là 60
        entBoss.skills[4].actionDelay = 2.0f;
    }
    else
    {
        strcpy(entBoss.skills[0].name, u8"Đòn Căn Bản");
        entBoss.skills[0].damageBase = 30;
    }
}

void Combat_Init()
{
    if (!resourcesLoaded)
    {
        if (FileExists("resources/vfx/triet_ha_doi_thu.png")) { // Ảnh đạn
            texTrietHa = LoadTexture("resources/vfx/triet_ha_doi_thu.png");
            SetTextureFilter(texTrietHa, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/do_tim_con_moi.png")) {
            texDoTimConMoi = LoadTexture("resources/vfx/do_tim_con_moi.png");
            SetTextureFilter(texDoTimConMoi, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/pha_gia_thi_truong.png")) {
            texPhaGia = LoadTexture("resources/vfx/pha_gia_thi_truong.png");
            SetTextureFilter(texPhaGia, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/thu_hoi_von.png")) {
            texThuHoiVonCoin = LoadTexture("resources/vfx/thu_hoi_von.png");
            SetTextureFilter(texThuHoiVonCoin, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/dau_an_ki_sinh.png")) {
            texDauAnKiSinh = LoadTexture("resources/vfx/dau_an_ki_sinh.png");
            SetTextureFilter(texDauAnKiSinh, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/giao_keo_ac_quy.png")) {
            texGiaoKeoAcQuy = LoadTexture("resources/vfx/giao_keo_ac_quy.png");
            SetTextureFilter(texGiaoKeoAcQuy, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/haoquanhuyetsac.png")) {
            texHaoQuangHuyetSac = LoadTexture("resources/vfx/haoquanhuyetsac.png");
            SetTextureFilter(texHaoQuangHuyetSac, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/huyet_tien.png")) {
            texHuyetTien = LoadTexture("resources/vfx/huyet_tien.png");
            SetTextureFilter(texHuyetTien, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/dinh_li_cuoi_cung.png")) {
            texDinhLiCuoiCung = LoadTexture("resources/vfx/dinh_li_cuoi_cung.png");
            SetTextureFilter(texDinhLiCuoiCung, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/tu_duy_chien_thuat.png")) {
            texTuDuyChienThuat = LoadTexture("resources/vfx/tu_duy_chien_thuat.png");
            SetTextureFilter(texTuDuyChienThuat, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/cau_tu_truong.png")) {
            texCauTuTruong = LoadTexture("resources/vfx/cau_tu_truong.png");
            SetTextureFilter(texCauTuTruong, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/khien_animation.png")) {
            texKhienAnimation = LoadTexture("resources/vfx/khien_animation.png");
            SetTextureFilter(texKhienAnimation, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/tang_cong.png")) {
            texTangCong = LoadTexture("resources/vfx/tang_cong.png");
            SetTextureFilter(texTangCong, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/icon_taptrungcaodo.png")) {
            texIconTapTrung = LoadTexture("resources/vfx/icon_taptrungcaodo.png");
            SetTextureFilter(texIconTapTrung, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/phan_ung_hoa_hoc.png")) {
            texPhanUngHoaHoc = LoadTexture("resources/vfx/phan_ung_hoa_hoc.png");
            SetTextureFilter(texPhanUngHoaHoc, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/voidpunch.png")) {
            texVoidPunch = LoadTexture("resources/vfx/voidpunch.png");
            SetTextureFilter(texVoidPunch, TEXTURE_FILTER_BILINEAR);
        }

        if (FileExists("resources/vfx/telegate.png")) {
            texTelegateVFX = LoadTexture("resources/vfx/telegate.png");
            SetTextureFilter(texTelegateVFX, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/phase2_aura.png")) {
            texPhase2Aura = LoadTexture("resources/vfx/phase2_aura.png");
            SetTextureFilter(texPhase2Aura, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/echo_lazer.png")) {
            texLaserVFX = LoadTexture("resources/vfx/echo_lazer.png");
            SetTextureFilter(texLaserVFX, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/blackhole.png")) {
            texBlackHoleVFX = LoadTexture("resources/vfx/blackhole.png");
            SetTextureFilter(texBlackHoleVFX, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/game_map/map6/phonglab.png"))
            texBackground = LoadTexture("resources/game_map/map6/phonglab.png");
        if (FileExists("resources/menu/ui.png"))
        {
            texCombatUI = LoadTexture("resources/menu/ui.png");
            SetTextureFilter(texCombatUI, TEXTURE_FILTER_POINT);
        }
        if (FileExists("resources/vfx/book.png"))
            texBookVFX = LoadTexture("resources/vfx/book.png");
        if (FileExists("resources/vfx/explosion.png"))
            texExplosionVFX = LoadTexture("resources/vfx/explosion.png");
        if (FileExists("resources/vfx/punch.png"))
            texPunchVFX = LoadTexture("resources/vfx/punch.png");
        if (FileExists("resources/vfx/bow.png"))
            texBowVFX = LoadTexture("resources/vfx/bow.png");
        if (FileExists("resources/vfx/arrow.png"))
            texArrowVFX = LoadTexture("resources/vfx/arrow.png");
        if (FileExists("resources/vfx/coin.png"))
            texCoinVFX = LoadTexture("resources/vfx/coin.png");
        if (FileExists("resources/vfx/giap_gai_animation.png")) {
            texThornVFX = LoadTexture("resources/vfx/giap_gai_animation.png");
            SetTextureFilter(texThornVFX, TEXTURE_FILTER_BILINEAR);
        }

        if (FileExists("resources/vfx/icon_kechiudon.png")) {
            texKeChiuDonIcon = LoadTexture("resources/vfx/icon_kechiudon.png");
        }

        if (FileExists("resources/vfx/kechiudon_animation.png")) {
            texKeChiuDonVFX = LoadTexture("resources/vfx/kechiudon_animation.png");
            SetTextureFilter(texKeChiuDonVFX, TEXTURE_FILTER_BILINEAR); 
        }

        // ĐƯA SẤM SÉT VỀ ĐÚNG NƠI QUY ĐỊNH (LOAD 1 LẦN TỪ ĐẦU GAME)
        if (FileExists("resources/vfx/icon_cudamsamset.png")) {
            texLightningIcon = LoadTexture("resources/vfx/icon_cudamsamset.png");
        }

        if (FileExists("resources/vfx/lightning.png")) {
            texLightningVFX = LoadTexture("resources/vfx/lightning.png");
            SetTextureFilter(texLightningVFX, TEXTURE_FILTER_BILINEAR); 
        }
        if (FileExists("resources/vfx/rung_chan.png")) {
            texRungChanIcon = LoadTexture("resources/vfx/rung_chan.png");
        }

        if (FileExists("resources/vfx/untimate_daugau.png")) {
            texUltiDauGau = LoadTexture("resources/vfx/untimate_daugau.png");
            SetTextureFilter(texUltiDauGau, TEXTURE_FILTER_BILINEAR);
        }

        if (FileExists("resources/vfx/poision.png"))
            texIconPoison = LoadTexture("resources/vfx/poision.png");
        if (FileExists("resources/vfx/cam_lang.png"))
            texIconSilence = LoadTexture("resources/vfx/cam_lang.png");
        if (FileExists("resources/vfx/up_hp.png"))
            texIconUpHp = LoadTexture("resources/vfx/up_hp.png");
        if (FileExists("resources/vfx/up_def.png"))
            texIconUpDef = LoadTexture("resources/vfx/up_def.png");
        if (FileExists("resources/vfx/up_attack.png"))
            texIconUpAtk = LoadTexture("resources/vfx/up_attack.png");

            // --- LOAD ICON CHÍ MẠNG PHÚ NHỊ ĐẠI ---
        if (FileExists("resources/vfx/crit_up.png")) {
            texIconUpCrit = LoadTexture("resources/vfx/crit_up.png");
            // Áp filter Bilinear để khi nó scale nhỏ lại thì ảnh vẫn nét, không bị răng cưa
            SetTextureFilter(texIconUpCrit, TEXTURE_FILTER_BILINEAR);
        }
        if (FileExists("resources/vfx/stun.png"))
            texIconStun = LoadTexture("resources/vfx/stun.png");
        else
        {
            Image img = GenImageColor(512, 512, MAGENTA);
            texIconStun = LoadTextureFromImage(img);
            UnloadImage(img);
        }

        if (FileExists("resources/menu/title_frames.png"))
        {
            texTitleFrame = LoadTexture("resources/menu/title_frames.png");
            SetTextureFilter(texTitleFrame, TEXTURE_FILTER_POINT);
        }
        if (FileExists("resources/menu/aura.png"))
        {
            texAura = LoadTexture("resources/menu/aura.png");
            SetTextureFilter(texAura, TEXTURE_FILTER_BILINEAR);
        }

        resourcesLoaded = true;
    }
}

void Combat_Shutdown()
{
    if (resourcesLoaded)
    {
        UnloadTexture(texExplosionVFX);
        UnloadTexture(texBookVFX);
        UnloadTexture(texBackground);
        UnloadTexture(texCombatUI);
        UnloadTexture(texTitleFrame);
        UnloadTexture(texAura);
        UnloadTexture(texPunchVFX);
        UnloadTexture(texBowVFX);
        UnloadTexture(texArrowVFX);
        UnloadTexture(texCoinVFX);
        UnloadTexture(texIconPoison);
        UnloadTexture(texIconUpCrit);
        UnloadTexture(texIconSilence);
        UnloadTexture(texPhase2Aura);
        UnloadTexture(texIconUpHp);
        UnloadTexture(texIconUpDef);
        UnloadTexture(texIconUpAtk);
        UnloadTexture(texIconStun);
        UnloadTexture(texThornVFX);
        UnloadTexture(texKeChiuDonIcon);
        UnloadTexture(texKeChiuDonVFX);
        UnloadTexture(texLightningIcon);
        UnloadTexture(texLightningVFX);
          UnloadTexture(texRungChanIcon);
          UnloadTexture(texUltiDauGau);
          UnloadTexture(texTelegateVFX);
          UnloadTexture(texLaserVFX);
          UnloadTexture(texBlackHoleVFX);
          UnloadTexture(texVoidPunch);
          UnloadTexture(texPhanUngHoaHoc);
          UnloadTexture(texKhienAnimation);
        UnloadTexture(texTangCong);
        UnloadTexture(texIconTapTrung);
        UnloadTexture(texCauTuTruong);
        UnloadTexture(texTuDuyChienThuat);
        UnloadTexture(texDinhLiCuoiCung);
        UnloadTexture(texHaoQuangHuyetSac);
        UnloadTexture(texHuyetTien);
        UnloadTexture(texDauAnKiSinh);
        UnloadTexture(texGiaoKeoAcQuy);
        UnloadTexture(texThuHoiVonCoin);
        UnloadTexture(texPhaGia);
        UnloadTexture(texDoTimConMoi);
        UnloadTexture(texTrietHa);

        resourcesLoaded = false;
    }
    if (phase1TexLoaded)
    {
        UnloadTexture(texPhase1Trung);
        UnloadTexture(texPhase1BV1);
        UnloadTexture(texPhase1BV2);
        phase1TexLoaded = false;
    }
}

int Combat_GetResult() { return combatResult; }
void Combat_ResetResult() { combatResult = 0; }
bool Combat_IsActive() { return combatActive; }

void AddActionValue(CombatEntity *ent, float delayMultiplier) { ent->actionValue += (10000.0f / ent->speed) * delayMultiplier; }

void Combat_Start(Player *player, Npc *enemy, int phase)
{
    refPlayer = player;
    refBoss = enemy;
    currentPhase = phase;
    combatResult = 0;
    combatActive = true;
    combatState = COMBAT_STATE_INTRO;
    stateTimer = 0.0f;
    fadeAlpha = 0.0f;
    showTurnAnim = false;
    cb_bookState = CB_BOOK_IDLE;
    cb_bookProgress = 0.0f;
    selectedBookSkill = -1;
    cb_titleState = 0;
    cb_titleYOffset = -600.0f;
    cb_titleVelocity = 0.0f;
    cb_waitingToCloseBook = false;
    hasUsedRevive = false; // Reset lại Đặc Quyền Sửa Sai
    strcpy(messageLog, "");
    logCount = 0;             // [NÂNG CẤP LOG]
    showLogWindow = false;    // [NÂNG CẤP LOG]

    for (int i = 0; i < 10; i++)
        iconAnims[i].active = false; // Xóa sạch hoạt ảnh cũ

    entPlayer.isPlayer = true;
    strcpy(entPlayer.name, u8"Bạn");
    entPlayer.pClass = player->pClass;
    entPlayer.texture = player->currentTexture;
    entPlayer.frameRec = player->frameRec;
    entPlayer.maxHp = player->stats.maxHp;
    entPlayer.hp = player->stats.currentHp;
    if (entPlayer.hp > entPlayer.maxHp) {//chặn chàn máu    
        entPlayer.hp = entPlayer.maxHp;
    }
    entPlayer.maxStamina = 100;
    entPlayer.stamina = player->stats.stamina;
    entPlayer.atk = player->stats.damage;
    entPlayer.permanentDefBonus = 0;      // Reset giáp tích lũy
    entPlayer.permanentAtkBonus = 0;
    entPlayer.permanentCritBonus = 0;
    entPlayer.staminaSpent = 0;
    entPlayer.dmgTakenLastTurn = 0;       // Reset sát thương nhận vào trận trước
    entPlayer.isResilienceActive = false;
    entPlayer.def = player->stats.defense;
    entPlayer.critRate = 20;     // <--- THÊM DÒNG NÀY (Ví dụ: 10%)
    entPlayer.critDamage = 2.0f; // <--- THÊM DÒNG NÀY (Sát thương x1.5)

    if (player->pClass == CLASS_HOC_BA || player->pClass == CLASS_WARRIOR)
        entPlayer.speed = 100;
    else if (player->pClass == CLASS_SOAI_CA || player->pClass == CLASS_MAGE)
        entPlayer.speed = 95;
    else if (player->pClass == CLASS_PHU_NHI_DAI || player->pClass == CLASS_ARCHER)
        entPlayer.speed = 130;
    else
        entPlayer.speed = 80;

    SetupPlayerSkills();

    entBoss.isPlayer = false;
    entBoss.texture = &enemy->texture;
    float enemyW = (float)enemy->texture.width / (enemy->frameCount > 0 ? enemy->frameCount : 1);
    entBoss.frameRec = (Rectangle){0, 0, enemyW, (float)enemy->texture.height};
    entBoss.stamina = 100;

    if (phase == 1)
    {
        strcpy(entBoss.name, u8"Đội Thanh Trừng ECHO");
        entBoss.maxHp = 3000;
        entBoss.hp = 3000;
        entBoss.atk = 250;
        entBoss.def = 40;
        entBoss.speed = 95;
        if (!phase1TexLoaded)
        {
            texPhase1Trung = LoadTexture("resources/npc/map_easter_egg/ba_gia.png");
            texPhase1BV1 = LoadTexture("resources/npc/map_alpha/baove1.png");
            texPhase1BV2 = LoadTexture("resources/npc/map_alpha/baove2.png");
            phase1TexLoaded = true;
        }
    }
    else if (phase == 2)
    {
        strcpy(entBoss.name, u8"Quốc Trung [ATX-10]");
        entBoss.maxHp = 5000;
        entBoss.hp = 5000;
        entBoss.atk = 320;
        entBoss.def = 65;
        entBoss.speed = 115;
    }
    else
    {
        strcpy(entBoss.name, enemy->name);
        entBoss.maxHp = enemy->stats.maxHp;
        entBoss.hp = enemy->stats.maxHp;
        entBoss.atk = enemy->stats.damage;
        entBoss.def = enemy->stats.defense;
        entBoss.speed = 100;
    }
    SetupBossSkills();

    entPlayer.actionValue = 0.0f; // Ép người chơi luôn được đi đầu tiên ở Turn 1
    entBoss.actionValue = 10000.0f / entBoss.speed;
    // [SỬA ĐOẠN NÀY] Kiểm tra Phase để phát nhạc tương ứng
    if (phase == 2) {
        Audio_PlayMusic(MUSIC_BATTLE_PHASE2);
    } else {
        Audio_PlayMusic(MUSIC_BATTLE); // Dùng cho Phase 1 hoặc quái thường
    }
    LogMsg(TextFormat(u8"VÀO TRẬN: %s!", entBoss.name));
}

int GetTotalDef(CombatEntity *ent)
{
    int baseDef = ent->def + ent->permanentDefBonus;
    float multiplier = 1.0f;

    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (ent->effects[i].duration > 0)
        {
            if (ent->effects[i].type == EFFECT_THORN_ARMOR)
            {
                multiplier += (float)ent->effects[i].value / 100.0f;
            }
            else if (ent->effects[i].type == EFFECT_DEF_UP)
            {
                baseDef += ent->effects[i].value;
            }
            else if (ent->effects[i].type == EFFECT_DEF_DOWN)
            {
                multiplier -= (float)ent->effects[i].value / 100.0f;
            }
        }
    }
    if (multiplier < 0.0f)
        multiplier = 0.0f;
    return (int)(baseDef * multiplier);
}
int GetTotalAtk(CombatEntity *ent)
{
    int baseAtk = ent->atk + ent->permanentAtkBonus; // Đã cộng ATK tích lũy
    float multiplier = 1.0f;

    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (ent->effects[i].duration > 0)
        {
            if (ent->effects[i].type == EFFECT_ATK_UP)
            {
                multiplier += (float)ent->effects[i].value / 100.0f; // Cộng thêm %
            }
            else if (ent->effects[i].type == EFFECT_ATK_DOWN)
            {
                multiplier -= (float)ent->effects[i].value / 100.0f; // Trừ đi %
            }
        }
    }
    
    if (multiplier < 0.0f) multiplier = 0.0f; // Không để sát thương bị âm
    return (int)(baseAtk * multiplier);
}

void ExecuteAction(CombatEntity *attacker, CombatEntity *defender, CombatSkill *skill)
{
    attacker->stamina -= skill->staminaCost;
    if (attacker->stamina > attacker->maxStamina)
        attacker->stamina = attacker->maxStamina;
    if (skill->maxCooldown > 0)
        skill->currentCooldown = skill->maxCooldown;

   int rawDmg = GetTotalAtk(attacker) + skill->damageBase;

    if (skill->damageByDefPercent > 0.0f)
        rawDmg = GetTotalAtk(attacker) + (int)(GetTotalDef(attacker) * skill->damageByDefPercent);
    if (skill->scaleWithMaxHp)
        rawDmg += (int)(attacker->maxHp * 0.1f);

    int finalDmg = rawDmg - GetTotalDef(defender);
    // =========================================================
    // [PHÚ NHỊ ĐẠI] XỬ LÝ SÁT THƯƠNG ĐẶC BIỆT
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_PHU_NHI_DAI || attacker->pClass == CLASS_ARCHER)) {
        // Đánh thường (Q): Cộng thêm 30% Tổng ATK
        if (strcmp(skill->name, u8"Cổ Tức Đều Đặn") == 0) {
            finalDmg += (int)(GetTotalAtk(attacker) * 0.3f); 
        }
        
        // Tuyệt Kỹ (R): Canh Bạc Tất Tay
        if (strcmp(skill->name, u8"Canh Bạc Tất Tay") == 0) {
            int lostHp = (int)(attacker->hp * 0.99f);
            int lostDef = (int)(GetTotalDef(attacker) * 0.99f);
            
            attacker->hp -= lostHp;
            attacker->permanentDefBonus -= lostDef; 
            
            // Xuyên 100% Giáp (Gán đè finalDmg)
            finalDmg = (GetTotalAtk(attacker) * 2) + (lostHp * 2) + (lostDef * 2); 
            SpawnFloatingText("-99% HP!", (Vector2){180, SCREEN_HEIGHT - 350}, PURPLE, true);
        }
    }
   // =========================================================
    //(HBA) [HỌC BÁ] XỬ LÝ SÁT THƯƠNG TUYỆT KỸ: ĐỊNH LÝ CUỐI CÙNG
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_HOC_BA || attacker->pClass == CLASS_WARRIOR) && 
        strcmp(skill->name, u8"Định Lý Cuối Cùng") == 0) {
        
        // 1. Đếm số lượng Debuff trên người Boss
        int debuffCount = 0;
        for (int k = 0; k < MAX_EFFECTS; k++) {
            if (defender->effects[k].duration > 0 && 
               (defender->effects[k].type == EFFECT_POISON || defender->effects[k].type == EFFECT_STUN || 
                defender->effects[k].type == EFFECT_SPD_DOWN || defender->effects[k].type == EFFECT_DEF_DOWN || 
                defender->effects[k].type == EFFECT_SILENCE)) {
                debuffCount++;
            }
        }

        // 2. Đếm số lượng Buff trên người Học Bá
        int buffCount = 0;
        for (int k = 0; k < MAX_EFFECTS; k++) {
            if (attacker->effects[k].duration > 0 && 
               (attacker->effects[k].type == EFFECT_HEAL || attacker->effects[k].type == EFFECT_ATK_UP || 
                attacker->effects[k].type == EFFECT_DEF_UP || attacker->effects[k].type == EFFECT_THORN_ARMOR || 
                attacker->effects[k].type == EFFECT_SPD_UP)) {
                buffCount++;
            }
        }

        // 3. Tính lại sát thương: Bỏ qua 100% giáp và nhân với tổng số liệu (Buff + Debuff)
        int totalData = debuffCount + buffCount;
        finalDmg = rawDmg; // Bỏ qua trừ Giáp
        float multiplier = 1.0f + (0.8f * totalData); // Mỗi data tăng 50% DMG
        finalDmg = (int)(finalDmg * multiplier);
        
        LogMsg(TextFormat(u8"Định Lý: Quét %d Buff & %d Debuff! DMG x%.1f", buffCount, debuffCount, multiplier));
    }
    // =========================================================

    // =========================================================
    // [SOÁI CA] XỬ LÝ SÁT THƯƠNG TUYỆT KỸ: VẠN TIỄN XUYÊN TÂM
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_SOAI_CA || attacker->pClass == CLASS_MAGE) && 
        strcmp(skill->name, u8"Vạn Tiễn Xuyên Tâm") == 0) {
        
        finalDmg = (int)(attacker->maxHp * 0.5f); // 50% Máu tối đa
        // Code không trừ đi GetTotalDef(defender) => Mặc định Xuyên 100% Giáp
        
        LogMsg(TextFormat(u8"Vạn Tiễn Xuyên Tâm: Gây %d Sát thương chuẩn!", finalDmg));
    }

    // Thêm điều kiện !skill->isUltimate để không xóa sát thương của Chiêu cuối
    if (skill->damageBase == 0 && skill->damageByDefPercent == 0.0f && !skill->isUltimate)
        finalDmg = 0;
    else if (finalDmg <= 0)
        finalDmg = 1;

   bool isCrit = false;
    int totalCritRate = attacker->critRate + attacker->permanentCritBonus; // Cộng nội tại
    
    // Cộng thêm từ Buff Giao Dịch Nội Gián
    for (int k = 0; k < MAX_EFFECTS; k++) {
        if (attacker->effects[k].duration > 0 && attacker->effects[k].type == EFFECT_CRIT_UP) {
            totalCritRate += attacker->effects[k].value;
        }
    }
    // Chiêu cuối 100% Chí mạng
    if (strcmp(skill->name, u8"Canh Bạc Tất Tay") == 0) {
        totalCritRate = 1000; 
    }

    // Roll tỉ lệ chí mạng
    if (finalDmg > 0 && GetRandomValue(1, 100) <= totalCritRate)
    {
        isCrit = true;
        finalDmg = (int)(finalDmg * attacker->critDamage);
    }

    // Căn tọa độ text ngay giữa ngực nhân vật
    Vector2 textPos = defender->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 300} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 300};

    if (finalDmg > 0)
    {
        defender->hp -= finalDmg;
        if (defender->hp < 0)
            defender->hp = 0;

        defender->dmgTakenLastTurn += finalDmg;

        // [CẬP NHẬT MÀU] Đánh thường nổ Trắng, Crit nổ Đỏ
        SpawnFloatingText(TextFormat("-%d", finalDmg), textPos, isCrit ? RED : YELLOW, isCrit);

        bool hasThorn = false;
        for (int i = 0; i < MAX_EFFECTS; i++)
        {
            if (defender->effects[i].duration > 0 && defender->effects[i].type == EFFECT_THORN_ARMOR)
            {
                hasThorn = true;
                break;
            }
        }
        //CHỈNh SKILL GIÁP GAI(DG)
        if (hasThorn)
        {
            //SÁT THƯNG PHẢN LẠI
            int reflectDmg = (int)(GetTotalDef(defender) * 3.0f);
            if (reflectDmg <= 0)
                reflectDmg = 1;
            attacker->hp -= reflectDmg;
            if (attacker->hp < 0)
                attacker->hp = 0;
            Vector2 refPos = attacker->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 300} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 300};
            SpawnFloatingText(TextFormat("PHAN -%d", reflectDmg), refPos, PURPLE, false);
        }

        if (skill->lifestealDmgPercent > 0.0f)
        {
            int healAmount = (int)(finalDmg * skill->lifestealDmgPercent);
            attacker->hp += healAmount;
            if (attacker->hp > attacker->maxHp)
                attacker->hp = attacker->maxHp;

            Vector2 healPos = attacker->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 300} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 300};
            SpawnFloatingText(TextFormat("+%d", healAmount), healPos, GREEN, false);
        }
    }

    if (skill->healByDefPercent > 0.0f)
    {
        int healAmount = (int)(GetTotalDef(attacker) * skill->healByDefPercent);
        attacker->hp += healAmount;
        if (attacker->hp > attacker->maxHp)
            attacker->hp = attacker->maxHp;

        Vector2 healPos = attacker->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 300} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 300};
        SpawnFloatingText(TextFormat("+%d", healAmount), healPos, GREEN, false);
    }

    // 6. CƠ CHẾ ÁP DỤNG HIỆU ỨNG VÀ KÍCH HOẠT HOẠT ẢNH
    if (skill->applyEffect != EFFECT_NONE)
    {
        if (strcmp(skill->name, u8"Hào Quang Huyết Sắc") == 0) {
            skill->effectValue = (int)(attacker->maxHp * 0.1f);
        }
        int chance = (skill->effectChance > 0) ? skill->effectChance : 100;
        if (GetRandomValue(1, 100) <= chance)
        {
            CombatEntity *target = defender;
           if (skill->applyEffect == EFFECT_HEAL || skill->applyEffect == EFFECT_DEF_UP ||
                skill->applyEffect == EFFECT_THORN_ARMOR || skill->applyEffect == EFFECT_SPD_UP || 
                skill->applyEffect == EFFECT_ATK_UP || skill->applyEffect == EFFECT_CRIT_UP)
            {
                target = attacker;
            }

            bool effectExists = false;
            for (int i = 0; i < MAX_EFFECTS; i++)
            {
                if (target->effects[i].type == skill->applyEffect && target->effects[i].duration > 0)
                {
                    target->effects[i].duration = skill->effectDuration;
                    target->effects[i].value = skill->effectValue;
                    effectExists = true;
                    break;
                }
            }

            // CHỈ BẮN HOẠT ẢNH BAY VỚI BUFF/DEBUFF MỚI
            if (!effectExists)
            {
                for (int i = 0; i < MAX_EFFECTS; i++)
                {
                    if (target->effects[i].type == EFFECT_NONE || target->effects[i].duration <= 0)
                    {
                        target->effects[i].type = skill->applyEffect;
                        target->effects[i].duration = skill->effectDuration;
                        target->effects[i].value = skill->effectValue;

                        // Kích hoạt Icon Animation
                        SpawnIconAnim(skill->applyEffect, target->isPlayer);
                        // --- THÊM ĐOẠN NÀY VÀO ---
                        if (skill->applyEffect == EFFECT_THORN_ARMOR)
                        {
                            isThornAnimating = true;
                            thornAnimTimer = 0.0f;
                        }
                        break;
                    }
                }
            }

            if (skill->applyEffect == EFFECT_STUN)
                LogMsg(u8"Mục tiêu đã bị CHOÁNG!");
        }
        else
        {
            if (skill->applyEffect == EFFECT_STUN)
                LogMsg(u8"Gây CHOÁNG hụt!");
        }
    }

    AddActionValue(attacker, skill->actionDelay);
    if (!attacker->isPlayer && strcmp(skill->name, u8"Dịch Chuyển Thời Không") == 0)
    {
        // Đẩy lùi thanh hành động của người chơi thêm 0.8 lượt
        AddActionValue(defender, 0.8f); 
        
        // Bắn chữ nổi thông báo cướp lượt
        Vector2 textPos = defender->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 350} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 350};
        SpawnFloatingText("LUI LUOT!", textPos, PURPLE, false);
    }
    // CHỈNH NỘI TẠI ĐÒN ĐÁNH(DG) Cơ chế cộng thủ vĩnh viễn cho đòn đánh thường của Đầu Gấu
    if (attacker->isPlayer && (attacker->pClass == CLASS_DAU_GAU || attacker->pClass == CLASS_STUDENT))
    {
        // Kiểm tra nếu tên kỹ năng là "Đánh Thường"
        if (strcmp(skill->name, u8"Đánh Thường") == 0)
        {
            attacker->permanentDefBonus += 12; // Cộng 3 thủ vào biến tích lũy vĩnh viễn

            // Hiện số thông báo màu xanh dương để biết là đã tăng thủ thành công
            SpawnFloatingText("+4 THU (VP)", (Vector2){180, SCREEN_HEIGHT - 350}, BLUE, false);
        }
    }
    // [CƠ CHẾ RIÊNG PHASE 2] 30% tỷ lệ gây Choáng của Đột Kích Phân Rã
    if (!attacker->isPlayer && strcmp(skill->name, u8"Đột Kích Phân Rã") == 0) {
        if (GetRandomValue(1, 100) <= 30) { 
            bool hasStun = false;
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (defender->effects[i].type == EFFECT_STUN && defender->effects[i].duration > 0) {
                    defender->effects[i].duration = 1;
                    hasStun = true; break;
                }
            }
            if (!hasStun) {
                for (int i = 0; i < MAX_EFFECTS; i++) {
                    if (defender->effects[i].type == EFFECT_NONE || defender->effects[i].duration <= 0) {
                        defender->effects[i].type = EFFECT_STUN;
                        defender->effects[i].duration = 1;
                        SpawnIconAnim(EFFECT_STUN, defender->isPlayer);
                        LogMsg(u8"Đột Kích Phân Rã gây CHOÁNG!");
                        break;
                    }
                }
            }
        }
    }
    // === [MỚI CHÈN] GẮN THÊM BUFF GIÁP CHO QUÁ TẢI HUYẾT THANH ===
    if (!attacker->isPlayer && strcmp(skill->name, u8"Quá Tải Huyết Thanh") == 0) {
        for (int i = 0; i < MAX_EFFECTS; i++) {
            // Tìm slot trống hoặc slot đã hết hạn để buff Giáp
            if (attacker->effects[i].type == EFFECT_NONE || attacker->effects[i].duration <= 0) {
                attacker->effects[i].type = EFFECT_DEF_UP;
                attacker->effects[i].duration = 2; // Buff tồn tại 2 lượt (bằng với ATK)
                attacker->effects[i].value = 100;   // Cộng 50 Giáp (Có thể chỉnh sửa số này)
                SpawnIconAnim(EFFECT_DEF_UP, attacker->isPlayer);
                LogMsg(u8"Quốc Trung gồng nộ: Tăng cả Công lẫn Thủ!");
                break;
            }
        }
    }
    // =========================================================
    // [HỌC BÁ] XỬ LÝ NỘI TẠI & HIỆU ỨNG KỸ NĂNG PHỤ
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_HOC_BA || attacker->pClass == CLASS_WARRIOR)) {
        
        // Nội tại(HBA): Đánh thường hồi 30 Thể lực
        if (strcmp(skill->name, u8"Đánh Thường") == 0) {
            attacker->stamina += 50;
            if (attacker->stamina > attacker->maxStamina) attacker->stamina = attacker->maxStamina;
            SpawnFloatingText("+30 TL", (Vector2){180, SCREEN_HEIGHT - 350}, YELLOW, false);
        }

        // Skill 1(HBA): Phản ứng hóa học -> Đẩy lùi Boss 0.5 lượt
        if (strcmp(skill->name, u8"Phản Ứng Hóa Học") == 0) {
            AddActionValue(defender, 0.7f);
            Vector2 txtPos = (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 350};
            SpawnFloatingText("LUI LUOT!", txtPos, PURPLE, false);
        }

        // Skill 2:(HBA) Tập trung cao độ -> Buff thêm DEF (ATK đã xử lý ở struct)
        if (strcmp(skill->name, u8"Tập Trung Cao Độ") == 0) {
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (attacker->effects[i].type == EFFECT_NONE || attacker->effects[i].duration <= 0) {
                    attacker->effects[i].type = EFFECT_DEF_UP;
                    attacker->effects[i].duration = 2;
                    attacker->effects[i].value = 160;
                    SpawnIconAnim(EFFECT_DEF_UP, true);
                    break;
                }
            }
        }

        // Skill 3: Cầu từ trường(HBA) -> 60% Choáng (DEF Down đã xử lý ở struct)
        if (strcmp(skill->name, u8"Cầu Từ Trường") == 0) {
            if (GetRandomValue(1, 100) <= 50) {
                for (int i = 0; i < MAX_EFFECTS; i++) {
                    if (defender->effects[i].type == EFFECT_NONE || defender->effects[i].duration <= 0) {
                        defender->effects[i].type = EFFECT_STUN;
                        defender->effects[i].duration = 2;
                        SpawnIconAnim(EFFECT_STUN, false);
                        break;
                    }
                }
            }
        }

        // Skill 4: Chiến thuật tư duy -> Hồi máu dựa trên mớ hỗn độn
        if (strcmp(skill->name, u8"Chiến Thuật Tư Duy") == 0) {
            // Đếm lượng Buff/Debuff đang có
            int effectCount = 0;
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (attacker->effects[i].duration > 0) effectCount++;
            }
            
            // Công thức(HBA): 100 máu gốc + 50 máu cho mỗi hiệu ứng
            int healAmount = 300 + (300 * effectCount);
            
            attacker->hp += healAmount;
            if (attacker->hp > attacker->maxHp) attacker->hp = attacker->maxHp;
            
            // Bắn chữ thông báo rất ngầu
            SpawnFloatingText(TextFormat("+%d HP (x%d Data)", healAmount, effectCount), (Vector2){180, SCREEN_HEIGHT - 350}, GREEN, false);
            LogMsg(TextFormat(u8"Chiến Thuật Tư Duy: Phân tích %d dữ liệu, Hồi %d HP!", effectCount, healAmount));
        }
    }

    // =========================================================
    // [SOÁI CA](SCA) XỬ LÝ NỘI TẠI & HIỆU ỨNG KỸ NĂNG PHỤ
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_SOAI_CA || attacker->pClass == CLASS_MAGE)) {
        
        // Nội tại: Đánh thường (Q) -> Thêm % Max HP vào sát thương 
        if (strcmp(skill->name, u8"Đánh Thường") == 0) {
            int extraDmg = (int)(attacker->maxHp * 0.2f);
            defender->hp -= extraDmg;
            if (defender->hp < 0) defender->hp = 0;
            defender->dmgTakenLastTurn += extraDmg;
            
            Vector2 textPos = defender->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 300} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 300};
            SpawnFloatingText(TextFormat("-%d (HUYET)", extraDmg), textPos, RED, false);
        }

        // Skill 2: Huyết Tiễn -> Hồi ngay 10% HP
        if (strcmp(skill->name, u8"Huyết Tiễn") == 0) {
            attacker->hp += (int)(attacker->maxHp * 0.2f);
            if (attacker->hp > attacker->maxHp) attacker->hp = attacker->maxHp;
            SpawnFloatingText("+200 HP", (Vector2){180, SCREEN_HEIGHT - 350}, GREEN, false);
        }

        // Skill 4: Giao Kèo Ác Quỷ -> Tự trừ 20% HP hiện tại, nhận thêm buff Heal 5% Max HP
        if (strcmp(skill->name, u8"Giao Kèo Ác Quỷ") == 0) {
            int hpCost = (int)(attacker->maxHp * 0.2f);
            attacker->hp -= hpCost;
            if (attacker->hp <= 0) attacker->hp = 1; // Không thể tự sát
            
            SpawnFloatingText(TextFormat("-%d (HIEN TE)", hpCost), (Vector2){180, SCREEN_HEIGHT - 350}, RED, false);
            
            // Cấp thêm buff Heal 5%
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (attacker->effects[i].type == EFFECT_NONE || attacker->effects[i].duration <= 0) {
                    attacker->effects[i].type = EFFECT_HEAL;
                    attacker->effects[i].duration = 2;
                    attacker->effects[i].value = (int)(attacker->maxHp * 0.05f);
                    SpawnIconAnim(EFFECT_HEAL, true);
                    break;
                }
            }
        }
    }
    // =========================================================
    // [PHÚ NHỊ ĐẠI] XỬ LÝ NỘI TẠI & KỸ NĂNG PHỤ
    // =========================================================
    if (attacker->isPlayer && (attacker->pClass == CLASS_PHU_NHI_DAI || attacker->pClass == CLASS_ARCHER)) {
        
        // Nội tại 1: Tích lũy Thể Lực -> Lên Crit
        if (skill->staminaCost > 0) {
            attacker->staminaSpent += skill->staminaCost;
            while (attacker->staminaSpent >= 35) {
                attacker->staminaSpent -= 50;
                if (attacker->permanentCritBonus < 20) { // Giới hạn 20%
                    attacker->permanentCritBonus += 4;
                    SpawnFloatingText("+4% CRIT (VP)", (Vector2){180, SCREEN_HEIGHT - 350}, YELLOW, false);
                }
            }
        }
        
        // Nội tại 2: Đánh thường +10 ATK (Giới hạn 50)
        if (strcmp(skill->name, u8"Cổ Tức Đều Đặn") == 0) {
            if (attacker->permanentAtkBonus < 50) {
                attacker->permanentAtkBonus += 10;
                SpawnFloatingText("+10 ATK (VP)", (Vector2){180, SCREEN_HEIGHT - 320}, RED, false);
            }
        }
        
        // Skill 3 (Giao Dịch Nội Gián): Cấp THÊM buff SPD_UP (Vì struct chỉ cho gán 1 buff)
        if (strcmp(skill->name, u8"Dò Tìm Con Mồi") == 0) {
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (attacker->effects[i].type == EFFECT_NONE || attacker->effects[i].duration <= 0) {
                    attacker->effects[i].type = EFFECT_SPD_UP;
                    attacker->effects[i].duration = 3;
                    attacker->effects[i].value = 50;
                    SpawnIconAnim(EFFECT_SPD_UP, true);
                    break;
                }
            }
        }
    }
    if (finalDmg > 0)
    {
        Audio_PlaySoundEffect(SFX_ATTACK);
        LogMsg(TextFormat(u8"%s dùng %s! Gây %d DMG.", attacker->name, skill->name, finalDmg));
    }
    else
    {
        Audio_PlaySoundEffect(SFX_UI_HOVER);
        LogMsg(TextFormat(u8"%s dùng %s!", attacker->name, skill->name));
    }
}

Rectangle GetBtnRect(int i)
{
    float btnW = 180.0f;
    float btnH = 55.0f;
    float spacingX = 25.0f;
    float spacingY = 15.0f;

    int col = i % 2;
    int row = i / 2;

    float startX = 350.0f;
    float startY = SCREEN_HEIGHT - 150.0f;

    float x = startX + col * (btnW + spacingX);
    float y = startY + row * (btnH + spacingY);

    return (Rectangle){x, y, btnW, btnH};
}

bool ProcessStartTurn(CombatEntity *ent);

void Combat_Update()
{
    if (!combatActive)
        return;
    stateTimer += GetFrameTime();
    // --- [NÂNG CẤP] BẬT/TẮT BẢNG LOG BẰNG PHÍM X ---
    if (IsKeyPressed(KEY_X)) {
        showLogWindow = !showLogWindow;
        Audio_PlaySoundEffect(SFX_UI_CLICK); 
    }
    
    // Khóa Update hành động nếu đang mở bảng Log (Tùy chọn, để game tạm dừng chờ bạn đọc)
    if (showLogWindow) return;
   // =========================================================
    // [HỌC BÁ] NỘI TẠI: ĐẶC QUYỀN SỬA SAI (TỪ CHỐI TỬ THẦN)
    // =========================================================
    if (entPlayer.hp <= 0 && (entPlayer.pClass == CLASS_HOC_BA || entPlayer.pClass == CLASS_WARRIOR) && !hasUsedRevive) {
        // ... (Logic hồi máu, thể lực, xóa debuff giữ nguyên) ...
        entPlayer.hp = 1;
        entPlayer.stamina = entPlayer.maxStamina;
        hasUsedRevive = true; 
        
        for (int i = 0; i < MAX_EFFECTS; i++) {
            // ... (xóa debuff giữ nguyên) ...
        }
        
        // --- [MỚI CHÈN] KÍCH HOẠT VFX TỎA SÁNG HỒI SINH ---
        reviveVFXTimer = 0.001f; // Bắt đầu đếm thời gian VFX (phải khác 0)
        // --------------------------------------------------
        
        SpawnFloatingText("SUA SAI!", (Vector2){180, SCREEN_HEIGHT - 350}, GOLD, true);
        LogMsg(u8"Nội tại [Đặc Quyền Sửa Sai]: Từ chối tử thần, hồi đầy Thể Lực!");
        screenShakeTimer = 0.5f; 
    }
    // =========================================================
    // =========================================================
    Vector2 mousePos = GetMouseScaled();
    bool isClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (showTurnAnim)
    {
        turnAnimTimer += GetFrameTime();
        if (turnAnimTimer > 1.5f)
            showTurnAnim = false;
    }

    float dt = GetFrameTime();
    cb_auraRot += 60.0f * dt;

    if (cb_bookState == CB_BOOK_FULLY_OPEN && cb_titleState == 0 && !cb_waitingToCloseBook)
    {
        cb_titleState = 1;
        cb_titleYOffset = -600.0f;
        cb_titleVelocity = 0.0f;
    }

    if (cb_titleState == 1)
    {
        cb_titleVelocity += 3500.0f * dt;
        cb_titleYOffset += cb_titleVelocity * dt;
        if (cb_titleYOffset >= 0.0f)
        {
            cb_titleYOffset = 0.0f;
            cb_titleVelocity *= -0.35f;
            if (fabs(cb_titleVelocity) < 60.0f)
                cb_titleState = 2;
        }
    }
    else if (cb_titleState == 3)
    {
        cb_titleVelocity -= 10000.0f * dt;
        cb_titleYOffset += cb_titleVelocity * dt;
        if (cb_titleYOffset < -600.0f)
        {
            cb_titleState = 0;
            if (cb_waitingToCloseBook)
            {
                cb_bookState = CB_BOOK_CLOSE_LEFT;
                cb_bookProgress = 0.0f;
                cb_waitingToCloseBook = false;
            }
        }
    }

    if (cb_bookState == CB_BOOK_DROPPING)
    {
        cb_bookVelocity += 10000.0f * dt;
        cb_bookDropY += cb_bookVelocity * dt;
        if (cb_bookDropY >= 0.0f)
        {
            cb_bookDropY = 0.0f;
            cb_bookVelocity *= -0.3f;
            if (fabs(cb_bookVelocity) < 100.0f)
            {
                cb_bookState = CB_BOOK_PAUSE;
                cb_pauseTimer = 0.0f;
            }
        }
    }
    else if (cb_bookState == CB_BOOK_PAUSE)
    {
        cb_pauseTimer += dt;
        if (cb_pauseTimer > 0.05f)
        {
            cb_bookState = CB_BOOK_OPEN_RIGHT;
            cb_bookProgress = 0.0f;
        }
    }
    else if (cb_bookState >= CB_BOOK_OPEN_RIGHT && cb_bookState <= CB_BOOK_CLOSE_RIGHT)
    {
        if (cb_bookState != CB_BOOK_FULLY_OPEN)
        {
            cb_bookProgress += 4.5f * dt;
            if (cb_bookProgress >= 1.0f)
            {
                cb_bookProgress = 1.0f;
                if (cb_bookState == CB_BOOK_OPEN_RIGHT)
                {
                    cb_bookState = CB_BOOK_OPEN_LEFT;
                    cb_bookProgress = 0.0f;
                }
                else if (cb_bookState == CB_BOOK_OPEN_LEFT)
                    cb_bookState = CB_BOOK_FULLY_OPEN;
                else if (cb_bookState == CB_BOOK_CLOSE_LEFT)
                {
                    cb_bookState = CB_BOOK_CLOSE_RIGHT;
                    cb_bookProgress = 0.0f;
                }
                else if (cb_bookState == CB_BOOK_CLOSE_RIGHT)
                {
                    cb_bookState = CB_BOOK_EXITING;
                    cb_bookVelocity = -500.0f;
                }
            }
        }
    }
    else if (cb_bookState == CB_BOOK_EXITING)
    {
        cb_bookVelocity -= 1000.0f * dt;
        cb_bookDropY += cb_bookVelocity * dt;
        if (cb_bookDropY < -1000.0f)
            cb_bookState = CB_BOOK_IDLE;
    }

    if (cb_bookState == CB_BOOK_FULLY_OPEN && IsKeyPressed(KEY_ESCAPE) && !cb_waitingToCloseBook)
    {
        if (cb_titleState == 2 || cb_titleState == 1)
        {
            cb_titleState = 3;
            cb_titleVelocity = -500.0f;
            cb_waitingToCloseBook = true;
            Audio_PlaySoundEffect(SFX_UI_CLICK);
        }
    }

    switch (combatState)
    {
    case COMBAT_STATE_INTRO:
        if (stateTimer > 1.5f)
            combatState = COMBAT_STATE_CALC_TURN;
        break;

   case COMBAT_STATE_CALC_TURN:
    {
        float minAV = entPlayer.actionValue < entBoss.actionValue ? entPlayer.actionValue : entBoss.actionValue;
        entPlayer.actionValue -= minAV;
        entBoss.actionValue -= minAV;

        if (entPlayer.actionValue <= 0)
        {
            bool isStunned = ProcessStartTurn(&entPlayer);
            
            // [FIXED] CHẶN ĐỨNG THÂY MA: Kiểm tra xem Player có chết vì Độc không
            if (entPlayer.hp <= 0) {
                combatState = COMBAT_STATE_DEFEAT;
                stateTimer = 0.0f;
                fadeAlpha = 0.0f;
                break; // Thoát ngay lập tức
            }

            if (isStunned)
            {
                LogMsg(u8"Bạn đang bị CHOÁNG! Bỏ qua lượt.");
                AddActionValue(&entPlayer, 1.0f);
                combatState = COMBAT_STATE_CALC_TURN;
            }
            else
            {
                combatState = COMBAT_STATE_PLAYER_TURN;
                strcpy(turnText, u8"LƯỢT CỦA BẠN!");
                turnColor = GREEN;
                turnAnimTimer = 0.0f;
                showTurnAnim = true;
            }
        }
        else
        {
            bool isStunned = ProcessStartTurn(&entBoss);
            
            // [FIXED] CHẶN ĐỨNG THÂY MA: Kiểm tra Boss có chết vì Độc không
            if (entBoss.hp <= 0) {
                combatState = COMBAT_STATE_VICTORY;
                stateTimer = 0.0f;
                fadeAlpha = 0.0f;
                break; // Thoát ngay lập tức
            }

            if (isStunned)
            {
                LogMsg(TextFormat(u8"%s đang bị CHOÁNG! Bỏ qua lượt.", entBoss.name));
                AddActionValue(&entBoss, 1.0f);
                combatState = COMBAT_STATE_CALC_TURN;
            }
            else
            {
                combatState = COMBAT_STATE_ENEMY_TURN;
                strcpy(turnText, u8"LƯỢT CỦA ĐỊCH!");
                turnColor = RED;
                turnAnimTimer = 0.0f;
                showTurnAnim = true;
                stateTimer = 0.0f;
            }
        }
        break;
    }
    case COMBAT_STATE_PLAYER_TURN:
        if (cb_bookState == CB_BOOK_IDLE)
        {
            if (IsKeyPressed(KEY_C) && cb_bookState == CB_BOOK_IDLE)
            {
                cb_bookState = CB_BOOK_DROPPING;
                cb_bookDropY = -1000.0f;
                cb_bookVelocity = 0.0f;
                cb_bookMode = 1;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
            for (int i = 0; i < 4; i++)
            {
                if (IsKeyPressed(KEY_Q + i) || (CheckCollisionPointRec(mousePos, GetBtnRect(i)) && isClick))
                {
                    int realKey = i;
                    if (i == 3 && IsKeyPressed(KEY_R))
                        realKey = 3;

                    if (realKey == 1)
                    {
                        if (cb_bookState == CB_BOOK_IDLE)
                        {
                            cb_bookState = CB_BOOK_DROPPING;
                            cb_bookDropY = -1000.0f;
                            cb_bookVelocity = 0.0f;
                            selectedBookSkill = -1;
                            cb_bookMode = 0;
                            Audio_PlaySoundEffect(SFX_UI_CLICK);
                        }
                    }
                    else
                    {
                       int dataIndex = 0;
                        if (realKey == 0) dataIndex = 0; // Q -> Đánh thường
                        if (realKey == 2) dataIndex = 1; // E -> Phòng Thủ
                        if (realKey == 3) dataIndex = 2; // R -> Tuyệt Kỹ

                        CombatSkill *chosenSkill = &entPlayer.skills[dataIndex];

                        if (chosenSkill->currentCooldown > 0)
                        {
                            LogMsg(TextFormat(u8"Đang hồi chiêu (%d lượt)!", chosenSkill->currentCooldown));
                            Audio_PlaySoundEffect(SFX_UI_HOVER);
                        }
                        else if (entPlayer.stamina >= chosenSkill->staminaCost)
                        {
                            activeAttacker = &entPlayer;
                            activeDefender = &entBoss;
                            activeSkill = chosenSkill;
                            hasDealtDamage = false;

                            if (cb_bookState == CB_BOOK_FULLY_OPEN)
                            {
                                cb_bookState = CB_BOOK_CLOSE_LEFT;
                                cb_bookProgress = 0.0f;
                            }
                            combatState = COMBAT_STATE_ACTION;
                            stateTimer = 0.0f;
                        }
                        else
                        {
                            LogMsg(u8"Không đủ Thể Lực!");
                            Audio_PlaySoundEffect(SFX_UI_HOVER);
                        }
                    }
                }
            }
        }
        else if (cb_bookState == CB_BOOK_FULLY_OPEN)
        {
            if (cb_waitingToCloseBook)
                break;
            float sw = (float)SCREEN_WIDTH;
            float sh = (float)SCREEN_HEIGHT;
            float bookWidth = sw * 0.8f;
            float bookHeight = sh * 0.75f;
            float startX = (sw - bookWidth) / 2.0f;
            float startY = (sh - bookHeight) / 2.0f + 20.0f;

            float paddingX = 25.0f;
            float gap = -25.0f;
            float pageWidth = (bookWidth - (paddingX * 2) - gap) / 2.0f;
            float leftCenterX = startX + paddingX + pageWidth / 2.0f;
            float rightCenterX = startX + paddingX + pageWidth + gap + pageWidth / 2.0f;

            for (int i = 0; i < 4; i++)
            {
                int col = i % 2;
                int row = i / 2;
                float slotSize = 70.0f;
                float slotSpacing = 40.0f;
                float startGridX = leftCenterX - ((slotSize * 2) + slotSpacing) / 2.0f;
                float startGridY = startY + 80.0f;

                Rectangle slotRec = {startGridX + col * (slotSize + slotSpacing), startGridY + row * (slotSize + 30.0f), slotSize, slotSize};

                if (CheckCollisionPointRec(mousePos, slotRec) && isClick)
                {
                    selectedBookSkill = i;
                    Audio_PlaySoundEffect(SFX_UI_HOVER);
                }
            }

            if (selectedBookSkill != -1)
            {
                float descY = startY + 80.0f;
                Rectangle useBtn = {rightCenterX - 90, descY + 200, 180, 50};

                if (CheckCollisionPointRec(mousePos, useBtn) && isClick)
                {
                    CombatSkill *chosen = &entPlayer.skills[3 + selectedBookSkill];
                    bool isSilenced = false;//check câm lặng
                    for (int k = 0; k < MAX_EFFECTS; k++) {
                        if (entPlayer.effects[k].duration > 0 && entPlayer.effects[k].type == EFFECT_SILENCE) {
                            isSilenced = true; break;
                        }
                    }

                    if (isSilenced)
                    {
                        LogMsg(u8"Bạn đang bị CÂM LẶNG! Không thể dùng sách.");
                        Audio_PlaySoundEffect(SFX_UI_HOVER);
                    }
                    else if (chosen->currentCooldown > 0)
                    {
                        LogMsg(TextFormat(u8"Đang hồi chiêu (%d lượt)!", chosen->currentCooldown));
                        Audio_PlaySoundEffect(SFX_UI_HOVER);
                    }
                    else if (entPlayer.stamina >= chosen->staminaCost)
                    {
                        activeAttacker = &entPlayer;
                        activeDefender = &entBoss;
                        activeSkill = chosen;
                        hasDealtDamage = false;

                        cb_titleState = 3;
                        cb_titleVelocity = -500.0f;
                        cb_waitingToCloseBook = true;

                        combatState = COMBAT_STATE_ACTION;
                        stateTimer = 0.0f;
                    }
                    else
                    {
                        LogMsg(u8"Không đủ Thể Lực!");
                        Audio_PlaySoundEffect(SFX_UI_HOVER);
                    }
                }
            }
        }
        break;
    case COMBAT_STATE_ENEMY_TURN:
        if (stateTimer > 1.2f)
        {
           int skillIndex = 0;
            if (currentPhase == 1) {
                // (Logic cũ của Phase 1 giữ nguyên)
                if (entBoss.hp < entBoss.maxHp / 2 && GetRandomValue(1, 100) <= 40) {
                    skillIndex = 2; 
                } else {
                    skillIndex = GetRandomValue(0, 1);
                }
            }
            else if (currentPhase == 2) {
                // Kiểm tra xem Boss đã có buff ATK chưa
                bool hasAtkBuff = false;
                for (int i = 0; i < MAX_EFFECTS; i++) {
                    if (entBoss.effects[i].type == EFFECT_ATK_UP && entBoss.effects[i].duration > 0) hasAtkBuff = true;
                }

                if (entBoss.hp < entBoss.maxHp * 0.4f && GetRandomValue(1, 100) <= 30) {
                    skillIndex = 2; // Chiêu cuối: Nghịch Lý Hủy Diệt
                } else if (!hasAtkBuff && GetRandomValue(1, 100) <= 50) {
                    skillIndex = 1; // Buff: Quá Tải Huyết Thanh
                } else {
                    // Random 1 trong 3 chiêu tấn công: [0] Đột kích, [3] Lazer, [4] Dịch chuyển
                    int attackSkills[] = {0, 3, 4};
                    int randomIndex = GetRandomValue(0, 2);
                    skillIndex = attackSkills[randomIndex];
                }
            }

            activeAttacker = &entBoss;
            // ... (Đoạn dưới giữ nguyên)
            activeDefender = &entPlayer;
            activeSkill = &entBoss.skills[skillIndex];
            hasDealtDamage = false;
            combatState = COMBAT_STATE_ACTION;
            stateTimer = 0.0f;
        }
        break;

  case COMBAT_STATE_ACTION:
        if (cb_bookState != CB_BOOK_IDLE) {
            stateTimer = 0.0f; 
            break;
        }

        // --- [SỬA LỖI] HỆ THỐNG PHÁT ÂM THANH SFX CĂN TIMING ---
        static bool hasPlayedPrepSound = false;
        static bool hasPlayedBoomSound = false;
        static float arrowSpamTimer = 0.0f; // [MỚI] Biến đếm nhịp bắn tên liên thanh
        static float loopTimer = 0.0f;

        // Reset cờ an toàn: Chỉ cần hoạt ảnh ở những frame đầu tiên (nhỏ hơn 0.2 giây)
        // Cách này chống kẹt cờ tuyệt đối dù FPS của game có thay đổi
        if (stateTimer < 0.2f) {
            hasPlayedPrepSound = false;
            hasPlayedBoomSound = false;
            arrowSpamTimer = 0.0f; // [MỚI] Reset lại đếm nhịp
            loopTimer = 0.0f;
        }

        // Kiểm tra xem có đúng là Đầu Gấu đang dùng Đánh Thường không
        bool isDauGauBasic = (activeAttacker->isPlayer && 
                             (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                              activeSkill != NULL && strcmp(activeSkill->name, u8"Đánh Thường") == 0);
        //kiểm tra kẻ chịu đòn
        bool isKeChiuDon = (activeAttacker->isPlayer && 
                           (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                            activeSkill != NULL && strcmp(activeSkill->name, u8"Kẻ Chịu Đòn") == 0);
        // ... [Bên dưới các biến isDauGauBasic và isKeChiuDon] ...
        bool isCuDamSamSet = (activeAttacker->isPlayer && 
                             (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                              activeSkill != NULL && strcmp(activeSkill->name, u8"Cú Đấm Sấm Sét") == 0);
        bool isRungChan = (activeAttacker->isPlayer && 
                          (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                           activeSkill != NULL && strcmp(activeSkill->name, u8"Rung Chấn") == 0);
        bool isGiapGai = (activeAttacker->isPlayer && 
                         (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                          activeSkill != NULL && strcmp(activeSkill->name, u8"Giáp Gai") == 0);
        bool isLayThuBuCong = (activeAttacker->isPlayer && 
                              (activeAttacker->pClass == CLASS_DAU_GAU || activeAttacker->pClass == CLASS_STUDENT) && 
                               activeSkill != NULL && strcmp(activeSkill->name, u8"Lấy Thủ Bù Công") == 0);
        bool isHocBa = (activeAttacker->isPlayer && 
                       (activeAttacker->pClass == CLASS_HOC_BA || activeAttacker->pClass == CLASS_WARRIOR) && 
                        activeSkill != NULL);
        
        bool isSoaiCa = (activeAttacker->isPlayer && 
                        (activeAttacker->pClass == CLASS_SOAI_CA|| activeAttacker->pClass == CLASS_MAGE) && 
                         activeSkill != NULL);
        bool isPhuNhiDai = (activeAttacker->isPlayer && 
                           (activeAttacker->pClass == CLASS_PHU_NHI_DAI || activeAttacker->pClass == CLASS_ARCHER) && 
                            activeSkill != NULL);
        if (isDauGauBasic) {
            // Giây 0.5: Phát âm thanh lấy đà
            if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                hasPlayedPrepSound = true;
            }
            // Giây 3.0: Phát âm thanh nổ
            if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                hasPlayedBoomSound = true;
            }
        }
        else if (isKeChiuDon) {
            if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_KECHIUDON);
                hasPlayedPrepSound = true; // Khóa lại ngay lập tức
            }
        }
        // [MỚI CHÈN] Xử lý Cú Đấm Sấm Sét
        else if (isCuDamSamSet) {
            // Giây 0.5: Tích tụ điện
            if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_CUDAMSAMSET_PREP);
                hasPlayedPrepSound = true;
            }
            // Giây 3.0: Đánh giáng sấm sét xuống
            if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);
                hasPlayedBoomSound = true;
            }
        }
        else if (isRungChan) {
            // Giây 0.5: Phát tiếng lấy đà động đất
            if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_RUNGCHAN_PREP);
                hasPlayedPrepSound = true;
            }
            // Giây 3.0: Phát tiếng nổ lớn
            if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                hasPlayedBoomSound = true;
            }
        }
        else if (isGiapGai) {
            // Giây 2.0: Phát tiếng gai đâm tủa ra
            if (stateTimer >= 2.0f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_GIAPGAI);
                hasPlayedPrepSound = true; // Khóa lại ngay lập tức
            }
        }
        // 5. CHIÊU CUỐI (LẤY THỦ BÙ CÔNG)
        else if (isLayThuBuCong) {
            // Giây 0.5: Tái sử dụng tiếng lấy đà của Rung Chấn
            if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                Audio_PlaySoundEffect(SFX_RUNGCHAN_PREP);
                hasPlayedPrepSound = true;
            }
            // Giây 3.0: Tái sử dụng tiếng bùng nổ
            if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                hasPlayedBoomSound = true;
            }
        }
        // ==========================================
        // XỬ LÝ ÂM THANH CHO CLASS HỌC BÁ
        // ==========================================
        else if (isHocBa) {
            // 1. Phản Ứng Hóa Học
            if (strcmp(activeSkill->name, u8"Phản Ứng Hóa Học") == 0) {
                // Giây 0.5: Ném bình độc và phát ra phản ứng
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_PHANUNGHOAHOC);
                    hasPlayedPrepSound = true;
                }
            }
            // 2. [MỚI CHÈN] Đánh Thường (Học Bá)
            else if (strcmp(activeSkill->name, u8"Đánh Thường") == 0) {
                // Giây 0.5: Tái sử dụng tiếng lấy đà
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Tái sử dụng tiếng nổ
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            else if (strcmp(activeSkill->name, u8"Tập Trung Cao Độ") == 0) {
                // Giây 2.0: Phát âm thanh buff
                if (stateTimer >= 2.0f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_TAPTRUNGCAODO);
                    hasPlayedPrepSound = true;
                }
            }
            // 4. [MỚI CHÈN] Cầu Từ Trường
            else if (strcmp(activeSkill->name, u8"Cầu Từ Trường") == 0) {
                // Giây 0.5: Phát tiếng tụ từ trường
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_CAUTUTRUONG_PREP);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Tận dụng tiếng sét nổ tung (Lightning Hit Boom)
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);
                    hasPlayedBoomSound = true;
                }
            }
            else if (strcmp(activeSkill->name, u8"Chiến Thuật Tư Duy") == 0) {
                // Giây 0.5: Tái sử dụng tiếng gồng của Kẻ Chịu Đòn
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_KECHIUDON);
                    hasPlayedPrepSound = true;
                }
            }
            // 6. [MỚI CHÈN] Tuyệt Kỹ: Định Lý Cuối Cùng
            else if (strcmp(activeSkill->name, u8"Định Lý Cuối Cùng") == 0) {
                // Giây 0.2: Phát ngay lập tức tiếng lấy đà (Mốc 0.2f để né vòng lặp reset cờ)
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DINHLICUOICUNG_PREP);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Gọi CÙNG LÚC 2 hiệu ứng Nổ và Sét
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);
                    hasPlayedBoomSound = true;
                }
            }
        }
        // ==========================================
        // XỬ LÝ ÂM THANH CHO CLASS SOÁI CA
        // ==========================================
        else if (isSoaiCa) {
            // 1. Đánh Thường (Soái Ca)
            if (strcmp(activeSkill->name, u8"Đánh Thường") == 0) {
                // Giây 0.5: Tái sử dụng tiếng lấy đà của Đầu Gấu
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Tái sử dụng tiếng nổ
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            // 2. Huyết Tiễn
            else if (strcmp(activeSkill->name, u8"Huyết Tiễn") == 0) {
                // Giây 0.5: Tái chế tiếng lấy đà của Đầu Gấu
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG); 
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Tiếng máu bắn trúng đích
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_SOAICA_HUYETTIEN);
                    hasPlayedBoomSound = true;
                }
            }
            // 3. Hào Quang Huyết Sắc (Buff)
            else if (strcmp(activeSkill->name, u8"Hào Quang Huyết Sắc") == 0) {
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_SOAICA_HAOQUANG);
                    hasPlayedPrepSound = true;
                }
            }
            // 4. Dấu Ấn Ký Sinh (Nhớ giữ nguyên \n trong tên chiêu)
            else if (strcmp(activeSkill->name, u8"Dấu Ấn Ký Sinh") == 0) {
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_SOAICA_DAUAN);
                    hasPlayedPrepSound = true;
                }
            }
            // 5. Giao Kèo Ác Quỷ (Buff)
            else if (strcmp(activeSkill->name, u8"Giao Kèo Ác Quỷ") == 0) {
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_SOAICA_GIAOKEO);
                    hasPlayedPrepSound = true;
                }
            }
           // 6. Tuyệt Kỹ: Vạn Tiễn Xuyên Tâm
            else if (strcmp(activeSkill->name, u8"Vạn Tiễn Xuyên Tâm") == 0) {
                // Phase 1: Mưa tên liên tục (Từ 0.5s đến 2.8s)
                if (stateTimer >= 0.5f && stateTimer < 2.8f) {
                    arrowSpamTimer += GetFrameTime();
                    
                    // Cứ mỗi 0.15 giây sẽ phát ra 1 cặp âm thanh: Tiếng Bay + Tiếng Cắm vào người
                    if (arrowSpamTimer >= 0.15f) { 
                        Audio_PlaySoundEffect(SFX_SOAICA_SINGLE_ARROW); // Tiếng xé gió
                        Audio_PlaySoundEffect(SFX_SOAICA_HUYETTIEN);    // Tiếng "phập" chạm địch
                        
                        arrowSpamTimer = 0.0f; // Reset nhịp đếm
                    }
                }
                
                // Phase 2: Giây 3.0 - Cú nổ chốt hạ kết thúc chiêu
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S); // Cú nổ âm thanh có sẵn
                    hasPlayedBoomSound = true;
                }
            }   
        }
        // ==========================================
        // XỬ LÝ ÂM THANH CHO CLASS PHÚ NHỊ ĐẠI
        // ==========================================
        else if (isPhuNhiDai) {
            // 0. Đánh Thường: Cổ Tức Đều Đặn
            if (strcmp(activeSkill->name, u8"Cổ Tức Đều Đặn") == 0) {
                // Giây 0.5: Phát CÙNG LÚC tiếng đấm Đầu gấu và Xu xoay
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_COINSPIN);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Tiếng nổ
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            // 1. W1: Thu Hồi Vốn (Y hệt đánh thường Đầu Gấu)
            else if (strcmp(activeSkill->name, u8"Thu Hồi Vốn") == 0) {
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                    hasPlayedPrepSound = true;
                }
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            // 2. W2: Phá Giá Thị Trường (Đầu Gấu + Nổ + Sét)
            else if (strcmp(activeSkill->name, u8"Phá Giá Thị Trường") == 0) {
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_DAUGAU_DANHTHUONG);
                    hasPlayedPrepSound = true;
                }
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);
                    hasPlayedBoomSound = true;
                }
            }
            // 3. W3: Dò Tìm Con Mồi (Chia 2 Phase)
            else if (strcmp(activeSkill->name, u8"Dò Tìm Con Mồi") == 0) {
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_DOTIM_PREP);
                    hasPlayedPrepSound = true;
                }
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_DOTIM_BOOM);
                    hasPlayedBoomSound = true;
                }
            }
            
            // 4. W4: Triệt Hạ Con Mồi (Xả đạn liên tục từ 0.2s đến 3.0s)
           // 3. W3: Dò Tìm Con Mồi (Giờ chỉ còn tiếng vận chiêu lúc đầu)
            else if (strcmp(activeSkill->name, u8"Dò Tìm Con Mồi") == 0) {
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_DOTIM_PREP);
                    hasPlayedPrepSound = true;
                }
                // Đã xóa phần tiếng nổ ở giây 3.0
            }
            
            // 4. W4: Triệt Hạ Con Mồi (Khởi động + Xả đạn liên tục)
            else if (strcmp(activeSkill->name, u8"Triệt Hạ Con Mồi") == 0) {
                // Giây 0.2: Phát tiếng khởi động uy lực (lấy từ tiếng Boom của chiêu 3 cũ)
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_DOTIM_BOOM); // Tiếng nòng súng/chuẩn bị
                    hasPlayedPrepSound = true;
                    
                    // Có thể phát luôn viên đạn đầu tiên ở đây nếu muốn
                    // Audio_PlaySoundEffect(SFX_PHUNHIDAI_TRIETHA); 
                    
                    loopTimer = 0.0f; // Bắt đầu tính giờ xả đạn
                }
                
                // Từ 0.2s đến 3.0s: Bắt đầu xả đạn liên tục
                if (stateTimer >= 0.2f && stateTimer < 3.0f) {
                    loopTimer += GetFrameTime();
                    
                    // Cứ mỗi 0.3 giây phát tiếng xả đạn 1 lần
                    if (loopTimer >= 0.1f) { 
                        Audio_PlaySoundEffect(SFX_PHUNHIDAI_TRIETHA); // File xả đạn 0.3s
                        loopTimer = 0.0f; // Reset đếm lại từ đầu
                    }
                }
            }
            // 5. Tuyệt Kỹ (R): Canh Bạc Tất Tay (Bành Trướng Lãnh Địa)
            else if (strcmp(activeSkill->name, u8"Canh Bạc Tất Tay") == 0) {
                
                // Phase 1 (0.2s): Mở Lãnh Địa - Bẻ cong không gian & Tung đồng xu
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    // Dùng hàm Multi để các âm thanh không tự đè nhau tắt mất
                    Audio_PlaySoundEffect(SFX_BOSS_TELEGATE);       // Tiếng bẻ cong thời không (Domain Expansion)
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_COINSPIN);  // Tiếng đồng xu xoay vòng định mệnh
                    hasPlayedPrepSound = true;
                    
                    loopTimer = 0.0f; // Mượn tạm biến loopTimer làm cờ đánh dấu cho Phase 2
                }
                
                // Phase 2 (1.5s): Nạp năng lượng tuyệt đối
                // (Khi không gian đã mở xong, bắt đầu rút 99% HP để tụ lực)
                if (stateTimer >= 1.5f && loopTimer == 0.0f) {
                    Audio_PlaySoundEffect(SFX_BOSS_PHAOHODEN); // Tiếng hố đen rít lên tĩnh mịch và rợn người
                    loopTimer = 1.0f; // Đánh dấu là đã phát Phase 2
                }
                
                // Phase 3 (3.0s): TẤT TAY - Sự kiện tận thế (Nổ + Sét + Chấn động)
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);          // Tiếng nổ tung rực lửa
                    Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);      // Sấm sét giáng xuống gầm trời
                    Audio_PlaySoundEffect(SFX_PHUNHIDAI_DOTIM_BOOM);  // Âm thanh chốt hạ chát chúa của giới tư bản
                    
                    hasPlayedBoomSound = true;
                }
            }
        }
        // ... (Code cũ của Đầu Gấu và Học Bá)

        // ==========================================
        // XỬ LÝ ÂM THANH CHO BOSS
        // ==========================================
        // Nhận diện Boss (không phải là người chơi)
        else if (!activeAttacker->isPlayer && activeSkill != NULL) {
            
            // 1. Lazer Hủy Diệt (Dùng chung cho cả Phase 1 và Phase 2)
            if (strcmp(activeSkill->name, u8"Lazer Hủy Diệt") == 0) {
                // Giây 0.2: Bắt đầu tụ hố đen và nạp năng lượng
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_LAZER);
                    hasPlayedPrepSound = true;
                }
            }
            else if (strcmp(activeSkill->name, u8"Dịch Chuyển Thời Không") == 0) {
                // Giây 0.2: Phát tiếng vận chiêu xoáy hố đen
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_TELEGATE);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Bùng nổ ánh sáng trắng tím (Gọi kèm tiếng nổ)
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            // 3. [MỚI CHÈN] Tuyệt Kỹ: Pháo Hố Đen
            else if (strcmp(activeSkill->name, u8"Pháo Hố Đen") == 0) {
                // Giây 0.2: Phát tiếng tụ hố đen (Kéo dài đến 3s)
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_PHAOHODEN);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Bùng nổ (Sử dụng lại tiếng nổ to)
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            else if (strcmp(activeSkill->name, u8"Đột Kích Phân Rã") == 0) {
                // Giây 0.5: Tiếng lao tới và tụ lực nắm đấm
                if (stateTimer >= 0.5f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_DOTKICHPHANRA);
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Bùng nổ hố đen (Kèm với sát thương gốc của chiêu)
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    hasPlayedBoomSound = true;
                }
            }
            // 5. [MỚI CHÈN] Quá Tải Huyết Thanh (Phase 2 - Buff)
            else if (strcmp(activeSkill->name, u8"Quá Tải Huyết Thanh") == 0) {
                // Giây 0.2: Phát tiếng gồng máu, sôi sục huyết thanh
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_QUATAIHUYETTHANH);
                    hasPlayedPrepSound = true;
                }
            }
            // 6. [MỚI CHÈN] Tuyệt Kỹ: Nghịch Lý Hủy Diệt (Phase 2)
            else if (strcmp(activeSkill->name, u8"Nghịch Lý Hủy Diệt") == 0) {
                // Giây 0.2: Tái sử dụng âm thanh xoáy không gian của Chiêu 2 (Dịch Chuyển Thời Không)
                if (stateTimer >= 0.2f && !hasPlayedPrepSound) {
                    Audio_PlaySoundEffect(SFX_BOSS_TELEGATE); 
                    // (Mẹo: Nếu ý bạn "chiêu 2" là Quá Tải Huyết Thanh thì đổi thành SFX_BOSS_QUATAIHUYETTHANH nhé)
                    
                    hasPlayedPrepSound = true;
                }
                // Giây 3.0: Sự kiện tận thế - Gọi CÙNG LÚC Nổ và Sấm Sét
                if (stateTimer >= 3.0f && !hasPlayedBoomSound) {
                    Audio_PlaySoundEffect(SFX_EXPLOSION_3S);
                    Audio_PlaySoundEffect(SFX_CUDAMSAMSET_BOOM);
                    hasPlayedBoomSound = true;
                }
            }
            
            // Các chiêu khác của Boss sẽ được chèn thêm vào đây...
        }
        
        // ---------------------------------------------------------

        // Tiếp tục đếm thời gian cho animation
        stateTimer += GetFrameTime();
        
        if (screenShakeTimer > 0.0f)
            screenShakeTimer -= GetFrameTime();

        if (stateTimer >= 3.0f && !hasDealtDamage)
        {
            ExecuteAction(activeAttacker, activeDefender, activeSkill);
            hasDealtDamage = true;
            if (activeAttacker->isPlayer)
                screenShakeTimer = 0.3f;
        }

        if (stateTimer > 3.7f)
        {
            if (entBoss.hp <= 0)
            {
                combatState = COMBAT_STATE_VICTORY;
                stateTimer = 0.0f;
                fadeAlpha = 0.0f;
            }
            else if (entPlayer.hp <= 0)
            {
                combatState = COMBAT_STATE_DEFEAT;
                stateTimer = 0.0f;
                fadeAlpha = 0.0f;
            }
            else {
                // RESET TRẠNG THÁI ANIMATION KHI KẾT THÚC HÀNH ĐỘNG
                currentActionPhase = ACTION_PHASE_IDLE;
                combatState = COMBAT_STATE_CALC_TURN;
                // Lưu ý: dashOffset và jumpOffset sẽ tự về 0 ở frame tiếp theo 
                // vì chúng được tính toán dựa trên stateTimer trong Combat_Draw
            }
        }
        break;

    case COMBAT_STATE_VICTORY:
    case COMBAT_STATE_DEFEAT:
        fadeAlpha += GetFrameTime() * 0.6f;
        if (fadeAlpha > 1.0f)
            fadeAlpha = 1.0f;
        if (stateTimer > 3.5f)
        {
            combatResult = (combatState == COMBAT_STATE_VICTORY) ? 1 : -1;
            refPlayer->stats.currentHp = (combatResult == 1) ? entPlayer.hp : 1;
            refPlayer->stats.stamina = entPlayer.stamina;
            combatActive = false;
            Audio_PlayMusic(MUSIC_LAB);
        }
        break;
    }
}

void DrawSkillButton(Rectangle rect, const char *key, const char *name, int dmg, int cost, bool isHover, bool canCast)
{
    Rectangle srcRect = (!canCast) ? SRC_BTN_LOCK : (isHover ? SRC_BTN_HOVER : SRC_BTN_NORMAL);
    NPatchInfo nPatch = {.source = srcRect, .left = 4, .top = 4, .right = 4, .bottom = 4, .layout = NPATCH_NINE_PATCH};
    DrawTextureNPatch(texCombatUI, nPatch, rect, (Vector2){0, 0}, 0.0f, WHITE);

    DrawTextEx(globalFont, TextFormat("%s (%s)", name, key), (Vector2){rect.x + 15, rect.y + 8}, 18, 1, canCast ? BLACK : DARKGRAY);
    if (dmg > 0)
        DrawTextEx(globalFont, TextFormat("DMG: %d", dmg), (Vector2){rect.x + 15, rect.y + 30}, 16, 1, Fade(DARKBLUE, canCast ? 1.0f : 0.5f));
    else
        DrawTextEx(globalFont, "DEFEND", (Vector2){rect.x + 15, rect.y + 30}, 16, 1, Fade(DARKGREEN, canCast ? 1.0f : 0.5f));

    if (cost > 0)
        DrawTextEx(globalFont, TextFormat("TL: -%d", cost), (Vector2){rect.x + 110, rect.y + 30}, 16, 1, Fade(RED, canCast ? 1.0f : 0.5f));
    else if (cost < 0)
        DrawTextEx(globalFont, TextFormat("TL: +%d", -cost), (Vector2){rect.x + 110, rect.y + 30}, 16, 1, Fade(GREEN, canCast ? 1.0f : 0.5f));
}

void DrawStatusIcons(CombatEntity *ent, Vector2 startPos)
{
    int iconCount = 0;
    float iconSize = 35.0f;

    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (ent->effects[i].duration > 0)
        {
            Texture2D *tex = NULL;
            bool isDown = false;

            switch (ent->effects[i].type)
            {
            
            case EFFECT_POISON:
                tex = &texIconPoison;
                break;
            case EFFECT_SILENCE:
                tex = &texIconSilence;
                break;
            case EFFECT_HEAL:
                tex = &texIconUpHp;
                break;
            case EFFECT_DEF_UP:
                tex = &texIconUpDef;
                break;
            case EFFECT_DEF_DOWN:
                tex = &texIconUpDef;
                isDown = true;
                break;
            case EFFECT_ATK_DOWN:
                tex = &texIconUpAtk; // Dùng lại ảnh cái kiếm
                isDown = true;       // Lật mũi tên đỏ chúc xuống
                break;
            case EFFECT_ATK_UP:
                tex = &texIconUpAtk;
                break;
            case EFFECT_STUN:
                tex = &texIconStun;
                break;
            case EFFECT_CRIT_UP:
                tex = &texIconUpCrit;
                break;
            default:
                break;
            }

            if (tex != NULL && tex->id != 0)
            {
                Vector2 drawPos = {startPos.x + iconCount * (iconSize + 8), startPos.y};

                DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height}, (Rectangle){drawPos.x, drawPos.y, iconSize, iconSize}, (Vector2){0, 0}, 0.0f, WHITE);

                if (isDown)
                {
                    DrawLineEx((Vector2){drawPos.x + iconSize, drawPos.y}, (Vector2){drawPos.x + iconSize, drawPos.y + iconSize}, 3.0f, RED);
                    DrawTriangle((Vector2){drawPos.x + iconSize - 4, drawPos.y + iconSize - 4}, (Vector2){drawPos.x + iconSize + 4, drawPos.y + iconSize - 4}, (Vector2){drawPos.x + iconSize, drawPos.y + iconSize + 4}, RED);
                }

                DrawTextEx(globalFont, TextFormat("%d", ent->effects[i].duration), (Vector2){drawPos.x + 2, drawPos.y + iconSize - 16}, 25, 1, RED);
                iconCount++;
            }
        }
    }
}

bool ProcessStartTurn(CombatEntity *ent)
{
    bool wasStunned = false;

    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (ent->effects[i].duration > 0 && ent->effects[i].type == EFFECT_STUN)
        {
            wasStunned = true;
        }
    }

    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (ent->effects[i].duration > 0)
        {
            // [MỚI CHÈN] XỬ LÝ SÁT THƯƠNG ĐỘC TRƯỚC KHI TRỪ HIỆP
            // ==========================================
            if (ent->effects[i].type == EFFECT_POISON)
            {
                ent->hp -= ent->effects[i].value;
                if (ent->hp < 0) ent->hp = 0;

                // Bắn chữ nổi màu Tím để người chơi biết mình đang bị rút máu do Độc
                Vector2 textPos = ent->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 350} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 350};
                SpawnFloatingText(TextFormat("-%d DOC", ent->effects[i].value), textPos, PURPLE, false);
            }
            //heal mỗi lượt
            else if (ent->effects[i].type == EFFECT_HEAL)
            {
                ent->hp += ent->effects[i].value;
                if (ent->hp > ent->maxHp) ent->hp = ent->maxHp; // Không cho hồi lố máu tối đa
                Vector2 textPos = ent->isPlayer ? (Vector2){180, SCREEN_HEIGHT - 350} : (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 350};
                SpawnFloatingText(TextFormat("+%d HP", ent->effects[i].value), textPos, GREEN, false);
            }
            ent->effects[i].duration--;
            if (ent->effects[i].duration <= 0)
            {
                ent->effects[i].type = EFFECT_NONE;
            }
        }
    }

    for (int i = 0; i < 7; i++)
    {
        if (ent->skills[i].currentCooldown > 0)
        {
            ent->skills[i].currentCooldown--;
        }
    }
    //CHỈNH NỘI tại ĐẦU GẤU ĂN ĐÒN TĂNG GIÁP    (DG)
    if (ent->isPlayer && (ent->pClass == CLASS_DAU_GAU || ent->pClass == CLASS_STUDENT))
    {
        if (ent->dmgTakenLastTurn >= 200)
        {
            int times = ent->dmgTakenLastTurn / 200;//ST nhận vào
            int bonus = times * 15;//số gipas 

            ent->permanentDefBonus += bonus;
            SpawnFloatingText(TextFormat("+%d THU", bonus), (Vector2){180, SCREEN_HEIGHT - 300}, BLUE, false);

            ent->dmgTakenLastTurn = ent->dmgTakenLastTurn % 200;
        }
    }
    // [SOÁI CA](sca NỘI TẠI NỘI TẠI MỚI: Cuồng Huyết (Tăng 5% Giới hạn Máu mỗi hiệp)
    if (ent->isPlayer && (ent->pClass == CLASS_SOAI_CA || ent->pClass == CLASS_MAGE)) {
        int hpBonus = (int)(ent->maxHp * 0.04f); // Tính 5% máu tối đa hiện tại
        if (hpBonus > 0) {
            ent->maxHp += hpBonus; // Mở rộng giới hạn máu
            ent->hp += hpBonus;    // Hồi luôn lượng máu vừa được mở rộng để thanh HP không bị hụt
            
            // Bắn chữ thông báo màu Tím Hồng (MAGENTA) bay lên cho ngầu, lệch Y một chút để không đè chữ khác
            SpawnFloatingText(TextFormat("+%d MAX HP", hpBonus), (Vector2){180, SCREEN_HEIGHT - 380}, MAGENTA, false);
        }
    }   
    // [SOÁI CA] NỘI TẠI: Thức Tỉnh Huyết Tộc (Dưới 40% HP hồi 400 HP/lượt)
    if (ent->isPlayer && (ent->pClass == CLASS_SOAI_CA || ent->pClass == CLASS_MAGE)) {
        if (!ent->isResilienceActive && ent->hp > 0 && ent->hp < (ent->maxHp * 0.4f)) {
            ent->isResilienceActive = true; // Đánh dấu đã dùng 1 lần trong trận
            
            // Cấp buff Hồi 400 máu trong 2 lượt
            for (int i = 0; i < MAX_EFFECTS; i++) {
                if (ent->effects[i].type == EFFECT_NONE || ent->effects[i].duration <= 0) {
                    ent->effects[i].type = EFFECT_HEAL;
                    ent->effects[i].duration = 2;
                    ent->effects[i].value = 400;
                    SpawnIconAnim(EFFECT_HEAL, true);
                    break;
                }
            }
            SpawnFloatingText("THUC TINH HUYET TOC!", (Vector2){180, SCREEN_HEIGHT - 350}, RED, true);
            LogMsg(u8"Mị Lực Huyết Tộc: HP dưới 40%, bùng nổ khả năng hồi phục!");
        }
    }
    return wasStunned;
}

// =========================================================
// HỆ THỐNG VẼ VFX KỸ NĂNG ĐỘC LẬP
// =========================================================
// 1_ĐẦU GẤU_SKILL GIÁP GAI
void DrawSkillVFX_GiapGai(Vector2 targetPos)
{
    if (texThornVFX.id == 0)
        return; // Nếu chưa có ảnh thì bỏ qua

    thornRotation += 150.0f * GetFrameTime();

    float currentThornScale = 0.3f; // Kích thước ôm người
    float maxThornSize = 0.4f;       // Kích thước phình to

    if (isThornAnimating)
    {
        thornAnimTimer += GetFrameTime();
        float p = thornAnimTimer / 0.8f;

        if (p < 0.4f)
        {
            currentThornScale = (p / 0.4f) * maxThornSize;
        }
        else if (p < 0.8f)
        {
            currentThornScale = maxThornSize - (((p - 0.4f) / 0.4f) * (maxThornSize - 0.65f));
        }
        else
        {
            isThornAnimating = false;
        }
    }

    Rectangle destRec = {targetPos.x, targetPos.y,
                         (float)texThornVFX.width * currentThornScale,
                         (float)texThornVFX.height * currentThornScale};
    Vector2 origin = {destRec.width / 2.0f, destRec.height / 2.0f};

    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(texThornVFX,
                   (Rectangle){0, 0, (float)texThornVFX.width, (float)texThornVFX.height},
                   destRec, origin, thornRotation, Fade(SKYBLUE, 0.9f));
    EndBlendMode();
}

// =========================================================
// 2_ĐẦU GẤU_SKILL KẺ CHỊU ĐÒN
// =========================================================
void DrawSkillVFX_KeChiuDon(Vector2 pPos, float timer, float jumpOffset) {
    if (texKeChiuDonVFX.id == 0) return;

    Vector2 center = { pPos.x, pPos.y - jumpOffset };
    
    // Ép size: Dù ảnh 1024 hay 2048, nó sẽ tự tính toán để chiều rộng khiên luôn ở mức 250 pixels
    float maxScale = 250.0f / (float)texKeChiuDonVFX.width; 
    float currentScale = 0.0f;
    float rotSpeed = timer * 150.0f;

    // Giai đoạn 1 + 2: Phình ra và duy trì
    if (timer < 2.5f) {
        if (timer < 0.5f) currentScale = (timer / 0.5f) * maxScale;
        else currentScale = maxScale;
    } 
    // Giai đoạn 3: Hút ngược vào trong
    else if (timer < 3.0f) {
        float shrinkP = (timer - 2.5f) / 0.5f; 
        currentScale = maxScale * (1.0f - shrinkP); // Nhỏ dần về 0
    }

    // Vẽ Tấm Khiên (Nếu chưa đến giây thứ 3)
    if (timer < 3.0f) {
        Rectangle destRec = { center.x, center.y, texKeChiuDonVFX.width * currentScale, texKeChiuDonVFX.height * currentScale };
        Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };
        
        // Phủ lớp màu Xanh Trắng (SKYBLUE) lên ảnh
        DrawTexturePro(texKeChiuDonVFX,
            (Rectangle){ 0, 0, (float)texKeChiuDonVFX.width, (float)texKeChiuDonVFX.height },
            destRec, origin, rotSpeed, Fade(SKYBLUE, 0.9f));

        // Vẽ 5 hạt ánh sáng bay xoắn quanh khiên
        if (timer > 0.3f) {
            for(int i = 0; i < 5; i++) {
                float angle = rotSpeed * 2.0f + (i * 72.0f); // Tỏa đều 360 độ
                float radius = 100.0f * (currentScale / maxScale); // Bán kính vòng xoay thu nhỏ theo khiên
                
                Vector2 particlePos = {
                    center.x + cos(angle * DEG2RAD) * radius,
                    center.y + sin(angle * DEG2RAD) * radius
                };
                DrawCircleV(particlePos, 6.0f, Fade(GREEN, 0.6f)); // Lõi hạt xanh
                DrawCircleV(particlePos, 3.0f, WHITE);             // Viền sáng trắng
            }
        }
    }

    // Giai đoạn 4: Bùng nổ quầng sáng Hồi Máu Xanh Lá
    if (timer >= 3.0f && timer <= 3.7f) {
        float burstP = (timer - 3.0f) / 0.7f; 
        float radius = 50.0f + (burstP * 250.0f); // Quầng sáng nở to từ 50 lên 300px
        float alpha = 1.0f - burstP; // Mờ dần
        
        DrawCircleGradient((int)center.x, (int)center.y, radius, Fade(LIME, alpha), Fade(DARKGREEN, 0.0f));
    }
}
// =========================================================
// 3_ĐẦU GẤU_SKILL CÚ ĐẤM SẤM SÉT (TIMING CHUẨN)
// =========================================================
void DrawSkillVFX_CuDamSamSet(Vector2 pPos, Vector2 ePos, float timer, float jumpOffset, float dashOffset) {
    if (texLightningVFX.id == 0 || texPunchVFX.id == 0 || texExplosionVFX.id == 0) return;

    Vector2 currentPlayerPos = { pPos.x + dashOffset, pPos.y - jumpOffset };
    Vector2 targetPos = { ePos.x, ePos.y };

    // 1. Vẽ nắm đấm tụ lực (1.2s -> 2.0s) và đấm thẳng (2.0s -> 2.4s)
    if (timer >= 1.2f && timer <= 2.4f) {
        float punchScale = 120.0f / (float)texPunchVFX.width;
        Vector2 punchDrawPos = { currentPlayerPos.x + 60.0f, currentPlayerPos.y };
        Color punchColor = WHITE;

        if (timer <= 2.0f) {
            // Tụ lực: Rung lắc nhẹ và to dần
            float p = (timer - 1.2f) / 0.8f;
            punchScale *= (0.5f + p * 0.5f); 
            punchDrawPos.x += GetRandomValue(-4, 4);
            punchDrawPos.y += GetRandomValue(-4, 4);
        } else if (timer <= 2.3f) {
            // Đấm mạnh tới boss (Gia tốc cực nhanh)
            float p = (timer - 2.0f) / 0.3f;
            float easeP = p * p * p; 
            punchDrawPos.x += (targetPos.x - punchDrawPos.x - 50.0f) * easeP;
            punchScale *= 1.2f; 
        } else {
            // Chạm Boss (Dừng lại mờ dần)
            punchDrawPos.x = targetPos.x - 50.0f;
            punchScale *= 1.2f;
            punchColor = Fade(WHITE, 1.0f - ((timer - 2.3f) / 0.1f));
        }

        DrawTexturePro(texPunchVFX,
            (Rectangle){ 0, 0, (float)texPunchVFX.width, (float)texPunchVFX.height },
            (Rectangle){ punchDrawPos.x, punchDrawPos.y, texPunchVFX.width * punchScale, texPunchVFX.height * punchScale },
            (Vector2){ (texPunchVFX.width * punchScale) / 2.0f, (texPunchVFX.height * punchScale) / 2.0f },
            0.0f, punchColor);
    }

    // 2. Tia sét giáng xuống: Cắt ảnh render từ trên xuống (2.4s -> 3.0s)
    if (timer >= 2.4f && timer <= 3.0f) {
        float p = (timer - 2.4f) / 0.6f;
        float lightningWidth = 150.0f;
        float lightningMaxHeight = 700.0f; // Độ cao tia sét

        // Cắt src từ đỉnh xuống dần đáy
        Rectangle srcRec = { 0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height * p };
        // Đỉnh vẽ cố định ở trên cao, kéo dài dần xuống đầu Boss
        float startY = targetPos.y - lightningMaxHeight;
        Rectangle destRec = { targetPos.x, startY, lightningWidth, lightningMaxHeight * p };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texLightningVFX, srcRec, destRec,
            (Vector2){ lightningWidth / 2.0f, 0 }, 0.0f, WHITE);
        EndBlendMode();
    }

    // 3. Vụ nổ Đoàng và tan biến (3.0s -> 3.7s)
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float expScale = (400.0f / (float)texExplosionVFX.width) * (1.0f + p * 1.5f);
        float expAlpha = 1.0f - p;

        DrawTexturePro(texExplosionVFX,
            (Rectangle){ 0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height },
            (Rectangle){ targetPos.x, targetPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale },
            (Vector2){ (texExplosionVFX.width * expScale) / 2.0f, (texExplosionVFX.height * expScale) / 2.0f },
            timer * 50.0f, Fade(WHITE, expAlpha));

        // Phụ họa luồng chớp của Raylib
        DrawCircleGradient(targetPos.x, targetPos.y, 100.0f + p * 300.0f, Fade(YELLOW, expAlpha * 0.9f), Fade(RED, 0.0f));
    }
}
// =========================================================
// 4_ĐẦU GẤU_SKILL RUNG CHẤN (GIẬM ĐẤT TOÀN MÀN HÌNH)
// =========================================================
void DrawSkillVFX_RungChan(Vector2 pPos, float timer, int sw, int sh) {
    if (texLightningVFX.id == 0 || texExplosionVFX.id == 0) return;

    // Vị trí tâm giữa bản đồ nơi nhân vật giậm xuống
   Vector2 centerMap = { (float)sw / 2.0f, (float)sh / 2.0f + 50.0f };      

    // 1. Tia sét giáng xuống ngay lúc chạm đất (từ 2.0s đến 2.5s)
    if (timer >= 2.0f && timer <= 2.5f) {
        float p = (timer - 2.0f) / 0.5f;
        float lightningW = 200.0f;
        // Vẽ tia sét cắm từ trên trời xuống centerMap
        Rectangle lightningDest = { centerMap.x, centerMap.y - 600.0f, lightningW, 600.0f };
        
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texLightningVFX, 
            (Rectangle){ 0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height },
            lightningDest, (Vector2){ lightningW / 2.0f, 0 }, 0.0f, Fade(ORANGE, 1.0f - p));
        EndBlendMode();
    }

    // 2. Vụ nổ khói bụi bùng phát (từ 2.3s đến 3.7s)
    if (timer >= 2.3f && timer <= 3.7f) {
        float p = (timer - 2.3f) / 1.4f;
        // Vụ nổ cực lớn che gần hết màn hình
        float expScale = (800.0f / (float)texExplosionVFX.width) * (1.0f + p * 0.5f); 
        float expAlpha = 1.0f - p;

        DrawTexturePro(texExplosionVFX,
            (Rectangle){ 0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height },
            (Rectangle){ centerMap.x, centerMap.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale },
            (Vector2){ (texExplosionVFX.width * expScale) / 2.0f, (texExplosionVFX.height * expScale) / 2.0f },
            0.0f, Fade(WHITE, expAlpha));
            
        // Thêm hiệu ứng rung màn hình mạnh lúc bắt đầu nổ
        if (timer < 2.5f) {
            // Có thể thêm lệnh rung màn hình ở đây nếu cần
        }
    }
}
// =========================================================
// 5_ĐẦU GẤU_CHIÊU CUỐI: LẤY THỦ BÙ CÔNG
// =========================================================
void DrawSkillVFX_UltiDauGau(Vector2 ePos, float timer, int sw, int sh) {
    if (texUltiDauGau.id == 0 || texExplosionVFX.id == 0 || texLightningVFX.id == 0) return;

    // Tọa độ bắt đầu: Góc trái trên cùng (Lùi ra ngoài màn hình một chút cho đẹp)
    Vector2 startPos = { -200.0f, -200.0f };
    // Tọa độ kết thúc: Vị trí của Boss
    Vector2 endPos = { ePos.x, ePos.y };

    // GIAI ĐOẠN 1: Triệu hồi và bay xuống (0.5s đến 3.0s)
    if (timer >= 0.5f && timer <= 3.0f) {
        float p = (timer - 0.5f) / 2.5f; 
        
        // Easing: Càng gần Boss bay càng nhanh (Gia tốc rơi)
        float easeP = p * p * p; 

        Vector2 currentPos = {
            startPos.x + (endPos.x - startPos.x) * easeP,
            startPos.y + (endPos.y - startPos.y) * easeP
        };

        // Tính toán góc nghiêng để nắm đấm luôn chĩa thẳng vào mặt Boss
        float dx = endPos.x - startPos.x;
        float dy = endPos.y - startPos.y;
        float angle = atan2f(dy, dx) * RAD2DEG;
        
        // Tùy thuộc vào góc xoay gốc của bức ảnh bạn tải lên, nếu nắm đấm bị lệch hướng,
        // bạn có thể thay đổi số 45.0f bên dưới (thử các góc -90, 0, 90, 180 v.v.)
        float rotationOffset = -45.0f; 

        // Kích thước khổng lồ: Ép chiều rộng nắm đấm khoảng 600 pixel
        float scale = 600.0f / (float)texUltiDauGau.width;

        DrawTexturePro(texUltiDauGau,
            (Rectangle){ 0, 0, (float)texUltiDauGau.width, (float)texUltiDauGau.height },
            (Rectangle){ currentPos.x, currentPos.y, texUltiDauGau.width * scale, texUltiDauGau.height * scale },
            (Vector2){ (texUltiDauGau.width * scale) / 2.0f, (texUltiDauGau.height * scale) / 2.0f },
            angle + rotationOffset, WHITE);
    }

    // GIAI ĐOẠN 2: Va chạm! Sấm sét giáng xuống và Khói bụi bùng nổ (3.0s đến 3.7s)
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        
        // 1. Sấm sét (Xẹt xuống chớp nhoáng ở nửa đầu vụ nổ)
        if (p < 0.4f) {
            float lightAlpha = 1.0f - (p / 0.4f);
            float lightningW = 300.0f;
            Rectangle lightningDest = { endPos.x, endPos.y - 800.0f, lightningW, 800.0f };
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texLightningVFX, 
                (Rectangle){ 0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height },
                lightningDest, (Vector2){ lightningW / 2.0f, 0 }, 0.0f, Fade(YELLOW, lightAlpha));
            EndBlendMode();
        }

        // 2. Nổ khói bụi bành trướng
        float expScale = (600.0f / (float)texExplosionVFX.width) * (1.0f + p * 1.0f);
        float expAlpha = 1.0f - p;
        
        DrawTexturePro(texExplosionVFX,
            (Rectangle){ 0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height },
            (Rectangle){ endPos.x, endPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale },
            (Vector2){ (texExplosionVFX.width * expScale) / 2.0f, (texExplosionVFX.height * expScale) / 2.0f },
            GetTime() * 20.0f, Fade(WHITE, expAlpha));
            
        // Thêm luồng sóng chớp nháy (Flash)
        DrawCircleGradient(endPos.x, endPos.y, 200.0f + p * 400.0f, Fade(ORANGE, expAlpha * 0.8f), Fade(BLACK, 0.0f));
    }
}

// =========================================================
// VFX BOSS PHASE 1: Lazer Hủy Diệt (MỚI - Chậm rãi, Ngầu)
// =========================================================
void DrawSkillVFX_LaserHuyDiet(Vector2 bPos, Vector2 pPos, float timer) {
    if (texBlackHoleVFX.id == 0 || texLaserVFX.id == 0) return;
    Vector2 holePos = { bPos.x, bPos.y - 150.0f };

    // GIAI ĐOẠN 1 (0.0s -> 1.5s): Tụ hố đen từ từ, vòng sáng tỏa ra uy lực
    if (timer < 1.5f) {
        float p = timer / 1.5f;
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){holePos.x, holePos.y, texBlackHoleVFX.width * p, texBlackHoleVFX.height * p},
                       (Vector2){(texBlackHoleVFX.width * p)/2.0f, (texBlackHoleVFX.height * p)/2.0f}, timer * 100.0f, Fade(WHITE, p));
        DrawCircleGradient(holePos.x, holePos.y, 100.0f * p, Fade(VIOLET, p * 0.5f), BLANK); // Hào quang tụ lực
    } 
    // GIAI ĐOẠN 2 (1.5s -> 3.0s): Bắn Lazer duy trì cường độ cao, mạch đập
    else if (timer >= 1.5f && timer < 3.0f) {
        float p = (timer - 1.5f) / 1.5f;
        
        // Rung nhẹ hố đen khi đang xả năng lượng
        Vector2 shakeHole = { holePos.x + GetRandomValue(-2, 2), holePos.y + GetRandomValue(-2, 2) };
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){shakeHole.x, shakeHole.y, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Vector2){(float)texBlackHoleVFX.width/2.0f, (float)texBlackHoleVFX.height/2.0f}, timer * 300.0f, WHITE);

        float angle = atan2f(pPos.y - shakeHole.y, pPos.x - shakeHole.x) * RAD2DEG;
        float dist = Vector2Distance(shakeHole, pPos);
        
        // Độ dày tia laser chớp nháy (mạch đập) chứ không nhỏ dần nữa
        float laserWidth = 25.0f + sinf(timer * 25.0f) * 10.0f; 
        
        Rectangle sourceRec = { 0, 0, (float)texLaserVFX.width, (float)texLaserVFX.height };
        Rectangle destRec = { shakeHole.x, shakeHole.y, dist, laserWidth }; 
        Vector2 origin = { 0.0f, laserWidth / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texLaserVFX, sourceRec, destRec, origin, angle, WHITE); 
        // Lõi sáng laser trắng tinh
        DrawLineEx(shakeHole, pPos, laserWidth * 0.4f, Fade(WHITE, 0.8f + sinf(timer*40)*0.2f)); 
        EndBlendMode();
    }
    // GIAI ĐOẠN 3 (3.0s -> 3.7s): Laser tắt, Nổ tung tại vị trí người chơi
    if (timer >= 3.0f && timer < 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        if (texExplosionVFX.id != 0) {
            float expScale = (200.0f / (float)texExplosionVFX.width) * (1.0f + p * 2.0f); // Nổ bành trướng
            DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                           (Rectangle){pPos.x, pPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                           (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f}, timer * 50.0f, Fade(WHITE, 1.0f - p));
        }
    }
}

// =========================================================
// VFX BOSS PHASE 1: Dịch Chuyển Thời Không (MỚI)
// =========================================================
void DrawSkillVFX_DichChuyen(Vector2 pPos, float timer) {
    if (texTelegateVFX.id == 0 || texExplosionVFX.id == 0) return;

    // GIAI ĐOẠN 1 (0.0s -> 2.5s): Vòng ma thuật hiện lên dưới chân, xoay tít hút năng lượng
    if (timer < 2.5f) {
        float p = timer / 2.5f;
        float scale = 0.5f + (p * 1.5f); // Phình to cực kỳ từ từ
        float rotSpeed = timer * 150.0f;
        DrawTexturePro(texTelegateVFX, (Rectangle){0, 0, (float)texTelegateVFX.width, (float)texTelegateVFX.height},
                       (Rectangle){pPos.x, pPos.y, texTelegateVFX.width * scale, texTelegateVFX.height * scale},
                       (Vector2){(texTelegateVFX.width * scale)/2.0f, (texTelegateVFX.height * scale)/2.0f}, rotSpeed, Fade(WHITE, p));
        
        // Hút sáng vào tâm
        if (timer > 1.0f) {
            DrawCircleGradient(pPos.x, pPos.y, 150.0f * p, Fade(VIOLET, p * 0.5f), BLANK);
        }
    }
    // GIAI ĐOẠN 2 (2.5s -> 3.0s): Nén không gian (Vòng tròn đột ngột thu nhỏ lại)
    else if (timer >= 2.5f && timer < 3.0f) {
        float p = (timer - 2.5f) / 0.5f;
        float scale = 2.0f * (1.0f - p); // Ép nhỏ lại thật nhanh
        DrawTexturePro(texTelegateVFX, (Rectangle){0, 0, (float)texTelegateVFX.width, (float)texTelegateVFX.height},
                       (Rectangle){pPos.x, pPos.y, texTelegateVFX.width * scale, texTelegateVFX.height * scale},
                       (Vector2){(texTelegateVFX.width * scale)/2.0f, (texTelegateVFX.height * scale)/2.0f}, timer * 500.0f, WHITE);
        
        DrawCircleGradient(pPos.x, pPos.y, 200.0f * (1.0f - p), Fade(WHITE, p), BLANK); // Sáng lóe lên trước khi nổ
    }
    // GIAI ĐOẠN 3 (3.0s -> 3.7s): Bùng nổ ánh sáng trắng tím
    if (timer >= 3.0f && timer < 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float expScale = p * 6.0f;
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                       (Rectangle){pPos.x, pPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                       (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f}, timer * 50.0f, Fade(VIOLET, 1.0f - p));
        DrawCircleGradient(pPos.x, pPos.y, 100.0f + p * 500.0f, Fade(WHITE, (1.0f - p) * 0.8f), BLANK); // Sóng xung kích
        EndBlendMode();
    }
}

// =========================================================
// VFX BOSS PHASE 1_CHIÊU CUỐI: Pháo Hố Đen (MỚI)
// =========================================================
void DrawSkillVFX_PhaoHoDen(Vector2 pPos, float timer) {
    if (texBlackHoleVFX.id == 0 || texLightningVFX.id == 0 || texExplosionVFX.id == 0) return; 

    // GIAI ĐOẠN 1 (0.0s -> 2.0s): Hố đen khổng lồ nuốt chửng khu vực
    if (timer < 2.0f) {
        float p = timer / 2.0f;
        float scale = p * 2.0f; // To gấp đôi
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){pPos.x, pPos.y, texBlackHoleVFX.width * scale, texBlackHoleVFX.height * scale},
                       (Vector2){(texBlackHoleVFX.width * scale)/2.0f, (texBlackHoleVFX.height * scale)/2.0f}, timer * 100.0f, Fade(WHITE, p)); 
        DrawCircleGradient(pPos.x, pPos.y, 150.0f * p, Fade(BLACK, p * 0.8f), BLANK); // Phủ màn đêm
    } 
    // GIAI ĐOẠN 2 (2.0s -> 3.0s): 4 tia laser ngắm chuẩn dần + Sấm sét giáng xuống
    else if (timer >= 2.0f && timer < 3.0f) {
        float p = (timer - 2.0f) / 1.0f;
        // Giữ hố đen khổng lồ xoay điên cuồng
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){pPos.x, pPos.y, texBlackHoleVFX.width * 2.0f, texBlackHoleVFX.height * 2.0f},
                       (Vector2){(float)texBlackHoleVFX.width * 2.0f/2.0f, (float)texBlackHoleVFX.height * 2.0f/2.0f}, timer * 300.0f, WHITE); 
        
        Vector2 pts[4] = {{pPos.x, pPos.y - 800}, {pPos.x, pPos.y + 800}, {pPos.x - 800, pPos.y}, {pPos.x + 800, pPos.y}};
        BeginBlendMode(BLEND_ADDITIVE);
        for(int i = 0; i < 4; i++) {
            float lWidth = p * 15.0f; // Tia sáng to dần từ các phía tụ vào
            DrawLineEx(pts[i], pPos, lWidth + 5.0f, Fade(VIOLET, p));
            DrawLineEx(pts[i], pPos, lWidth, Fade(WHITE, p));
        }
        
        // Sấm sét giật dữ dội khi sắp nổ
        if (p > 0.3f) {
            DrawTexturePro(texLightningVFX, (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                           (Rectangle){pPos.x, pPos.y - 600.0f, 400.0f, 800.0f}, (Vector2){200.0f, 0}, (float)GetRandomValue(-30, 30), Fade(WHITE, p));
        }
        EndBlendMode();
    }
    // GIAI ĐOẠN 3 (3.0s -> 3.7s): Tận thế, vụ nổ kinh hoàng thổi bay tất cả
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        
        // Hố đen tan biến
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){pPos.x, pPos.y, texBlackHoleVFX.width * 2.0f, texBlackHoleVFX.height * 2.0f},
                       (Vector2){(float)texBlackHoleVFX.width * 2.0f/2.0f, (float)texBlackHoleVFX.height * 2.0f/2.0f}, timer * 300.0f, Fade(WHITE, 1.0f - p)); 
        
        // Nổ siêu tân tinh
        float expScale = p * 12.0f; 
        DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                       (Rectangle){pPos.x, pPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                       (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f}, GetTime()*50.0f, Fade(WHITE, 1.0f - p));
        
        // Flash trắng lóa màn hình
        DrawCircleGradient(pPos.x, pPos.y, 400.0f + p * 1000.0f, Fade(WHITE, 1.0f - p), BLANK);
    }
}

void DrawSkillVFX_DotKichPhanRa(Vector2 ePos, Vector2 pPos, float timer) {
    if (texVoidPunch.id == 0) return;

    // Tâm điểm đấm là ngay giữa người Player
    Vector2 targetPos = { pPos.x + 30.0f, pPos.y - 60.0f };

    // ================================================================
    // Phase 1: Nắm đấm xuất hiện sau lưng và tụ lực (1.0s -> 3.0s)
    // ================================================================
    if (timer >= 1.0f && timer <= 3.0f) {
        float p = (timer - 1.0f) / 2.0f; // 0.0 -> 1.0
        
        // Vị trí: Tụ lực sau lưng boss một chút (dựa vào ePos động)
        // Nó sẽ dần di chuyển ra phía trước khi p -> 1.0
        Vector2 chargePos = { ePos.x - 50.0f * (1.0f - p), ePos.y - 50.0f * (1.0f - p) };
        
        // Kích thước bự dần (từ 0.4 lên 1.3)
        float punchScale = (350.0f / (float)texVoidPunch.width) * (0.4f + p * 0.9f);
        
        // Góc đấm chếch nhẹ vào người chơi, rung lắc dữ dội khi p -> 1.0
        float rot = -45.0f + GetRandomValue(-3, 3) * p; 
        
        // Rung lắc dữ dội ngay trước khi chạm mục tiêu (2.5s -> 3.0s)
        if (timer > 2.5f) {
            chargePos.x += GetRandomValue(-8, 8);
            chargePos.y += GetRandomValue(-8, 8);
        }

        DrawTexturePro(texVoidPunch, 
            (Rectangle){0, 0, (float)texVoidPunch.width, (float)texVoidPunch.height},
            (Rectangle){chargePos.x, chargePos.y, texVoidPunch.width * punchScale, texVoidPunch.height * punchScale},
            (Vector2){(texVoidPunch.width * punchScale)/2.0f, (texVoidPunch.height * punchScale)/2.0f},
            rot, WHITE);
    }

    // ================================================================
    // Phase 2: Nắm đấm bay tới và ĐẤM TRÚNG (Ngay tại giây 3.0)
    //         Phase này siêu ngắn, ta chỉ vẽ nắm đấm ngay tại target
    // ================================================================
    if (timer >= 3.0f && timer < 3.2f) {
        float p = (timer - 3.0f) / 0.2f; // 0.0 -> 1.0
        float punchScale = (350.0f / (float)texVoidPunch.width) * 1.3f;
        
        // Nắm đấm rung chuyển cực mạnh tại vị trí player và mờ dần
        Vector2 finalPunchPos = { targetPos.x + GetRandomValue(-15, 15), targetPos.y + GetRandomValue(-15, 15) };
        
        DrawTexturePro(texVoidPunch, 
            (Rectangle){0, 0, (float)texVoidPunch.width, (float)texVoidPunch.height},
            (Rectangle){finalPunchPos.x, finalPunchPos.y, texVoidPunch.width * punchScale, texVoidPunch.height * punchScale},
            (Vector2){(texVoidPunch.width * punchScale)/2.0f, (texVoidPunch.height * punchScale)/2.0f},
            -45.0f, Fade(WHITE, 1.0f - p));
    }

    // ================================================================
    // Phase 3: Hố Đen & Vết Nứt BÙM (3.0s -> 3.7s)
    // ================================================================
    if (timer >= 3.0f && timer <= 3.7f) {
        // a. Vẽ Vết nứt không gian (Bùng nổ nhanh, mờ dần)
        float crackP = 1.0f - (timer - 3.0f)/0.7f; // 1.0 -> 0.0
        for(int i = 0; i < 7; i++) {
            float angle = (i * (360.0f / 7) + GetRandomValue(-20, 20)) * DEG2RAD;
            float length = 200.0f * (1.0f + sinf((timer - 3.0f)*5.0f)*0.3f) + GetRandomValue(50, 100);
            
            Vector2 endCrack = { targetPos.x + cosf(angle)*length, targetPos.y + sinf(angle)*length };
            // Điểm ngoằn ngoèo ở giữa
            Vector2 midCrack = { targetPos.x + cosf(angle)*length*0.5f + GetRandomValue(-40,40), 
                                 targetPos.y + sinf(angle)*length*0.5f + GetRandomValue(-40,40) };
            
            DrawLineEx(targetPos, midCrack, 15.0f * crackP, Fade(BLACK, crackP));
            DrawLineEx(midCrack, endCrack, 8.0f * crackP, Fade(PURPLE, crackP));
        }

        // b. Hiệu ứng phân rã (Pixel bay toán loạn)
        for (int j = 0; j < 25; j++) {
            float pTime = timer - 3.0f; // 0.0 -> 0.7
            Vector2 particle = {
                targetPos.x + GetRandomValue(-350, 350) * pTime,
                targetPos.y + GetRandomValue(-350, 350) * pTime
            };
            float size = GetRandomValue(5, 12) * (1.0f - pTime/0.7f);
            Color partColor = (GetRandomValue(0, 1) == 0) ? PURPLE : MAGENTA;
            DrawRectangleV(particle, (Vector2){size, size}, Fade(partColor, crackP));
        }

        // c. Vẽ Hố Đen (texBlackHole): Nở ra cực nhanh và thu lại cực nhanh
        if (texBlackHoleVFX.id != 0) {
            float bhP = 0.0f;
            if (timer < 3.2f) {
                // GIAI ĐOẠN 1 (3.0s -> 3.2s): Nở ra cực nhanh
                bhP = (timer - 3.0f) / 0.2f; // 0.0 -> 1.0
            } else if (timer < 3.5f) {
                // GIAI ĐOẠN 2 (3.2s -> 3.5s): Thu lại cực nhanh
                bhP = 1.0f - (timer - 3.2f) / 0.3f; // 1.0 -> 0.0
            }
            // Mờ dần ở giây 3.5 -> 3.7
            float bhAlpha = (timer < 3.5f) ? bhP : bhP * (1.0f - (timer - 3.5f)/0.2f);

            float bhScale = bhP * 1.8f;
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texBlackHoleVFX, 
                (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                (Rectangle){targetPos.x, targetPos.y, texBlackHoleVFX.width * bhScale, texBlackHoleVFX.height * bhScale},
                (Vector2){(texBlackHoleVFX.width * bhScale)/2.0f, (texBlackHoleVFX.height * bhScale)/2.0f},
                timer * -800.0f, Fade(WHITE, bhAlpha));
            EndBlendMode();
        }
    }
}

void DrawSkillVFX_NghichLyHuyDiet(float timer, int sw, int sh) {
    if (texBlackHoleVFX.id == 0 || texTelegateVFX.id == 0 || texLightningVFX.id == 0) return;

    // Tâm điểm của chiêu cuối là ngay giữa màn hình
    Vector2 center = { (float)sw / 2.0f, (float)sh / 2.0f };

    // ================================================================
    // GIAI ĐOẠN 1: Triệu hồi Cổng Không Gian (0.0s -> 3.0s)
    // ================================================================
    if (timer < 3.0f) {
        float p = timer / 3.0f; // 0.0 -> 1.0
        float scale = 2.0f + p * 3.0f; // Vòng tròn nở ra cực to, bao trùm cả 2 nhân vật
        float rotSpeed = timer * 100.0f * (1.0f + p * 2.0f); // Xoay càng lúc càng gắt
        
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texTelegateVFX, 
            (Rectangle){0, 0, (float)texTelegateVFX.width, (float)texTelegateVFX.height},
            (Rectangle){center.x, center.y, texTelegateVFX.width * scale, texTelegateVFX.height * scale},
            (Vector2){(texTelegateVFX.width * scale)/2.0f, (texTelegateVFX.height * scale)/2.0f},
            rotSpeed, Fade(MAGENTA, p * 0.8f)); // Ánh sáng tím bao trùm
        EndBlendMode();
    }

    // ================================================================
    // GIAI ĐOẠN 2: Lỗ Sâu Bộc Phát & Cuồng Phong Sấm Sét (1.5s -> 3.0s)
    // ================================================================
    if (timer >= 1.5f && timer < 3.0f) {
        float p = (timer - 1.5f) / 1.5f; // 0.0 -> 1.0
        
        // 1. Hố đen xuất hiện từ tâm và phình to
        float bhScale = p * 4.5f; 
        DrawTexturePro(texBlackHoleVFX, 
            (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
            (Rectangle){center.x, center.y, texBlackHoleVFX.width * bhScale, texBlackHoleVFX.height * bhScale},
            (Vector2){(texBlackHoleVFX.width * bhScale)/2.0f, (texBlackHoleVFX.height * bhScale)/2.0f},
            -timer * 300.0f, Fade(WHITE, p));
            
        // 2. Mưa sấm sét đánh ngẫu nhiên toàn bản đồ
        if (GetRandomValue(1, 100) < 30 + p * 60) { // Sét đánh càng lúc càng dày đặc
            float lx = center.x + GetRandomValue(-sw/2, sw/2);
            float ly = center.y - 600.0f;
            float lw = GetRandomValue(150, 350);
            
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texLightningVFX,
                (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                (Rectangle){lx, ly, lw, 800.0f},
                (Vector2){lw / 2.0f, 0}, (float)GetRandomValue(-25, 25), Fade(PURPLE, p));
            EndBlendMode();
        }
    }

    // ================================================================
    // GIAI ĐOẠN 3: VỤ NỔ ĐA CHIỀU & DƯ ÂM HỦY DIỆT (3.0s -> 3.7s)
    // ================================================================
    if (timer >= 3.0f && timer <= 3.7f) {
        float fadeOut = 1.0f - (timer - 3.0f) / 0.7f; // 1.0 -> 0.0
        
        // 1. Flash trắng lóa màn hình ngay mốc 3.0s
        if (timer < 3.15f) {
            float flashP = 1.0f - (timer - 3.0f) / 0.15f;
            DrawRectangle(0, 0, sw, sh, Fade(WHITE, flashP * 0.9f));
        }

        // 2. Vẽ Vết Nứt Không Gian Khổng Lồ
        // Vì gọi GetRandomValue mỗi frame sẽ làm vết nứt giật liên tục, tạo cảm giác không gian hỗn loạn (glitch) cực kỳ "điên" đúng chất Boss
        for(int i = 0; i < 15; i++) {
            float angle = (i * (360.0f / 15) + GetRandomValue(-10, 10)) * DEG2RAD;
            float length = 400.0f + GetRandomValue(100, 400);
            
            Vector2 endCrack = { center.x + cosf(angle)*length, center.y + sinf(angle)*length };
            Vector2 midCrack = { center.x + cosf(angle)*length*0.5f + GetRandomValue(-100, 100), 
                                 center.y + sinf(angle)*length*0.5f + GetRandomValue(-100, 100) };
            
            DrawLineEx(center, midCrack, 35.0f * fadeOut, Fade(BLACK, fadeOut));
            DrawLineEx(midCrack, endCrack, 15.0f * fadeOut, Fade(MAGENTA, fadeOut));
            DrawLineEx(midCrack, endCrack, 5.0f * fadeOut, Fade(WHITE, fadeOut)); // Lõi sáng
        }

        // 3. Hạt pixel phân rã bay tán loạn
        for (int j = 0; j < 80; j++) {
            Vector2 particle = {
                center.x + GetRandomValue(-sw/2, sw/2) * (1.5f - fadeOut),
                center.y + GetRandomValue(-sh/2, sh/2) * (1.5f - fadeOut)
            };
            float size = GetRandomValue(15, 35) * fadeOut;
            Color partColor = (GetRandomValue(0, 1) == 0) ? PURPLE : MAGENTA;
            DrawRectangleV(particle, (Vector2){size, size}, Fade(partColor, fadeOut));
        }
        
        // 4. Mưa sét tàn dư cháy nổ vớt vát
        if (GetRandomValue(1, 100) < 60) {
            float lx = center.x + GetRandomValue(-sw/2, sw/2);
            float ly = center.y - 600.0f;
            float lw = GetRandomValue(200, 400);
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texLightningVFX,
                (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                (Rectangle){lx, ly, lw, 800.0f},
                (Vector2){lw / 2.0f, 0}, (float)GetRandomValue(-35, 35), Fade(WHITE, fadeOut));
            EndBlendMode();
        }
    }
}

void DrawReviveVFX(Vector2 pPos, float timer) {
    if (timer <= 0.0f) return; // Không vẽ nếu timer bằng 0

    // Hiệu ứng kéo dài trong 2.0 giây
    if (timer > 2.0f) {
        reviveVFXTimer = 0.0f; // Tự động reset timer khi hết thời gian
        return; 
    }

    Vector2 targetPos = { pPos.x + 30.0f, pPos.y - 100.0f }; // Tâm hiệu ứng ở ngực người chơi
    float alpha = (timer < 1.0f) ? timer : 1.0f - (timer - 1.0f); // Hiện dần rồi mờ dần

    BeginBlendMode(BLEND_ADDITIVE);

    // ========================================================
    // 1. VẼ AURA TOẢ SÁNG TRẮNG-VÀNG
    // ========================================================
    if (texPhase2Aura.id != 0) { // Tái sử dụng ảnh Aura của Boss để tiết kiệm tài nguyên
        float baseScale = 450.0f / 1024.0f; // Scale cơ bản cho vừa người
        float scale = baseScale * (1.2f + sinf(timer * 6.0f) * 0.1f); // Nhấp nháy to nhỏ nhẹ
        
        // Toán học Sin để nhấp nháy màu giữa Trắng và Vàng
        float colorPulse = (sinf(timer * 8.0f) + 1.0f) / 2.0f; // 0.0 (Trắng) -> 1.0 (Vàng)
        Color tint = (Color){255, 255, (unsigned char)(255 * colorPulse), 255}; // Chỉnh kênh Blue để tạo màu Vàng
        
        Rectangle dest = { targetPos.x, targetPos.y, 1024.0f * scale, 1024.0f * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        // Lớp nền trắng
        DrawTexturePro(texPhase2Aura, (Rectangle){0, 0, 1024, 1024}, dest, origin, timer * 40.0f, Fade(WHITE, alpha * 0.6f));
        // Lớp màu vàng nhấp nháy
        DrawTexturePro(texPhase2Aura, (Rectangle){0, 0, 1024, 1024}, dest, origin, timer * -60.0f, Fade(tint, alpha));
    }

    // ========================================================
    // 2. VẼ HẠT HỒI SINH BAY NGƯỢC LÊN (Trắng + Vàng)
    // ========================================================
    for (int j = 0; j < 30; j++) {
        // Mỗi hạt có một thời gian bắt đầu khác nhau dựa trên timer tổng
        float pTime = fmodf(timer + j * 0.1f, 1.0f); // pTime: 0.0 -> 1.0 cho mỗi hạt
        
        // Hạt bay từ dưới chân lên đầu
        Vector2 particle = {
            pPos.x + GetRandomValue(-100, 100) * (1.0f - pTime * 0.3f), // Co vào giữa khi bay lên
            pPos.y + 150.0f - pTime * 350.0f // Bay lên 350px
        };
        
        float size = (float)GetRandomValue(5, 12) * (1.0f - pTime); // Nhỏ dần
        Color partColor = (GetRandomValue(0, 1) == 0) ? WHITE : GOLD;
        DrawRectangleV(particle, (Vector2){size, size}, Fade(partColor, alpha * (1.0f - pTime))); // Mờ dần
    }
    
    EndBlendMode();
}

void DrawSkillVFX_PhanUngHoaHoc(Vector2 pPos, Vector2 ePos, float timer, int sw, int sh) {
    if (texIconPoison.id == 0 || texPhanUngHoaHoc.id == 0) return;

    Vector2 startPos = { pPos.x + 5.0f, pPos.y - 5.0f }; // Vị trí trên đầu Học Bá
    Vector2 targetPos = { ePos.x, ePos.y - 50.0f };         // Điểm rơi giữa ngực Boss

    // ================================================================
    // GIAI ĐOẠN 1 & 2: Tụ lực và Ném Parabol (0.0s -> 3.0s)
    // ================================================================
    if (timer < 3.0f) {
        // [CÔNG THỨC ÉP SIZE] Đưa ảnh 1024px về cỡ chuẩn 50px
        float baseScale = 100.0f / (float)texIconPoison.width; 

        // 1. Tụ lực (0.0s -> 1.5s): Vòng sáng xanh lá và lọ thuốc to dần
        if (timer < 1.5f) {
            float p = timer / 1.5f;
            DrawCircleGradient(startPos.x, startPos.y, 80.0f * p, Fade(LIME, p * 0.6f), BLANK);
            
            float scale = baseScale * (p * 1.2f); // Lọ thuốc to dần lên cỡ 60px
            DrawTexturePro(texIconPoison, (Rectangle){0, 0, (float)texIconPoison.width, (float)texIconPoison.height},
                           (Rectangle){startPos.x, startPos.y, texIconPoison.width * scale, texIconPoison.height * scale},
                           (Vector2){(texIconPoison.width * scale)/2.0f, (texIconPoison.height * scale)/2.0f},
                           timer * 150.0f, Fade(WHITE, p));
        } 
        // 2. Ném bay vòng cung (1.5s -> 3.0s)
        else {
            float p = (timer - 1.5f) / 1.5f; // 0.0 -> 1.0
            
            // Tính toán quỹ đạo Parabol (trục Y cong vút lên rồi rơi xuống)
            Vector2 currentPos;
            currentPos.x = startPos.x + (targetPos.x - startPos.x) * p;
            currentPos.y = startPos.y + (targetPos.y - startPos.y) * p - sinf(p * PI) * 200.0f; // Cao độ parabol 200px
            
            // Vẽ vệt đuôi khói xanh (Trail)
            DrawCircleGradient(currentPos.x, currentPos.y, 30.0f, Fade(LIME, 0.5f), BLANK); // Giảm size vệt khói cho vừa chai thuốc
            
            // Vẽ lọ thuốc xoay tít thò lò với kích thước cố định
            float currentScale = baseScale * 1.2f; 
            float rot = timer * 800.0f;
            DrawTexturePro(texIconPoison, (Rectangle){0, 0, (float)texIconPoison.width, (float)texIconPoison.height},
                           (Rectangle){currentPos.x, currentPos.y, texIconPoison.width * currentScale, texIconPoison.height * currentScale},
                           (Vector2){(texIconPoison.width * currentScale)/2.0f, (texIconPoison.height * currentScale)/2.0f},
                           rot, WHITE);
        }
    }

    // ================================================================
    // GIAI ĐOẠN 3 & 4: Va Chạm, Khói Độc Bùng Nổ (3.0s -> 3.7s)
    // ================================================================
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        // 1. Flash màn hình chớp xanh lục ở đúng giây 3.0 (Kéo dài 0.15s)
        if (timer < 3.15f) {
            float flashP = 1.0f - (timer - 3.0f) / 0.15f;
            DrawRectangle(0, 0, sw, sh, Fade(LIME, flashP * 0.4f));
        }

        BeginBlendMode(BLEND_ADDITIVE);
        
        // 2. Bùng nổ ảnh khói phan_ung_hoa_hoc.png
        float smokeScale = (350.0f / (float)texPhanUngHoaHoc.width) * (1.0f + p * 1.5f); // Bành trướng to ra
        DrawTexturePro(texPhanUngHoaHoc, (Rectangle){0, 0, (float)texPhanUngHoaHoc.width, (float)texPhanUngHoaHoc.height},
                       (Rectangle){targetPos.x, targetPos.y, texPhanUngHoaHoc.width * smokeScale, texPhanUngHoaHoc.height * smokeScale},
                       (Vector2){(texPhanUngHoaHoc.width * smokeScale)/2.0f, (texPhanUngHoaHoc.height * smokeScale)/2.0f},
                       timer * -40.0f, Fade(LIME, fadeOut * 0.8f)); // Xoay chậm và mờ dần

        // 3. Khói hạt (Particles) xịt ra từ tâm vụ nổ và bay lên trên
        for (int i = 0; i < 50; i++) {
            float pTime = timer - 3.0f;
            Vector2 particle = {
                targetPos.x + GetRandomValue(-250, 250) * pTime,          // Tỏa ra 2 bên
                targetPos.y + GetRandomValue(-200, 200) * pTime - pTime * 150.0f // Bốc lên trên cao
            };
            float size = GetRandomValue(15, 35) * fadeOut;
            Color partColor = (GetRandomValue(0, 2) == 0) ? GREEN : ((GetRandomValue(0, 1) == 0) ? LIME : SKYBLUE); // Mix 3 màu khói
            DrawRectangleV(particle, (Vector2){size, size}, Fade(partColor, fadeOut));
        }

        EndBlendMode();
    }
}

// =========================================================
// [GLOBAL BUFF] HIỆU ỨNG TĂNG GIÁP VÀ TĂNG CÔNG
// =========================================================
void DrawBuffVFX_DefUp(Vector2 pos, float scaleMult) {
    if (texKhienAnimation.id == 0) return;
    float baseScale = (450.0f / (float)texKhienAnimation.width) * scaleMult;
    float rot = GetTime() * 120.0f; // Xoay vòng liên tục
    Rectangle dest = { pos.x, pos.y, texKhienAnimation.width * baseScale, texKhienAnimation.height * baseScale };
    Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
    
    BeginBlendMode(BLEND_ADDITIVE);
    // Vòng ngoài màu Xanh Lam quay xuôi
    DrawTexturePro(texKhienAnimation, (Rectangle){0, 0, (float)texKhienAnimation.width, (float)texKhienAnimation.height},
                   dest, origin, rot, Fade(SKYBLUE, 0.7f));
    // Vòng trong màu Trắng quay ngược
    DrawTexturePro(texKhienAnimation, (Rectangle){0, 0, (float)texKhienAnimation.width, (float)texKhienAnimation.height},
                   dest, origin, -rot * 0.7f, Fade(WHITE, 0.5f));
    EndBlendMode();
}

void DrawBuffVFX_AtkUp(Vector2 pos, float scaleMult) {
    if (texTangCong.id == 0) return;
    
    // Ép size gốc to hơn một chút cho hoành tráng
    float baseScale = (280.0f / (float)texTangCong.width) * scaleMult;
    
    // Tâm bốc lửa nằm ở dưới chân
    Rectangle dest = { pos.x, pos.y + 60.0f * scaleMult, texTangCong.width * baseScale, texTangCong.height * baseScale };
    Vector2 origin = { dest.width / 2.0f, dest.height * 0.9f }; 
    
    BeginBlendMode(BLEND_ADDITIVE);
    
    // VẼ 3 LỚP LỬA CHÁY CUỘN LÊN CAO (Hết nhún nhảy)
    for (int i = 0; i < 3; i++) {
        // p đi từ 0.0 đến 1.0 (chu kỳ bốc lên của mỗi lớp lửa, lệch pha nhau)
        float p = fmodf(GetTime() * 1.5f + (i * 0.33f), 1.0f); 
        
        // Lửa bốc lên cao thì mờ dần và ngọn lửa thu hẹp lại
        float currentAlpha = 1.0f - p; 
        float currentScaleX = dest.width * (1.0f - p * 0.4f); 
        float currentScaleY = dest.height * (1.0f + p * 0.6f); // Bốc vút lên cao
        
        // Lửa lắt lay sang 2 bên nhè nhẹ do gió
        float swayX = sinf(GetTime() * 5.0f + i) * 15.0f;
        
        Rectangle layerDest = { dest.x + swayX, dest.y, currentScaleX, currentScaleY };
        Vector2 layerOrigin = { currentScaleX / 2.0f, currentScaleY * 0.9f };
        
        // Trộn màu: Lớp gốc màu Cam Đỏ, lớp trên cùng màu Vàng Sáng
        Color fireColor = (i == 0) ? ORANGE : (i == 1 ? GOLD : YELLOW);
        
        DrawTexturePro(texTangCong, (Rectangle){0, 0, (float)texTangCong.width, (float)texTangCong.height},
                       layerDest, layerOrigin, 0.0f, Fade(fireColor, currentAlpha * 0.8f));
    }
    EndBlendMode();
}

// =========================================================
// [ACTION VFX] TẬP TRUNG CAO ĐỘ (HỌC BÁ)
// =========================================================
void DrawSkillVFX_TapTrungCaoDo(Vector2 pPos, float timer) {
    if (texIconTapTrung.id == 0) return;
    Vector2 center = { pPos.x + 30.0f, pPos.y - 60.0f }; // Tâm ở ngực nhân vật
    
    if (timer < 3.0f) {
        float p = timer / 3.0f; // 0.0 -> 1.0
        
        // 1. Vòng ma thuật dưới chân (Vàng kim)
        DrawEllipse(pPos.x + 30.0f, pPos.y + 60.0f, 60.0f + p * 30.0f, 20.0f + p * 10.0f, Fade(GOLD, p * 0.5f));
        DrawEllipseLines(pPos.x + 30.0f, pPos.y + 60.0f, 60.0f + p * 30.0f, 20.0f + p * 10.0f, Fade(YELLOW, p));

        // 2. Bóng bộ não lơ lửng phía sau
        float iconScale = (180.0f / (float)texIconTapTrung.width) * (1.0f + p * 0.3f); // Phóng to dần
        float shakeX = (p > 0.6f) ? GetRandomValue(-3, 3) : 0; // Rung lắc khi não bộ quá tải
        float shakeY = (p > 0.6f) ? GetRandomValue(-3, 3) : 0;
        
        Rectangle dest = { center.x + shakeX, center.y - 60.0f + shakeY, texIconTapTrung.width * iconScale, texIconTapTrung.height * iconScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texIconTapTrung, (Rectangle){0, 0, (float)texIconTapTrung.width, (float)texIconTapTrung.height},
                       dest, origin, 0.0f, Fade(WHITE, p));
        
        // 3. Luồng tia sáng (Tri thức) bay tụ vào đầu nhân vật
        int numRays = (int)(p * 25.0f); 
        for(int i = 0; i < numRays; i++) {
            float angle = (GetTime() * 100.0f + i * 45.0f) * DEG2RAD;
            float dist = 250.0f * (1.0f - fmodf(timer * 2.5f + i * 0.1f, 1.0f)); // Bắn từ ngoài vào trong tâm
            Vector2 rayStart = { center.x + cosf(angle) * dist, center.y - 60.0f + sinf(angle) * dist };
            DrawLineEx(rayStart, (Vector2){center.x, center.y - 60.0f}, 3.0f, Fade(GOLD, 0.7f));
        }
        EndBlendMode();
    } 
    // Giai đoạn 3: Bùng nổ ở giây 3.0
    else if (timer >= 3.0f && timer <= 3.4f) {
        float p = (timer - 3.0f) / 0.4f;
        float iconScale = (180.0f / (float)texIconTapTrung.width) * (1.3f + p * 1.5f);
        
        Rectangle dest = { center.x, center.y - 60.0f, texIconTapTrung.width * iconScale, texIconTapTrung.height * iconScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texIconTapTrung, (Rectangle){0, 0, (float)texIconTapTrung.width, (float)texIconTapTrung.height},
                       dest, origin, 0.0f, Fade(WHITE, 1.0f - p)); // Não bộ mờ đi
        // Sóng xung kích tri thức
        DrawCircleGradient(center.x, center.y - 60.0f, 100.0f + p * 300.0f, Fade(YELLOW, 1.0f - p), BLANK);
        EndBlendMode();
    }
}

// =========================================================
// [GLOBAL DEBUFF] HIỆU ỨNG CHOÁNG (Vẽ bằng Raylib thuần)
// =========================================================
void DrawBuffVFX_Stun(Vector2 pos) {
    float timer = GetTime();
    Vector2 headPos = { pos.x, pos.y - 120.0f }; // Vị trí lơ lửng trên đầu
    
    int numStars = 3;
    float radius = 40.0f; // Bán kính quay
    
    // Vẽ các ngôi sao quay vòng tròn méo (Elliptical) quanh đầu
    for (int i = 0; i < numStars; i++) {
        float angle = (timer * 4.0f + (i * 2.0f * PI / numStars)); // Tốc độ quay
        Vector2 starPos = { headPos.x + cosf(angle) * radius, headPos.y + sinf(angle) * 15.0f }; 
        
        BeginBlendMode(BLEND_ADDITIVE);
        // Lõi sao vàng sáng, viền tỏa sáng mờ
        DrawCircleGradient(starPos.x, starPos.y, 15.0f, Fade(YELLOW, 0.8f), BLANK);
        DrawCircle(starPos.x, starPos.y, 4.0f, WHITE);
        EndBlendMode();
    }
}

// =========================================================
// [ACTION VFX] CẦU TỪ TRƯỜNG (HỌC BÁ)
// =========================================================
void DrawSkillVFX_CauTuTruong(Vector2 pPos, Vector2 ePos, float timer) {
    if (texCauTuTruong.id == 0 || texLightningVFX.id == 0) return;

    Vector2 startPos = { pPos.x + 80.0f, pPos.y - 60.0f }; // Vị trí hội tụ trước mặt Học Bá
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Vị trí nổ trên người Boss

    // Ép size ảnh gốc 1024px về cỡ 120px
    float baseScale = 120.0f / (float)texCauTuTruong.width; 

    // ================================================================
    // GIAI ĐOẠN 1 & 2: Tích tụ và Bắn đi (0.0s -> 3.0s)
    // ================================================================
    if (timer < 3.0f) {
        Vector2 currentPos = startPos;
        float currentScale = baseScale;
        float rot = timer * 600.0f; // Quả cầu xoay cực nhanh

        if (timer < 2.0f) {
            // Giai đoạn 1 (0.0s -> 2.0s): Tích tụ điện từ
            float p = timer / 2.0f;
            currentScale = baseScale * p; // To dần từ 0
            
            // Vẽ các tia sét hút vào tâm
            BeginBlendMode(BLEND_ADDITIVE);
            DrawCircleGradient(startPos.x, startPos.y, 100.0f * p, Fade(SKYBLUE, p * 0.4f), BLANK);
            for(int i = 0; i < 3; i++) {
                DrawCircle(startPos.x + GetRandomValue(-30,30)*p, startPos.y + GetRandomValue(-30,30)*p, 3.0f, Fade(YELLOW, p));
            }
            EndBlendMode();
        } else {
            // Giai đoạn 2 (2.0s -> 3.0s): Phóng vút đi
            float p = (timer - 2.0f) / 1.0f;
            float easeP = p * p * p; // Gia tốc cực mạnh
            
            currentPos.x = startPos.x + (targetPos.x - startPos.x) * easeP;
            currentPos.y = startPos.y + (targetPos.y - startPos.y) * easeP;
            
            // Vẽ vệt sáng (Trail) nối từ điểm bắt đầu đến vị trí hiện tại
            DrawLineEx(startPos, currentPos, 25.0f, Fade(SKYBLUE, 0.6f));
            DrawLineEx(startPos, currentPos, 10.0f, Fade(WHITE, 0.8f));
        }

        // Vẽ quả cầu
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texCauTuTruong, (Rectangle){0, 0, (float)texCauTuTruong.width, (float)texCauTuTruong.height},
                       (Rectangle){currentPos.x, currentPos.y, texCauTuTruong.width * currentScale, texCauTuTruong.height * currentScale},
                       (Vector2){(texCauTuTruong.width * currentScale)/2.0f, (texCauTuTruong.height * currentScale)/2.0f},
                       rot, WHITE);
        EndBlendMode();
    }

    // ================================================================
    // GIAI ĐOẠN 3: Sét Giáng & Bùng Nổ Vàng Kim (3.0s -> 3.7s)
    // ================================================================
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        BeginBlendMode(BLEND_ADDITIVE);

        // 1. 3 Tia sét đánh thẳng vào Boss (Chỉ xuất hiện chớp nhoáng ở 0.3s đầu)
        if (p < 0.4f) {
            float lightP = p / 0.4f;
            for (int i = 0; i < 3; i++) {
                float offsetX = (i - 1) * 60.0f; // Lệch 3 tia ra 3 bên
                Rectangle lightDest = { targetPos.x + offsetX, targetPos.y - 800.0f, 150.0f, 850.0f };
                DrawTexturePro(texLightningVFX, (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                               lightDest, (Vector2){75.0f, 0}, (float)GetRandomValue(-10, 10), Fade(YELLOW, 1.0f - lightP));
            }
        }

        // 2. Vụ nổ Cầu Từ Trường (Bành trướng cực to)
        float expScale = baseScale * (2.0f + p * 5.0f); // Phóng to gấp 7 lần
        DrawTexturePro(texCauTuTruong, (Rectangle){0, 0, (float)texCauTuTruong.width, (float)texCauTuTruong.height},
                       (Rectangle){targetPos.x, targetPos.y, texCauTuTruong.width * expScale, texCauTuTruong.height * expScale},
                       (Vector2){(texCauTuTruong.width * expScale)/2.0f, (texCauTuTruong.height * expScale)/2.0f},
                       timer * -300.0f, Fade(GOLD, fadeOut));

        // 3. Quầng sáng lóa mắt (Flash Trắng Vàng)
        DrawCircleGradient(targetPos.x, targetPos.y, 150.0f + p * 400.0f, Fade(WHITE, fadeOut * 0.9f), BLANK);
        DrawCircleGradient(targetPos.x, targetPos.y, 300.0f + p * 200.0f, Fade(YELLOW, fadeOut * 0.5f), BLANK);

        EndBlendMode();
    }
}

// =========================================================
// [GLOBAL BUFF] HIỆU ỨNG HỒI MÁU (Raylib Thuần)
// =========================================================
void DrawBuffVFX_Heal_Universal(Vector2 pos) {
    float timer = GetTime();
    BeginBlendMode(BLEND_ADDITIVE);
    
    // 1. Quầng sáng LIME tỏa từ dưới chân
    float pulse = fmodf(timer * 1.5f, 1.0f); 
    DrawCircleGradient(pos.x, pos.y + 30.0f, 70.0f * (0.5f + pulse), Fade(LIME, (1.0f - pulse) * 0.5f), BLANK);
    
    // 2. Các dấu "+" trôi bồng bềnh lên cao
    for (int i = 0; i < 4; i++) {
        float tp = fmodf(timer * 1.5f + (i * 0.25f), 1.0f);
        Vector2 plusPos = { 
            pos.x + sinf(timer * 3.0f + i) * 25.0f - 10.0f, 
            pos.y + 30.0f - tp * 150.0f 
        };
        DrawTextEx(globalFont, "+", plusPos, 35, 1, Fade(GREEN, 1.0f - tp));
    }
    EndBlendMode();
}

// =========================================================
// [ACTION VFX] CHIẾN THUẬT TƯ DUY (HỌC BÁ)
// =========================================================
void DrawSkillVFX_ChienThuatTuDuy(Vector2 pPos, float timer, CombatEntity *ent) {
    if (texTuDuyChienThuat.id == 0) return;
    
    Vector2 center = { pPos.x + 20.0f, pPos.y - 60.0f }; // Lơ lửng sau lưng
    float baseScale = 400.0f / (float)texTuDuyChienThuat.width;

    // GIAI ĐOẠN 1 & 2 (0.0s -> 3.0s): Thu thập dữ liệu và xử lý
    if (timer < 3.0f) {
        float p = timer / 3.0f;
        float currentScale = baseScale * (0.8f + p * 0.2f); // Phóng to chầm chậm
        
        Rectangle dest = { center.x, center.y, texTuDuyChienThuat.width * currentScale, texTuDuyChienThuat.height * currentScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        BeginBlendMode(BLEND_ADDITIVE);
        
        // Ảnh bộ não mờ ảo phía sau
        DrawTexturePro(texTuDuyChienThuat, (Rectangle){0, 0, (float)texTuDuyChienThuat.width, (float)texTuDuyChienThuat.height},
                       dest, origin, 0.0f, Fade(WHITE, p));
                       
        // Quét số lượng hiệu ứng để vẽ luồng dữ liệu (Data Streams)
        int effectCount = 0;
        for(int i = 0; i < MAX_EFFECTS; i++) {
            if(ent->effects[i].duration > 0) effectCount++;
        }
        
        // Vẽ các hạt dữ liệu hội tụ về đầu nhân vật
        int numParticles = (effectCount == 0) ? 5 : effectCount * 8; 
        for(int i = 0; i < numParticles; i++) {
            float angle = (GetTime() * 80.0f + i * 360.0f / numParticles) * DEG2RAD;
            float dist = 200.0f * (1.0f - fmodf(timer * 1.5f + i * 0.1f, 1.0f));
            Vector2 particle = { center.x + cosf(angle) * dist, center.y + sinf(angle) * dist };
            DrawCircleV(particle, 3.0f, Fade(SKYBLUE, p));
        }
        EndBlendMode();
    } 
    // GIAI ĐOẠN 3: BÙM - Tích hợp năng lượng (3.0s -> 3.7s)
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float expScale = baseScale * (1.0f + p * 1.5f);
        
        Rectangle dest = { center.x, center.y, texTuDuyChienThuat.width * expScale, texTuDuyChienThuat.height * expScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        BeginBlendMode(BLEND_ADDITIVE);
        // Ảnh nổ tung mờ dần
        DrawTexturePro(texTuDuyChienThuat, (Rectangle){0, 0, (float)texTuDuyChienThuat.width, (float)texTuDuyChienThuat.height},
                       dest, origin, 0.0f, Fade(WHITE, 1.0f - p));
        // Sóng xanh lan tỏa
        DrawCircleGradient(center.x, center.y, 100.0f + p * 350.0f, Fade(SKYBLUE, (1.0f - p) * 0.8f), BLANK);
        DrawCircleGradient(center.x, center.y, 50.0f + p * 200.0f, Fade(LIME, (1.0f - p) * 0.9f), BLANK);
        EndBlendMode();
    }
}


// =========================================================
// [ACTION VFX] ĐỊNH LÝ CUỐI CÙNG (HỌC BÁ ULTIMATE - THIÊN THẠCH HỦY DIỆT)
// =========================================================
void DrawSkillVFX_DinhLiCuoiCung(Vector2 pPos, Vector2 ePos, float timer, CombatEntity *p, CombatEntity *e) {
    if (texDinhLiCuoiCung.id == 0) return;

   int sw = SCREEN_WIDTH;   
    int sh = SCREEN_HEIGHT;  
    Vector2 centerMap = { (float)sw / 2.0f, (float)sh / 2.0f };

    // 1. Thu thập TẤT CẢ các icon hiệu ứng
    typedef struct { Texture2D *tex; Color color; bool isDown; } EffectIcon;
    EffectIcon activeIcons[MAX_EFFECTS * 2];
    int iconCount = 0;

    for (int k = 0; k < MAX_EFFECTS; k++) {
        // Debuff trên Boss
        if (e->effects[k].duration > 0) {
            Texture2D *tex = NULL; Color color = WHITE; bool isDown = false;
            switch (e->effects[k].type) {
                case EFFECT_POISON: tex = &texIconPoison; color = LIME; break;
                case EFFECT_STUN: tex = &texIconStun; color = YELLOW; break;
                case EFFECT_SPD_DOWN: tex = &texIconUpAtk; color = RED; isDown = true; break; 
                case EFFECT_DEF_DOWN: tex = &texIconUpDef; color = RED; isDown = true; break;
                case EFFECT_ATK_DOWN: tex = &texIconUpAtk; color = RED; isDown = true; break;
                case EFFECT_SILENCE: tex = &texIconSilence; color = PURPLE; break;
                default: break;
            }
            if (tex && tex->id != 0) { activeIcons[iconCount] = (EffectIcon){tex, color, isDown}; iconCount++; }
        }
        // Buff trên Học Bá
        if (p->effects[k].duration > 0) {
            Texture2D *tex = NULL; Color color = WHITE; bool isDown = false;
            switch (p->effects[k].type) {
                case EFFECT_HEAL: tex = &texIconUpHp; color = GREEN; break; 
                case EFFECT_ATK_UP: tex = &texIconUpAtk; color = SKYBLUE; break;
                case EFFECT_DEF_UP: tex = &texIconUpDef; color = SKYBLUE; break;
                case EFFECT_THORN_ARMOR: tex = &texIconUpDef; color = ORANGE; break; 
                case EFFECT_SPD_UP: tex = &texIconUpAtk; color = SKYBLUE; break; 
                default: break;
            }
            if (tex && tex->id != 0) { activeIcons[iconCount] = (EffectIcon){tex, color, isDown}; iconCount++; }
        }
    }

    // ================================================================
    // GIAI ĐOẠN 1: Bóp Méo Không Gian & Đồng Hồ Khổng Lồ (0.0s -> 1.5s)
    // ================================================================
    // [FIXED] Phủ bóng tối toàn màn hình chạy xuyên suốt 3 giây đầu để chống chớp tắt giật cục
    if (timer < 3.0f) {
        float bgAlpha = (timer < 1.5f) ? (timer / 1.5f) * 0.8f : 0.8f * (1.0f - (timer - 1.5f) / 1.5f);
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, bgAlpha));
    }
    if (timer < 1.5f) {
        float p_val = timer / 1.5f; 
        float easeOut = 1.0f - powf(1.0f - p_val, 4.0f); // Phóng to nhanh rồi chậm lại
        
       

        // Đồng hồ khổng lồ giữa màn hình (Scale bành trướng 800px)
        float maxScale = 800.0f / (float)texDinhLiCuoiCung.width;
        float clockScale = maxScale * easeOut;
        float currentRot = easeOut * 360.0f * 3.0f; // Quay cuồng 3 vòng
        
        Rectangle dest = { centerMap.x, centerMap.y - 50.0f, texDinhLiCuoiCung.width * clockScale, texDinhLiCuoiCung.height * clockScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        
        // Vẽ đồng hồ chói lòa
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texDinhLiCuoiCung, (Rectangle){0, 0, (float)texDinhLiCuoiCung.width, (float)texDinhLiCuoiCung.height},
                       dest, origin, currentRot, Fade(WHITE, p_val));
        
        // Lõi đồng hồ bùng nổ ánh sáng hút vào trong
        DrawCircleGradient(centerMap.x, centerMap.y - 50.0f, 200.0f * easeOut, Fade(GOLD, p_val * 0.8f), BLANK);
        
        // Kim quay điên cuồng như thao túng thời gian
        float handRot = p_val * 360.0f * 15.0f;
        float clockRadius = 350.0f * easeOut;
        Vector2 handEnd = { centerMap.x + cosf((handRot - 90.0f) * DEG2RAD) * clockRadius, 
                            centerMap.y - 50.0f + sinf((handRot - 90.0f) * DEG2RAD) * clockRadius };
        DrawLineEx((Vector2){centerMap.x, centerMap.y - 50.0f}, handEnd, 15.0f, Fade(WHITE, p_val));
        EndBlendMode();
    }

    // ================================================================
    // GIAI ĐOẠN 2: Mưa Thiên Thạch Dữ Liệu (1.5s -> 3.0s)
    // ================================================================
    if (timer >= 1.5f && timer <= 3.7f) {
        float p_val = (timer - 1.5f) / 2.2f; 

        // Giữ cái đồng hồ lờ mờ đằng sau làm nền vũ trụ
        float maxScale = 900.0f / (float)texDinhLiCuoiCung.width;
        Rectangle dest = { centerMap.x, centerMap.y - 50.0f, texDinhLiCuoiCung.width * maxScale, texDinhLiCuoiCung.height * maxScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        DrawTexturePro(texDinhLiCuoiCung, (Rectangle){0, 0, (float)texDinhLiCuoiCung.width, (float)texDinhLiCuoiCung.height},
                       dest, origin, timer * 20.0f, Fade(WHITE, (1.0f - p_val) * 0.3f)); // Quay chầm chậm

        // Màn hình chớp giật liên tục do áp lực
        if (GetRandomValue(1, 100) < 15) {
            DrawRectangle(0, 0, sw, sh, Fade(WHITE, 0.1f));
        }

        if (iconCount > 0) {
            for (int i = 0; i < iconCount; i++) {
                // Rải đều thời gian rơi từ 1.5s đến 2.8s
                float startTime = ((float)i / (float)iconCount) * 0.6f; 
                float endTime = startTime + 0.15f; // Tốc độ rơi cực kỳ khủng khiếp 
                
                // --- KHI THIÊN THẠCH ĐANG RƠI TỪ TRÊN TRỜI XUỐNG ---
                if (p_val >= startTime && p_val <= endTime) {
                    float flyP = (p_val - startTime) / (endTime - startTime); 
                    
                    // Điểm bắt đầu tít trên trời, văng chéo một góc
                    float offsetX = sinf(i * 99.0f) * 400.0f;
                    Vector2 startPos = { ePos.x + offsetX, ePos.y - 1200.0f };
                    
                    // Rơi cắm thẳng vào mục tiêu
                    Vector2 currentPos = { startPos.x + (ePos.x - startPos.x) * flyP, 
                                           startPos.y + (ePos.y - startPos.y) * flyP };
                    
                    // Icon khổng lồ hóa
                    float iconScale = 1.5f; 
                    Rectangle iconDest = { currentPos.x, currentPos.y, activeIcons[i].tex->width * iconScale, activeIcons[i].tex->height * iconScale };
                    if (activeIcons[i].isDown) iconDest.height *= -1;
                    
                    BeginBlendMode(BLEND_ADDITIVE);
                    // Đuôi lửa thiên thạch rực sáng
                    DrawLineEx(startPos, currentPos, 60.0f * flyP, Fade(activeIcons[i].color, 0.7f));
                    DrawLineEx(startPos, currentPos, 20.0f * flyP, Fade(WHITE, 0.9f));
                    
                    // Icon xoay tít
                    DrawTexturePro(*activeIcons[i].tex, (Rectangle){0, 0, (float)activeIcons[i].tex->width, (float)activeIcons[i].tex->height},
                                   iconDest, (Vector2){iconDest.width/2, iconDest.height/2}, flyP * 2000.0f, activeIcons[i].color);
                    EndBlendMode();
                }
                
                // --- CHẠM ĐẤT: SÉT GIÁNG & NỔ NHUỘM MÀU ---
                if (p_val > endTime && p_val < endTime + 0.3f) {
                    float expP = (p_val - endTime) / 0.3f; 
                    
                    BeginBlendMode(BLEND_ADDITIVE);
                    // 1. Sét giáng xuống phụ họa
                    if (texLightningVFX.id != 0 && expP < 0.5f) {
                        float lightW = 200.0f;
                        DrawTexturePro(texLightningVFX, (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                                       (Rectangle){ePos.x, ePos.y - 600.0f, lightW, 600.0f},
                                       (Vector2){lightW/2.0f, 0}, (float)GetRandomValue(-15, 15), Fade(activeIcons[i].color, 1.0f - expP*2.0f));
                    }
                    
                    // 2. Vụ nổ nhuộm màu
                    if (texExplosionVFX.id != 0) {
                        float boomScale = (400.0f / (float)texExplosionVFX.width) * (1.0f + expP * 2.0f);
                        DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                                       (Rectangle){ePos.x, ePos.y, texExplosionVFX.width * boomScale, texExplosionVFX.height * boomScale},
                                       (Vector2){(texExplosionVFX.width * boomScale)/2.0f, (texExplosionVFX.height * boomScale)/2.0f},
                                       GetTime() * 100.0f, Fade(activeIcons[i].color, 1.0f - expP));
                    }
                    EndBlendMode();
                }
            }
        }
        
        // ================================================================
        // GIAI ĐOẠN 3: CÚ CHỐT - ĐẬP TAN THỰC TẠI (Giây 3.0s -> 3.7s)
        // ================================================================
        if (p_val > 0.68f) { // ~3.0s
            float shatterP = (p_val - 0.68f) / 0.32f; // 0.0 -> 1.0
            float fadeOut = 1.0f - shatterP;
            
            // 1. Màn hình lóe trắng lóa cực mạnh ở tíc tắc đầu tiên
            if (shatterP < 0.2f) {
                DrawRectangle(0, 0, sw, sh, Fade(WHITE, 1.0f - (shatterP / 0.2f)));
            }

            // 2. Vết nứt không gian khổng lồ đâm xuyên Boss (Như mặt kính vỡ)
            for(int i = 0; i < 15; i++) {
                float angle = (i * (360.0f / 15) + GetRandomValue(-15, 15)) * DEG2RAD;
                float length = 800.0f * (1.0f + shatterP); // Nứt lan ra toàn màn hình
                
                Vector2 endCrack = { ePos.x + cosf(angle)*length, ePos.y + sinf(angle)*length };
                Vector2 midCrack = { ePos.x + cosf(angle)*length*0.3f + GetRandomValue(-100, 100), 
                                     ePos.y + sinf(angle)*length*0.3f + GetRandomValue(-100, 100) };
                
                DrawLineEx(ePos, midCrack, 50.0f * fadeOut, Fade(BLACK, fadeOut));
                DrawLineEx(midCrack, endCrack, 25.0f * fadeOut, Fade(GOLD, fadeOut));
                DrawLineEx(midCrack, endCrack, 10.0f * fadeOut, Fade(WHITE, fadeOut)); 
            }

            // 3. Vụ nổ siêu tân tinh (Supernova shockwave)
            BeginBlendMode(BLEND_ADDITIVE);
            DrawCircleGradient(ePos.x, ePos.y, 300.0f + shatterP * 1500.0f, Fade(GOLD, fadeOut * 0.9f), BLANK);
            DrawCircleGradient(ePos.x, ePos.y, 100.0f + shatterP * 600.0f, Fade(WHITE, fadeOut), BLANK);
            if (texExplosionVFX.id != 0) {
                float finalExpScale = (800.0f / (float)texExplosionVFX.width) * (1.0f + shatterP * 2.5f);
                DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                               (Rectangle){ePos.x, ePos.y, texExplosionVFX.width * finalExpScale, texExplosionVFX.height * finalExpScale},
                               (Vector2){(texExplosionVFX.width * finalExpScale)/2.0f, (texExplosionVFX.height * finalExpScale)/2.0f},
                               GetTime() * 80.0f, Fade(WHITE, fadeOut));
            }
            EndBlendMode();
        }
    }
}

// =========================================================
// [ACTION VFX] HÀO QUANG HUYẾT SẮC (SOÁI CA)
// =========================================================
void DrawSkillVFX_HaoQuangHuyetSac(Vector2 pPos, float timer) {
    if (texHaoQuangHuyetSac.id == 0) return;
    Vector2 center = { pPos.x + 30.0f, pPos.y - 60.0f }; // Tâm ở ngực nhân vật

    // Ép size ảnh gốc 1024px về cỡ 250px
    float baseScale = 250.0f / (float)texHaoQuangHuyetSac.width;

    if (timer < 3.0f) {
        float p = timer / 3.0f; // 0.0 -> 1.0
        float scale = baseScale * (0.8f + p * 0.4f); // Nở ra từ từ
        float rot = timer * 100.0f;

        Rectangle dest = { center.x, center.y, texHaoQuangHuyetSac.width * scale, texHaoQuangHuyetSac.height * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        // Lớp 1: Hào quang đỏ thẫm (MAROON)
        DrawTexturePro(texHaoQuangHuyetSac, (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                       dest, origin, rot, Fade(MAROON, 0.8f * (1.0f - p * 0.3f)));
        // Lớp 2: Lõi đỏ tươi xoay ngược chiều
        DrawTexturePro(texHaoQuangHuyetSac, (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                       dest, origin, -rot * 0.5f, Fade(RED, 0.5f));
        EndBlendMode();

        // Hạt máu bay lên
        for (int i = 0; i < 4; i++) {
            float tp = fmodf(timer * 2.0f + (i * 0.25f), 1.0f);
            Vector2 plusPos = { center.x + sinf(timer * 5.0f + i) * 40.0f, center.y + 60.0f - tp * 180.0f };
            DrawTextEx(globalFont, "+", plusPos, 40, 1, Fade(RED, 1.0f - tp));
        }
    } 
    // Giây 3.0: Bùng nổ hấp thụ máu
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float scale = baseScale * (1.2f + p * 1.5f);
        Rectangle dest = { center.x, center.y, texHaoQuangHuyetSac.width * scale, texHaoQuangHuyetSac.height * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texHaoQuangHuyetSac, (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                       dest, origin, timer * 150.0f, Fade(MAROON, 1.0f - p));
        DrawCircleGradient(center.x, center.y, 80.0f + p * 300.0f, Fade(RED, (1.0f - p) * 0.8f), BLANK);
        EndBlendMode();
    }
}

// =========================================================
// [ACTION VFX] HUYẾT TIỄN (SOÁI CA)
// =========================================================
void DrawSkillVFX_HuyetTien(Vector2 pPos, Vector2 ePos, float timer) {
    if (texHuyetTien.id == 0 || texBlackHoleVFX.id == 0) return;

    Vector2 startPos = { pPos.x + 80.0f, pPos.y - 60.0f }; // Trước mặt Soái Ca
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Ngực Boss

    // GIAI ĐOẠN 1: Hố đen mở ra, mũi tên thò ra (0.0s -> 2.5s)
    if (timer < 2.5f) {
        float p = timer / 2.5f;

        // 1. Vẽ hố đen đỏ máu bằng cách nhuộm màu MAROON
        float bhScale = (180.0f / (float)texBlackHoleVFX.width) * (0.2f + p * 0.8f);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){startPos.x, startPos.y, texBlackHoleVFX.width * bhScale, texBlackHoleVFX.height * bhScale},
                       (Vector2){(texBlackHoleVFX.width * bhScale)/2.0f, (texBlackHoleVFX.height * bhScale)/2.0f},
                       timer * -500.0f, Fade(MAROON, p));
        EndBlendMode();

        // 2. Mũi tên từ từ trồi ra (từ giây 1.0)
        float arrowP = (timer > 1.0f) ? (timer - 1.0f) / 1.5f : 0.0f;
        if (arrowP > 0.0f) {
            float arrowScale = 200.0f / (float)texHuyetTien.width;
            float angle = atan2f(targetPos.y - startPos.y, targetPos.x - startPos.x) * RAD2DEG;
            
            // Offset để thò từ trong hố đen ra ngoài (-80px lùi về sau, tiến dần lên +20px)
            float offsetDist = -80.0f + arrowP * 100.0f;
            Vector2 arrowPos = { startPos.x + cosf(angle * DEG2RAD) * offsetDist,
                                 startPos.y + sinf(angle * DEG2RAD) * offsetDist };

            // Vẽ mũi tên mờ dần khi mới nhú ra
            DrawTexturePro(texHuyetTien, (Rectangle){0, 0, (float)texHuyetTien.width, (float)texHuyetTien.height},
                           (Rectangle){arrowPos.x, arrowPos.y, texHuyetTien.width * arrowScale, texHuyetTien.height * arrowScale},
                           (Vector2){(texHuyetTien.width * arrowScale)/2.0f, (texHuyetTien.height * arrowScale)/2.0f},
                           angle, Fade(WHITE, arrowP));
        }
    }
    // GIAI ĐOẠN 2: Bắn tỉa cực nhanh (2.5s -> 3.0s)
    else if (timer >= 2.5f && timer < 3.0f) {
        float p = (timer - 2.5f) / 0.5f;
        float easeP = p * p * p; // Gia tốc khối

        Vector2 currentPos = { startPos.x + (targetPos.x - startPos.x) * easeP,
                               startPos.y + (targetPos.y - startPos.y) * easeP };
        
        float angle = atan2f(targetPos.y - startPos.y, targetPos.x - startPos.x) * RAD2DEG;
        float arrowScale = 200.0f / (float)texHuyetTien.width;

        // Hố đen thu nhỏ nhanh
        float bhScale = (180.0f / (float)texBlackHoleVFX.width) * (1.0f - p);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texBlackHoleVFX, (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                       (Rectangle){startPos.x, startPos.y, texBlackHoleVFX.width * bhScale, texBlackHoleVFX.height * bhScale},
                       (Vector2){(texBlackHoleVFX.width * bhScale)/2.0f, (texBlackHoleVFX.height * bhScale)/2.0f},
                       timer * -800.0f, Fade(MAROON, 1.0f - p));
        
        // Vệt trail đỏ bay theo mũi tên
        DrawLineEx(startPos, currentPos, 20.0f, Fade(RED, 0.7f));
        EndBlendMode();

        DrawTexturePro(texHuyetTien, (Rectangle){0, 0, (float)texHuyetTien.width, (float)texHuyetTien.height},
                       (Rectangle){currentPos.x, currentPos.y, texHuyetTien.width * arrowScale, texHuyetTien.height * arrowScale},
                       (Vector2){(texHuyetTien.width * arrowScale)/2.0f, (texHuyetTien.height * arrowScale)/2.0f},
                       angle, WHITE);
    }
    // GIAI ĐOẠN 3: Va chạm nổ tung máu (3.0s -> 3.7s)
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        BeginBlendMode(BLEND_ADDITIVE);
        // Nhuộm ảnh vụ nổ thành màu máu (MAROON)
        if (texExplosionVFX.id != 0) {
            float expScale = (350.0f / (float)texExplosionVFX.width) * (1.0f + p * 1.5f);
            DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                           (Rectangle){targetPos.x, targetPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                           (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f},
                           timer * 100.0f, Fade(MAROON, fadeOut));
        }
        
        // Hạt pixel văng ra tứ tung đại diện cho giọt máu
        for (int i = 0; i < 30; i++) {
            Vector2 particle = {
                targetPos.x + GetRandomValue(-200, 200) * p,
                targetPos.y + GetRandomValue(-200, 200) * p + p * 80.0f // Rơi nhẹ xuống đất
            };
            float size = GetRandomValue(10, 25) * fadeOut;
            Color bloodColor = (GetRandomValue(0,1) == 0) ? RED : MAROON;
            DrawRectangleV(particle, (Vector2){size, size}, Fade(bloodColor, fadeOut));
        }
        DrawCircleGradient(targetPos.x, targetPos.y, 100.0f + p * 250.0f, Fade(RED, fadeOut * 0.8f), BLANK);
        EndBlendMode();
    }
}
// =========================================================
// [GLOBAL BUFF] HÀO QUANG HUYẾT SẮC (DUY TRÌ TỪNG TURN)
// =========================================================
void DrawBuffVFX_HaoQuangHuyetSac_Continuous(Vector2 pos) {
    if (texHaoQuangHuyetSac.id == 0) return;
    
    // Dùng GetTime() để animation quay liên tục không bị phụ thuộc vào stateTimer
    float timer = GetTime(); 
    float baseScale = 220.0f / (float)texHaoQuangHuyetSac.width;
    
    // Pulsate nhẹ nhàng (nhịp đập trái tim)
    float pulse = 1.0f + sinf(timer * 4.0f) * 0.08f;
    float scale = baseScale * pulse;
    float rot = timer * 60.0f; // Quay đều đặn ngầu lòi

    Rectangle dest = { pos.x, pos.y, texHaoQuangHuyetSac.width * scale, texHaoQuangHuyetSac.height * scale };
    Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

    BeginBlendMode(BLEND_ADDITIVE);
    // Lớp nền đỏ thẫm mờ ảo
    DrawTexturePro(texHaoQuangHuyetSac, (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                   dest, origin, rot, Fade(MAROON, 0.5f));
    // Lớp lõi đỏ tươi xoay ngược chiều
    DrawTexturePro(texHaoQuangHuyetSac, (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                   dest, origin, -rot * 0.7f, Fade(RED, 0.35f));
    EndBlendMode();
}

// =========================================================
// [ACTION VFX] DẤU ẤN KÝ SINH (W3 - SOÁI CA)
// =========================================================
void DrawSkillVFX_DauAnKiSinh(Vector2 ePos, float timer) {
    if (texDauAnKiSinh.id == 0) return;
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Vị trí ngực Boss
    float baseScale = 250.0f / (float)texDauAnKiSinh.width;

    // Giai đoạn 1 & 2: Xuất hiện, Phình to, rồi Co rụt đập thình thịch
    if (timer < 3.0f) {
        float scale = baseScale;
        if (timer < 1.5f) { // 1.5s đầu: Nở ra từ từ
            float p = timer / 1.5f;
            scale *= (0.2f + p * 1.5f);
        } else { // 1.5s -> 3.0s: Co lại và đập như nhịp tim
            float p = (timer - 1.5f) / 1.5f;
            scale *= (1.7f - p * 0.7f); // Thu nhỏ bớt
            scale += sinf(timer * 40.0f) * 0.05f; // Rung lắc nhẹ
        }

        Rectangle dest = { targetPos.x, targetPos.y, texDauAnKiSinh.width * scale, texDauAnKiSinh.height * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texDauAnKiSinh, (Rectangle){0, 0, (float)texDauAnKiSinh.width, (float)texDauAnKiSinh.height},
                       dest, origin, timer * 60.0f, WHITE);
        // Lõi đỏ máu tỏa sáng
        DrawCircleGradient(targetPos.x, targetPos.y, 100.0f * scale, Fade(MAROON, 0.6f), BLANK);
        EndBlendMode();
    } 
    // Giai đoạn 3: Giây 3.0 Nổ tung sắc máu
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        BeginBlendMode(BLEND_ADDITIVE);
        if (texExplosionVFX.id != 0) {
            float expScale = (350.0f / (float)texExplosionVFX.width) * (1.0f + p * 2.0f);
            DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                           (Rectangle){targetPos.x, targetPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                           (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f},
                           timer * 100.0f, Fade(MAROON, fadeOut));
        }
        // Vòng sóng siêu âm màu máu
        DrawCircleLines(targetPos.x, targetPos.y, 50.0f + p * 300.0f, Fade(RED, fadeOut));
        DrawCircleGradient(targetPos.x, targetPos.y, 150.0f + p * 200.0f, Fade(RED, fadeOut * 0.8f), BLANK);
        EndBlendMode();
    }
}

// =========================================================
// [ACTION VFX] GIAO KÈO ÁC QUỶ (W4 - SOÁI CA)
// =========================================================
void DrawSkillVFX_GiaoKeoAcQuy(Vector2 pPos, float timer) {
    if (texGiaoKeoAcQuy.id == 0) return;
    Vector2 center = { pPos.x + 10.0f, pPos.y - 40.0f };
    float baseScale = 300.0f / (float)texGiaoKeoAcQuy.width;

    if (timer < 3.0f) { // Triệu hồi trận pháp dưới chân
        float p = timer / 3.0f;
        float scale = baseScale * (0.5f + p * 0.7f);
        Rectangle dest = { center.x, center.y, texGiaoKeoAcQuy.width * scale, texGiaoKeoAcQuy.height * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        // Trận pháp xoay nhẹ và sáng bừng lên
        DrawTexturePro(texGiaoKeoAcQuy, (Rectangle){0, 0, (float)texGiaoKeoAcQuy.width, (float)texGiaoKeoAcQuy.height},
                       dest, origin, sinf(timer * 4.0f) * 15.0f, Fade(ORANGE, p));
        // Lửa bốc lên
        DrawCircleGradient(center.x, center.y, 100.0f + sinf(timer * 15.0f) * 20.0f, Fade(RED, p * 0.6f), BLANK);
        EndBlendMode();
    } else if (timer >= 3.0f && timer <= 3.7f) { // Bùng nổ Giao kèo
        float p = (timer - 3.0f) / 0.7f;
        float scale = baseScale * (1.2f + p * 1.5f);
        Rectangle dest = { center.x, center.y, texGiaoKeoAcQuy.width * scale, texGiaoKeoAcQuy.height * scale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texGiaoKeoAcQuy, (Rectangle){0, 0, (float)texGiaoKeoAcQuy.width, (float)texGiaoKeoAcQuy.height},
                       dest, origin, 0.0f, Fade(RED, 1.0f - p));
        DrawCircleGradient(center.x, center.y, 150.0f + p * 400.0f, Fade(ORANGE, 1.0f - p), BLANK);
        EndBlendMode();
    }
}

// [GLOBAL BUFF] GIAO KÈO ÁC QUỶ (BÁM THEO NGƯỜI - GIỮ MÀU GỐC)
// =========================================================
void DrawBuffVFX_GiaoKeoAcQuy_Continuous(Vector2 pos) {
    if (texGiaoKeoAcQuy.id == 0) return;
    float timer = GetTime();
    float baseScale = 220.0f / (float)texGiaoKeoAcQuy.width; 
    
    // Quỷ rung lắc và nhịp đập phập phồng như đang thở
    float pulse = 1.0f + sinf(timer * 4.0f) * 0.04f;
    float scale = baseScale * pulse;
    
    // Vị trí: Lơ lửng ngay sau lưng / trên đầu người chơi
    Rectangle dest = { pos.x, pos.y - 30.0f, texGiaoKeoAcQuy.width * scale, texGiaoKeoAcQuy.height * scale };
    Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };

    // BỎ BLEND_ADDITIVE: Vẽ ảnh gốc với màu chuẩn, chỉ chỉnh Alpha (độ mờ) nhấp nháy
    // Quỷ lắc lư nhẹ 2 bên (sinf) để tạo cảm giác sống động
    DrawTexturePro(texGiaoKeoAcQuy, (Rectangle){0, 0, (float)texGiaoKeoAcQuy.width, (float)texGiaoKeoAcQuy.height},
                   dest, origin, sinf(timer * 2.0f) * 5.0f, Fade(WHITE, 0.7f + sinf(timer * 8.0f) * 0.2f));

    // Phụ họa thêm sương mù máu chui lên từ dưới chân
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient(pos.x, pos.y + 40.0f, 80.0f + sinf(timer * 12.0f) * 15.0f, Fade(MAROON, 0.5f), BLANK);
    EndBlendMode();
}

// =========================================================
// [ACTION VFX] VẠN TIỄN XUYÊN TÂM (ULTIMATE - SOÁI CA)
// =========================================================
void DrawSkillVFX_VanTienXuyenTam(Vector2 pPos, Vector2 ePos, float timer) {
    if (texHuyetTien.id == 0) return;
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f };

    // 1.0s -> 3.0s: Sấy 40 mũi tên như súng Gatling 6 nòng
    if (timer >= 1.0f && timer < 3.0f) {
        int numArrows = 40; 
        float interval = 2.0f / numArrows; // Mỗi mũi cách nhau 0.05s

        for (int i = 0; i < numArrows; i++) {
            float spawnTime = 1.0f + i * interval;
            if (timer > spawnTime) {
                float flyTime = timer - spawnTime;
                
                // Thuật toán tạo "độ giật" và "tỏa" ngẫu nhiên nhưng cố định theo index i
                float offsetStartX = sinf(i * 123.4f) * 80.0f;
                float offsetStartY = cosf(i * 321.4f) * 80.0f;
                float offsetEndX = sinf(i * 555.5f) * 100.0f; // Boss bị găm nát tươm
                float offsetEndY = cosf(i * 777.7f) * 100.0f;

                Vector2 start = { pPos.x + 80.0f + offsetStartX, pPos.y - 20.0f + offsetStartY };
                Vector2 end = { targetPos.x + offsetEndX, targetPos.y + offsetEndY };

                if (flyTime < 0.25f) { // Bay siêu nhanh mất 0.25s
                    float p = flyTime / 0.25f;
                    Vector2 current = { start.x + (end.x - start.x) * p, start.y + (end.y - start.y) * p };
                    float angle = atan2f(end.y - start.y, end.x - start.x) * RAD2DEG;
                    float scale = 150.0f / (float)texHuyetTien.width;

                    BeginBlendMode(BLEND_ADDITIVE);
                    DrawLineEx(start, current, 8.0f, Fade(RED, 0.6f)); // Lõi laze bắn tỉa
                    DrawTexturePro(texHuyetTien, (Rectangle){0, 0, (float)texHuyetTien.width, (float)texHuyetTien.height},
                                   (Rectangle){current.x, current.y, texHuyetTien.width * scale, texHuyetTien.height * scale},
                                   (Vector2){(texHuyetTien.width * scale)/2.0f, (texHuyetTien.height * scale)/2.0f},
                                   angle, WHITE);
                    EndBlendMode();
                } 
                // Nổ đôm đốp trên người Boss
                else if (flyTime >= 0.25f && flyTime < 0.45f) { 
                    float p = (flyTime - 0.25f) / 0.2f;
                    DrawCircleGradient(end.x, end.y, 40.0f * (1.0f + p), Fade(RED, 1.0f - p), BLANK);
                }
            }
        }
    } 
    // Giây 3.0 -> 3.7: Cú nổ cộng dồn quét sạch
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;
        
        // Cột sáng đỏ rực cắm từ trên trời xuống Boss
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangle(targetPos.x - 100.0f - p*50.0f, targetPos.y - 1000.0f, 200.0f + p*100.0f, 1000.0f, Fade(RED, fadeOut * 0.5f));
        DrawCircleGradient(targetPos.x, targetPos.y, 150.0f + p * 400.0f, Fade(MAROON, fadeOut), BLANK);
        
        if (texExplosionVFX.id != 0) {
            float expScale = (600.0f / (float)texExplosionVFX.width) * (1.0f + p * 2.0f);
            DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, (float)texExplosionVFX.width, (float)texExplosionVFX.height},
                           (Rectangle){targetPos.x, targetPos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                           (Vector2){(texExplosionVFX.width * expScale)/2.0f, (texExplosionVFX.height * expScale)/2.0f},
                           timer * 150.0f, Fade(MAROON, fadeOut)); // ĐÃ ĐỔI THÀNH MAROON (ĐỎ MÁU THẪM)
        }
        EndBlendMode();
    }
}

// =========================================================
// [ACTION VFX] THU HỒI VỐN (W1 - PHÚ NHỊ ĐẠI) - CHUẨN PNG
// =========================================================
// =========================================================
// [ACTION VFX] THU HỒI VỐN (W1 - PHÚ NHỊ ĐẠI) - BẢN NẶNG ĐÔ
// =========================================================
void DrawSkillVFX_ThuHoiVon(Vector2 currentPos, Vector2 ePos, float timer) {
    if (texThuHoiVonCoin.id == 0) return;

    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Ngực Boss
    // Vì ta đã code dashOffset ở Bước 1, nên currentPos giờ đây chính là vị trí thực tế của Player đang lao ra!
    Vector2 playerCenter = { currentPos.x + 30.0f, currentPos.y - 60.0f }; 

    const int numCoins = 25; // Tăng thêm số lượng tiền cho áp đảo
    float baseScale = 50.0f / (float)texThuHoiVonCoin.width;

    // GIAI ĐOẠN 1: TỤ TIỀN & BAY XUYÊN ÂM (0.7s -> 3.0s)
    if (timer > 0.7f && timer <= 3.0f) {
        for (int i = 0; i < numCoins; i++) {
            Vector2 coinPos;
            float coinRotation = timer * 1000.0f; // Xoay chóng mặt
            
            if (timer <= 2.2f) {
                // Tiền xoay quanh người (Hút vào)
                float p = (timer - 0.7f) / 1.5f;
                float radius = 200.0f * (1.0f - p * 0.7f); // Thu hẹp vòng vây
                float angle = (i * (360.0f / numCoins)) + p * p * 2000.0f; 

                coinPos.x = playerCenter.x + cosf(angle * DEG2RAD) * radius;
                coinPos.y = playerCenter.y + sinf(angle * DEG2RAD) * radius;
                
                // Hào quang gồng lực
                BeginBlendMode(BLEND_ADDITIVE);
                DrawCircleGradient(playerCenter.x, playerCenter.y, 80.0f * p, Fade(GOLD, 0.2f), BLANK);
                EndBlendMode();
            } 
            else {
                // PHÓNG TIỀN: Quăng như bão tố
                float p = (timer - 2.2f) / 0.8f;
                float lastRadius = 200.0f * 0.3f;
                float lastAngle = (i * (360.0f / numCoins)) + 1.0f * 2000.0f;
                Vector2 startThrowPos = {
                    playerCenter.x + cosf(lastAngle * DEG2RAD) * lastRadius,
                    playerCenter.y + sinf(lastAngle * DEG2RAD) * lastRadius
                };

                // Delay từng đồng xu để tạo thành dải đạn (Gatling gun)
                float delayStep = 0.6f / numCoins; // Tổng delay chiếm tối đa 60% thời gian phóng
                float individualP = (p - (i * delayStep)) / (1.0f - numCoins * delayStep);
                
                if (individualP < 0.0f) individualP = 0.0f;
                if (individualP > 1.0f) individualP = 1.0f;

                // GIA TỐC CỰC GẮT: p^4 (Chậm -> Xé gió)
                float easeInP = individualP * individualP * individualP * individualP;

                coinPos.x = startThrowPos.x + (targetPos.x - startThrowPos.x) * easeInP;
                coinPos.y = startThrowPos.y + (targetPos.y - startThrowPos.y) * individualP;

                // Tia sáng kéo đuôi theo đồng xu (Tăng độ tốc độ)
                if (individualP > 0.1f && individualP < 1.0f) {
                    BeginBlendMode(BLEND_ADDITIVE);
                    DrawLineEx(startThrowPos, coinPos, 4.0f, Fade(YELLOW, 0.5f));
                    EndBlendMode();
                }
            }

            // Vẽ đồng tiền (Chỉ vẽ nếu chưa găm vào Boss)
            if (timer < 3.0f) {
                Rectangle dest = { coinPos.x, coinPos.y, texThuHoiVonCoin.width * baseScale, texThuHoiVonCoin.height * baseScale };
                Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
                
                DrawTexturePro(texThuHoiVonCoin, (Rectangle){0, 0, (float)texThuHoiVonCoin.width, (float)texThuHoiVonCoin.height},
                               dest, origin, coinRotation, WHITE);
            }
        }
    }

    // GIAI ĐOẠN 2: CHỚP SÁNG VÀ VA CHẠM NẶNG (3.0s -> 3.7s)
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        BeginBlendMode(BLEND_ADDITIVE);
        // Chớp sáng flash toàn màn hình ngay lúc 3.0s
        if (p < 0.1f) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(YELLOW, 0.4f * (1.0f - p*10.0f)));
        }

        // Vụ nổ hoàng kim đè nát Boss
        DrawCircleGradient(targetPos.x, targetPos.y, 150.0f + p * 300.0f, Fade(GOLD, fadeOut * 0.9f), BLANK);
        DrawCircleLines(targetPos.x, targetPos.y, 80.0f + p * 400.0f, Fade(YELLOW, fadeOut));
        
        // Luồng tiền/sinh khí hút ngược về Player (Cực nhanh)
        Vector2 returnPos = {
            targetPos.x + (playerCenter.x - targetPos.x) * (1.0f - fadeOut * fadeOut),
            targetPos.y + (playerCenter.y - targetPos.y) * (1.0f - fadeOut * fadeOut)
        };
        DrawCircleGradient(returnPos.x, returnPos.y, 60.0f, Fade(GREEN, fadeOut), BLANK);
        DrawLineEx(targetPos, returnPos, 15.0f * fadeOut, Fade(YELLOW, fadeOut));

        EndBlendMode();
    }
}

// =========================================================
// [ACTION VFX] PHÁ GIÁ THỊ TRƯỜNG (W2 - PHÚ NHỊ ĐẠI)
// Mũi tên giáng xuống chạm đích đúng mốc 3.0s, sau đó nổ tiền!
// =========================================================
void DrawSkillVFX_PhaGiaThiTruong(Vector2 ePos, float timer) {
    if (texPhaGia.id == 0) return;

    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Điểm rơi ngay ngực Boss

    // GIAI ĐOẠN 1: Mũi tên rơi tự do (0.0s -> 3.0s)
    if (timer < 3.0f) {
        float p = timer / 3.0f; 
        
        // Gia tốc rơi cực gắt (Rơi chậm ở trên cao, rớt cái vèo khi sát đất)
        float dropP = p * p * p * p; 

        float startY = targetPos.y - 900.0f; // Triệu hồi từ tít trên trời
        float currentY = startY + (targetPos.y - startY) * dropP;

        // Ép size mũi tên to bự (Khoảng 250px)
        float scale = 250.0f / (float)texPhaGia.width;

        // Vẽ mũi tên hướng xuống (xoay 180 độ)
        DrawTexturePro(texPhaGia,
            (Rectangle){ 0, 0, (float)texPhaGia.width, (float)texPhaGia.height },
            (Rectangle){ targetPos.x, currentY, texPhaGia.width * scale, texPhaGia.height * scale },
            (Vector2){ (texPhaGia.width * scale) / 2.0f, (texPhaGia.height * scale) / 2.0f },
            180.0f, WHITE);
    }
    // GIAI ĐOẠN 2: Bùng nổ vô số đồng tiền văng tung tóe (3.0s -> 3.7s)
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        BeginBlendMode(BLEND_ADDITIVE);
        // Sóng xung kích lúc va chạm
        DrawCircleGradient(targetPos.x, targetPos.y, 100.0f + p * 300.0f, Fade(GOLD, fadeOut * 0.8f), BLANK);
        
        // Hạt tiền văng ra (Tận dụng luôn texCoinVFX có sẵn trong file của bạn)
        if (texCoinVFX.id != 0) {
            for (int i = 0; i < 25; i++) {
                // Toán học tính góc và độ văng
                float angle = (i * (360.0f / 25)) * DEG2RAD;
                float dist = p * 400.0f * (0.4f + (i % 3) * 0.3f); // Văng xa gần xen kẽ
                
                // Trọng lực giả: Tiền văng ra rồi rớt xuống (cộng thêm p * p * 300.0f vào trục Y)
                Vector2 coinPos = { 
                    targetPos.x + cosf(angle) * dist, 
                    targetPos.y + sinf(angle) * dist + (p * p * 300.0f) 
                };

                float coinScale = 0.6f;
                DrawTexturePro(texCoinVFX,
                    (Rectangle){0, 0, (float)texCoinVFX.width, (float)texCoinVFX.height},
                    (Rectangle){coinPos.x, coinPos.y, texCoinVFX.width * coinScale, texCoinVFX.height * coinScale},
                    (Vector2){(texCoinVFX.width * coinScale)/2.0f, (texCoinVFX.height * coinScale)/2.0f},
                    timer * 1000.0f + i * 50.0f, Fade(WHITE, fadeOut)); // Tiền xoay tít
            }
        }
        EndBlendMode();
    }
}

// Hàm vẽ Tâm ngắm bắn tỉa [BẢN NÂNG CẤP - SÁNG RỰC & DÀY DẶN]
void DrawSniperCrosshair(Vector2 pos, float size, Color c) {
    if (size <= 0) return; // Tránh lỗi vẽ khi size âm

    // Lấy tỷ lệ Alpha hiện tại của màu truyền vào để đồng bộ hóa Fade
    float alphaFloat = (float)c.a / 255.0f;

    BeginBlendMode(BLEND_ADDITIVE);

    // 1. Vẽ Glow mờ lót nền (Giúp tâm ngắm nổi bật trên mọi loại background)
    DrawCircleGradient(pos.x, pos.y, size * 1.5f, Fade(RED, alphaFloat * 0.3f), BLANK);

    // 2. Vòng tròn ngoài (Dùng DrawRing để tạo độ dày 4 pixel thay vì 1 pixel)
    DrawRing(pos, size - 4.0f, size, 0.0f, 360.0f, 36, c);
    
    // 3. Vòng tròn trong (Nhỏ hơn, mờ hơn một chút, tạo viền công nghệ)
    if (size > 15.0f) {
        DrawRing(pos, size - 12.0f, size - 9.0f, 0.0f, 360.0f, 36, Fade(c, 0.5f)); 
    }

    // 4. 4 vạch ngắm (Làm dày hơn lên 5.0f và kéo dài ra ngoài một chút)
    float thick = 5.0f;
    DrawLineEx((Vector2){pos.x - size - 30, pos.y}, (Vector2){pos.x - size + 10, pos.y}, thick, c);
    DrawLineEx((Vector2){pos.x + size + 30, pos.y}, (Vector2){pos.x + size - 10, pos.y}, thick, c);
    DrawLineEx((Vector2){pos.x, pos.y - size - 30}, (Vector2){pos.x, pos.y - size + 10}, thick, c);
    DrawLineEx((Vector2){pos.x, pos.y + size + 30}, (Vector2){pos.x, pos.y + size - 10}, thick, c);

    // 5. Chấm laser giữa tâm
    DrawCircle(pos.x, pos.y, 6.0f, c);               // Vỏ đỏ
    DrawCircle(pos.x, pos.y, 2.5f, Fade(WHITE, alphaFloat)); // Lõi trắng cực chói

    EndBlendMode();
}

// =========================================================
// [ACTION VFX] DÒ TÌM CON MỒI (W3 - PHÚ NHỊ ĐẠI) - TIMING CHUẨN
// =========================================================
void DrawSkillVFX_DoTimConMoi(Vector2 pPos, Vector2 ePos, float timer, int sw, int sh) {
    Vector2 mapCenter = { (float)sw / 2.0f, (float)sh / 2.0f };
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Tâm Boss
    Vector2 playerCenter = { pPos.x + 30.0f, pPos.y - 50.0f }; // Tâm Player

    // 1. GIAI ĐOẠN 1: Radar quét (0.0s -> 2.4s)
    if (timer < 2.4f) {
        // Màn hình tối sầm lại nhanh chóng ở đầu, và sáng lại ở cuối
        float bgAlpha = (timer < 0.4f) ? (timer / 0.4f) * 0.7f : ((timer > 2.0f) ? (2.4f - timer)/0.4f * 0.7f : 0.7f);
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, bgAlpha));

        if (texDoTimConMoi.id != 0) {
            // Radar phóng to vọt lên trong 0.4s đầu, sau đó GIỮ NGUYÊN size để quét cho rõ
            float scaleP = (timer < 0.4f) ? (timer / 0.4f) : 1.0f;
            // Dùng hàm powf để tạo cảm giác bật nảy (Pop-up)
            float easeScale = 1.0f - powf(1.0f - scaleP, 3.0f); 
            float scale = (700.0f / (float)texDoTimConMoi.width) * easeScale; 
            
            // Mờ dần ở 0.4s cuối để chuyển mượt sang Tâm ngắm
            float radarAlpha = (timer > 2.0f) ? (2.4f - timer) / 0.4f : scaleP;

            // Tốc độ xoay: 300 độ/giây. Trong 2.4s sẽ quét được tròn trĩnh 2 vòng (720 độ) cực mượt.
            float rot = timer * 300.0f; 
            
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texDoTimConMoi, 
                (Rectangle){0, 0, (float)texDoTimConMoi.width, (float)texDoTimConMoi.height},
                (Rectangle){mapCenter.x, mapCenter.y, texDoTimConMoi.width * scale, texDoTimConMoi.height * scale},
                (Vector2){(texDoTimConMoi.width * scale)/2.0f, (texDoTimConMoi.height * scale)/2.0f},
                rot, Fade(LIME, radarAlpha)); 
            EndBlendMode();
        }
    }
    
    // 2. GIAI ĐOẠN 2: Khóa mục tiêu Lock-on (2.0s -> 3.0s)
    // Cố tình cho đè lên Radar 0.4s (từ 2.0s) để hiệu ứng nối tiếp nhau mượt mà không bị khựng
    if (timer >= 2.0f && timer < 3.0f) {
        float p = (timer - 2.0f) / 1.0f;
        
        // Tâm ngắm đỏ chót ép từ màn hình cực to (600px) thít chặt vào người Boss (50px)
        float crossSize = 600.0f * (1.0f - p * p) + 50.0f; 
        
        // Rung lắc nhẹ khi đang siết mục tiêu
        Vector2 shakeTarget = targetPos;
        if (p > 0.6f) {
            shakeTarget.x += GetRandomValue(-3, 3);
            shakeTarget.y += GetRandomValue(-3, 3);
        }

        DrawSniperCrosshair(shakeTarget, crossSize, Fade(RED, p));
        
        // Từ 2.4s (lúc radar vừa tắt), bắt đầu kéo tia dữ liệu về phía Player
        if (timer >= 2.4f) {
            float lineP = (timer - 2.4f) / 0.6f;
            BeginBlendMode(BLEND_ADDITIVE);
            DrawLineEx(shakeTarget, playerCenter, 15.0f * lineP, Fade(LIME, lineP));
            // Hào quang bắt đầu nhen nhóm quanh Player
            DrawCircleGradient(playerCenter.x, playerCenter.y, 150.0f * lineP, Fade(GOLD, lineP * 0.5f), BLANK);
            EndBlendMode();
        }
    }
    
    // 3. GIAI ĐOẠN 3: Bùng nổ Hào quang Buff (3.0s -> 3.7s)
    else if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f;
        float fadeOut = 1.0f - p;

        // Flash chớp sáng nhẹ màu vàng ở giây 3.0
        if (p < 0.2f) {
            DrawRectangle(0, 0, sw, sh, Fade(GOLD, 0.3f * (1.0f - p/0.2f)));
        }

        // Vẫn giữ tâm ngắm đỏ trên Boss, nhưng nhấp nháy mạnh
        float pulseCross = 50.0f + sinf(timer * 40.0f) * 15.0f;
        DrawSniperCrosshair(targetPos, pulseCross, Fade(RED, fadeOut));

        // Hào Quang Vàng Kim bùng nổ
        if (texHaoQuangHuyetSac.id != 0) {
            float auraScale = (500.0f / (float)texHaoQuangHuyetSac.width) * (0.8f + p * 0.5f);
            
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texHaoQuangHuyetSac, 
                (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                (Rectangle){playerCenter.x, playerCenter.y, texHaoQuangHuyetSac.width * auraScale, texHaoQuangHuyetSac.height * auraScale},
                (Vector2){(texHaoQuangHuyetSac.width * auraScale)/2.0f, (texHaoQuangHuyetSac.height * auraScale)/2.0f},
                timer * 100.0f, Fade(WHITE, fadeOut)); 
            
            DrawCircleGradient(playerCenter.x, playerCenter.y, 100.0f + p * 300.0f, Fade(GOLD, fadeOut * 0.9f), BLANK);
            EndBlendMode();
        }
    }
}

// =========================================================
// [GLOBAL BUFF] DÒ TÌM CON MỒI (DUY TRÌ TỪNG TURN)
// =========================================================
void DrawBuffVFX_DoTimConMoi_Continuous(Vector2 pPos, Vector2 ePos) {
    float timer = GetTime();
    
    // 1. Vẽ Hào Quang Vàng Kim lơ lửng sau lưng Player
    if (texHaoQuangHuyetSac.id != 0) {
        Vector2 playerCenter = { pPos.x + 30.0f, pPos.y - 50.0f };
        float baseScale = 250.0f / (float)texHaoQuangHuyetSac.width;
        float pulse = 1.0f + sinf(timer * 3.0f) * 0.05f; // Đập nhẹ nhàng
        
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(texHaoQuangHuyetSac, 
            (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
            (Rectangle){playerCenter.x, playerCenter.y, texHaoQuangHuyetSac.width * baseScale * pulse, texHaoQuangHuyetSac.height * baseScale * pulse},
            (Vector2){(texHaoQuangHuyetSac.width * baseScale * pulse)/2.0f, (texHaoQuangHuyetSac.height * baseScale * pulse)/2.0f},
            timer * 30.0f, Fade(WHITE, 0.4f)); // Độ mờ 0.4 để không che khuất nhân vật
        EndBlendMode();
    }

    // 2. Vẽ Tâm Ngắm Đỏ nhấp nháy liên tục trên người Boss
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f };
    float crossSize = 50.0f + sinf(timer * 10.0f) * 5.0f; // Nhấp nháy to nhỏ
    DrawSniperCrosshair(targetPos, crossSize, Fade(RED, 0.6f));
}
// =========================================================
// [ACTION VFX] TRIỆT HẠ CON MỒI (W4 - PHÚ NHỊ ĐẠI)
// =========================================================
void DrawSkillVFX_TrietHaConMoi(Vector2 pPos, Vector2 ePos, float timer) {
    if (texBlackHoleVFX.id == 0 || texTrietHa.id == 0) return;

    // Cổng lơ lửng phía sau và cao hơn Player một chút
    Vector2 portalPos = { pPos.x - 40.0f, pPos.y - 80.0f };
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Tim Boss

    // Đổi hố đen sang màu Cam Đỏ rực rỡ để khác biệt với Boss
    Color portalColor = (Color){ 255, 100, 0, 255 }; 

    // ----------------------------------------------------------------
    // 1. CỔNG KHÔNG GIAN (0.3s -> 3.7s)
    // ----------------------------------------------------------------
    if (timer >= 0.3f) {
        float portalScale = 0.0f;
        float portalAlpha = 1.0f;

        if (timer < 0.8f) {
            portalScale = (timer - 0.3f) / 0.5f; // Nở ra
        } else if (timer < 3.2f) {
            portalScale = 1.0f; // Duy trì mở
        } else {
            portalScale = 1.0f - ((timer - 3.2f) / 0.5f); // Thu bé lại
            portalAlpha = portalScale; // Mờ đi
        }

        if (portalScale > 0.0f) {
            float rot = timer * -400.0f; // Xoay ngược chiều kim đồng hồ
            float finalSize = (300.0f / (float)texBlackHoleVFX.width) * portalScale;
            
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texBlackHoleVFX,
                (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                (Rectangle){portalPos.x, portalPos.y, texBlackHoleVFX.width * finalSize, texBlackHoleVFX.height * finalSize},
                (Vector2){(texBlackHoleVFX.width * finalSize)/2.0f, (texBlackHoleVFX.height * finalSize)/2.0f},
                rot, Fade(portalColor, portalAlpha));
            EndBlendMode();
        }
    }

    // ----------------------------------------------------------------
    // 2. MƯA ĐẠN VÀ CHỚP NỔ (1.0s -> 3.4s)
    // ----------------------------------------------------------------
    if (timer >= 1.0f && timer <= 3.4f) { 
        int numBullets = 25; 
        float fireDuration = 2.1f; // Xả từ 1.0s đến 3.1s
        float interval = fireDuration / numBullets;
        float travelTime = 0.15f; // Thời gian bay siêu nhanh

        for (int i = 0; i < numBullets; i++) {
            float bulletStartTime = 1.0f + (i * interval);
            float bulletHitTime = bulletStartTime + travelTime;

            // A. VIÊN ĐẠN ĐANG BAY
            if (timer >= bulletStartTime && timer < bulletHitTime) {
                float p = (timer - bulletStartTime) / travelTime; 
                
                // Thuật toán lan đạn (Spread) tạo độ giật tỏa ra từ nòng
                float spreadY = sinf(i * 13.0f) * 60.0f * (1.0f - p); 
                
                Vector2 currentPos = {
                    Lerp(portalPos.x, targetPos.x, p),
                    Lerp(portalPos.y, targetPos.y, p) + spreadY
                };

                // Góc xoay của viên đạn hướng thẳng vào mặt Boss
                float angle = atan2f(targetPos.y - (portalPos.y + spreadY), targetPos.x - portalPos.x) * RAD2DEG;
                float bulletScale = 150.0f / (float)texTrietHa.width;

                DrawTexturePro(texTrietHa,
                    (Rectangle){0, 0, (float)texTrietHa.width, (float)texTrietHa.height},
                    (Rectangle){currentPos.x, currentPos.y, texTrietHa.width * bulletScale, texTrietHa.height * bulletScale},
                    (Vector2){(texTrietHa.width * bulletScale)/2.0f, (texTrietHa.height * bulletScale)/2.0f},
                    angle, WHITE);
            }
            
            // B. VIÊN ĐẠN CHẠM ĐÍCH (NỔ VÀNG TIA LỬA)
            if (timer >= bulletHitTime && timer < bulletHitTime + 0.15f) { 
                float expP = (timer - bulletHitTime) / 0.15f;
                float expRadius = 40.0f * (1.0f + expP);
                
                // Random nhẹ vị trí nổ trên ngực Boss cho bạo lực
                float hitX = targetPos.x + cosf(i * 7.0f) * 30.0f;
                float hitY = targetPos.y + sinf(i * 11.0f) * 30.0f;

                BeginBlendMode(BLEND_ADDITIVE);
                DrawCircleGradient(hitX, hitY, expRadius, Fade(YELLOW, 1.0f - expP), BLANK);
                DrawCircleGradient(hitX, hitY, expRadius * 0.5f, Fade(WHITE, 1.0f - expP), BLANK);
                EndBlendMode();
            }
        }
    }
}

// =========================================================
// [ACTION VFX] CANH BẠC TẤT TAY (ULTI/R) - BẢN FULL MÀN HÌNH
// =========================================================
void DrawSkillVFX_CanhBacTatTay(Vector2 pPos, Vector2 ePos, float timer) {
    int sw = SCREEN_WIDTH;
    int sh = SCREEN_HEIGHT;
    Vector2 centerScreen = { sw / 2.0f, sh / 2.0f };
    Vector2 playerCenter = { pPos.x + 30.0f, pPos.y - 50.0f };
    Vector2 targetPos = { ePos.x - 30.0f, ePos.y - 50.0f }; // Tâm Boss

    // ----------------------------------------------------------------
    // 1. BÀNH TRƯỚNG LÃNH ĐỊA & CƠN BÃO TIỀN (0.0s -> 3.0s)
    // ----------------------------------------------------------------
    if (timer < 3.0f) {
        float p = timer / 3.0f;
        
        // Vừa vào là TỐI SẦM toàn màn hình ngay lập tức
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.85f));

        BeginBlendMode(BLEND_ADDITIVE);
        
        // A. SIÊU HỐ ĐEN BAO TRÙM BACKGROUND (Kích thước x2 màn hình)
        if (texBlackHoleVFX.id != 0) {
            float bgHoleSize = sw * 2.0f; 
            DrawTexturePro(texBlackHoleVFX,
                (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                (Rectangle){centerScreen.x, centerScreen.y, bgHoleSize, bgHoleSize},
                (Vector2){bgHoleSize/2.0f, bgHoleSize/2.0f},
                timer * 100.0f, Fade((Color){255, 20, 0, 255}, 0.5f)); // Màu Đỏ máu cuộn trào
        }

        // B. HÀO QUANG HIẾN TẾ TỪ PLAYER
        if (texHaoQuangHuyetSac.id != 0) {
            float auraSize = sw * (0.8f + p); // Nở to dần nuốt chửng map
            DrawTexturePro(texHaoQuangHuyetSac,
                (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                (Rectangle){playerCenter.x, playerCenter.y, auraSize, auraSize},
                (Vector2){auraSize/2.0f, auraSize/2.0f},
                timer * -150.0f, Fade(RED, 0.6f));
        }

        // C. CƠN BÃO TIỀN BAY NGƯỢC TRỌNG LỰC LÊN TRỜI
        if (texCoinVFX.id != 0) {
            for (int i = 0; i < 50; i++) {
                // [FIXED] Dùng sin/cos để tạo độ ngẫu nhiên CỐ ĐỊNH, chống giật hình
                float rand1 = fabs(sinf(i * 123.45f));
                float rand2 = fabs(cosf(i * 321.12f));
                float rand3 = fabs(sinf(i * 777.77f));

                float startX = rand1 * sw;
                float startY = sh + 100.0f + (rand2 * 700.0f); // Xuất phát từ dưới đáy
                float speedY = 1000.0f + (rand3 * 1500.0f);    // Tốc độ bay
                float currentY = startY - (speedY * timer); 
                
                if (currentY > -100) { 
                    float cScale = 0.3f + (fabs(cosf(i * 11.11f)) * 0.5f);
                    DrawTexturePro(texCoinVFX,
                        (Rectangle){0, 0, (float)texCoinVFX.width, (float)texCoinVFX.height},
                        (Rectangle){startX, currentY, texCoinVFX.width * cScale, texCoinVFX.height * cScale},
                        (Vector2){(texCoinVFX.width * cScale)/2.0f, (texCoinVFX.height * cScale)/2.0f},
                        timer * 1500.0f + i * 50.0f, Fade(GOLD, 0.9f));
                }
            }
        }

        // D. SẤM SÉT ĐỎ GIẬT LIÊN TỤC KHẮP BẢN ĐỒ
        if (texLightningVFX.id != 0 && GetRandomValue(1, 100) <= 25) { // 25% tỷ lệ giật sấm mỗi frame
            float lX = GetRandomValue(0, sw);
            float lScale = GetRandomValue(300, 600) / (float)texLightningVFX.width;
            DrawTexturePro(texLightningVFX,
                (Rectangle){0, 0, (float)texLightningVFX.width, (float)texLightningVFX.height},
                (Rectangle){lX, centerScreen.y, texLightningVFX.width * lScale, sh * 1.5f}, // Cột sét dọc toàn màn
                (Vector2){(texLightningVFX.width * lScale)/2.0f, (sh * 1.5f)/2.0f}, 
                0.0f, Fade(WHITE, 0.9f));
        }
        EndBlendMode();
        
        // E. KHÓA TÂM NGẮM VÀO BOSS (Siết chặt)
        DrawSniperCrosshair(targetPos, 250.0f - (p * 200.0f), Fade(RED, 0.8f));
    }

    // ----------------------------------------------------------------
    // 2. THIÊN THẠCH VÀNG RƠI XUỐNG (1.0s -> 3.0s)
    // ----------------------------------------------------------------
    if (timer >= 1.0f && timer < 3.0f) {
        float p = (timer - 1.0f) / 2.0f;
        float dropP = p * p * p; // Gia tốc rơi như sao chổi
        
        float startX = targetPos.x; 
        float startY = -600.0f; // Tít trên cao ngoài màn hình
        Vector2 currentPos = { targetPos.x, Lerp(startY, targetPos.y, dropP) };

        BeginBlendMode(BLEND_ADDITIVE);
        // Lửa cháy bùng theo đuôi đồng xu
        if (texHaoQuangHuyetSac.id != 0) {
            DrawTexturePro(texHaoQuangHuyetSac,
                (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                (Rectangle){currentPos.x, currentPos.y - 200.0f, 600.0f, 1500.0f}, // Kéo dãn ảnh ra làm đuôi sao chổi
                (Vector2){300.0f, 1200.0f}, 0.0f, Fade(ORANGE, 0.9f));
        }
        EndBlendMode();

        // Đồng xu khổng lồ (Ép size cực đại)
        if (texCoinVFX.id != 0) {
            float giantScale = 600.0f / (float)texCoinVFX.width; 
            DrawTexturePro(texCoinVFX,
                (Rectangle){0, 0, (float)texCoinVFX.width, (float)texCoinVFX.height},
                (Rectangle){currentPos.x, currentPos.y, texCoinVFX.width * giantScale, texCoinVFX.height * giantScale},
                (Vector2){(texCoinVFX.width * giantScale)/2.0f, (texCoinVFX.height * giantScale)/2.0f},
                timer * 2000.0f, WHITE); 
        }
    }

    // ----------------------------------------------------------------
    // 3. VỤ NỔ ĐỊNH MỆNH (3.0s -> 3.7s) - BẢN NÂNG CẤP MAX SIZE
    // ----------------------------------------------------------------
    if (timer >= 3.0f && timer <= 3.7f) {
        float p = (timer - 3.0f) / 0.7f; // 0.0 -> 1.0
        float fadeOut = 1.0f - p;
        float blastP = (p < 0.2f) ? (p / 0.2f) : 1.0f; 
        float coreFade = (p < 0.3f) ? 1.0f - (p/0.3f) : 0.0f; 

        // Flash mù lòa toàn màn
        if (p < 0.15f) DrawRectangle(0, 0, sw, sh, Fade(WHITE, 1.0f - (p/0.15f)));

        BeginBlendMode(BLEND_ADDITIVE);

        // Lớp khói lửa đỏ/cam cuộn trào
        if (texBlackHoleVFX.id != 0) {
            float smokeSize = 800.0f + (p * 500.0f); // Phóng to cực đại bao cả map
            DrawTexturePro(texBlackHoleVFX,
                (Rectangle){0, 0, (float)texBlackHoleVFX.width, (float)texBlackHoleVFX.height},
                (Rectangle){targetPos.x, targetPos.y, smokeSize, smokeSize},
                (Vector2){smokeSize/2.0f, smokeSize/2.0f},
                timer * 150.0f, Fade((Color){255, 60, 0, 255}, fadeOut * 0.9f));
        }

        // Tâm nổ Glow rực rỡ
        if (texHaoQuangHuyetSac.id != 0) {
            float fireSize = 1000.0f * blastP; // 1000px
            DrawTexturePro(texHaoQuangHuyetSac,
                (Rectangle){0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height},
                (Rectangle){targetPos.x, targetPos.y, fireSize, fireSize},
                (Vector2){fireSize/2.0f, fireSize/2.0f},
                timer * 300.0f, Fade(GOLD, fadeOut));
        }
        
        // Lõi nhiệt trắng
        DrawCircleGradient(targetPos.x, targetPos.y, 350.0f * blastP, Fade(WHITE, coreFade), BLANK);
        DrawCircleGradient(targetPos.x, targetPos.y, 500.0f * blastP, Fade(YELLOW, fadeOut), BLANK);

        // Tàn lửa xé gió tàn phá (60 vệt bay xuyên viền màn hình)
       for (int i = 0; i < 60; i++) {
            // [FIXED] Pseudo-random chống giật đường đạn
            float randAngle = sinf(i * 45.6f) * 10.0f;
            float randSpeed = fabs(cosf(i * 89.1f));

            float angle = (i * 6.0f + randAngle) * DEG2RAD;
            float speed = 2000.0f + (randSpeed * 2500.0f); 
            float sparkDist = speed * p; 
            float sparkLength = speed * 0.08f; 

            Vector2 sparkEnd = { targetPos.x + cosf(angle) * sparkDist, targetPos.y + sinf(angle) * sparkDist };
            Vector2 sparkStart = { targetPos.x + cosf(angle) * (sparkDist - sparkLength), targetPos.y + sinf(angle) * (sparkDist - sparkLength) };
            
            Color sparkColor = (i % 3 == 0) ? WHITE : ((i % 2 == 0) ? YELLOW : ORANGE);
            DrawLineEx(sparkStart, sparkEnd, 3.0f + fabs(sinf(i))*5.0f, Fade(sparkColor, fadeOut));
        }

        // Sóng xung kích 
        DrawRing(targetPos, 1000.0f * p, 1000.0f * p + 30.0f, 0, 360, 60, Fade(WHITE, coreFade));

        EndBlendMode();
    }
}

void Combat_Draw()
{
    if (!combatActive)
        return;

    int sw = SCREEN_WIDTH, sh = SCREEN_HEIGHT, uiY = sh - 180;
    float shakeX = 0, shakeY = 0;
    if (screenShakeTimer > 0)
    {
        shakeX = (float)GetRandomValue(-8, 8);
        shakeY = (float)GetRandomValue(-8, 8);
    }

    DrawTexturePro(texBackground, (Rectangle){0, 0, (float)texBackground.width, (float)texBackground.height},
                   (Rectangle){shakeX, shakeY, (float)sw, (float)uiY}, (Vector2){0, 0}, 0.0f, WHITE);

   Vector2 ePos = {sw - 150 + shakeX, uiY / 2 + 30 + shakeY};
    Vector2 pPos = {150 + shakeX, uiY / 2 + 30 + shakeY};
    float jumpOffset = 0.0f;
    float dashOffset = 0.0f;
    float bossDashOffset = 0.0f; // Biến lướt của Boss
    bool isDotKich = (activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Đột Kích Phân Rã") == 0);
    bool isCuDamSamSet = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Cú Đấm Sấm Sét") == 0);
    bool isRungChan = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Rung Chấn") == 0);
    bool isUltiDauGau = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Lấy Thủ Bù Công") == 0);
    bool isPhanUng = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Phản Ứng Hóa Học") == 0);
    bool isTapTrung = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Tập Trung Cao Độ") == 0);
    bool isCauTuTruong = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Cầu Từ Trường") == 0);
    bool isTuDuy = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Chiến Thuật Tư Duy") == 0);
    bool isDinhLi = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Định Lý Cuối Cùng") == 0);
    bool isHaoQuang = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Hào Quang Huyết Sắc") == 0);
    bool isHuyetTien = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Huyết Tiễn") == 0);
    bool isDauAn = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Dấu Ấn Ký Sinh") == 0);
    bool isGiaoKeo = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Giao Kèo Ác Quỷ") == 0);
    bool isVanTien = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Vạn Tiễn Xuyên Tâm") == 0);
    bool isThuHoiVon = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Thu Hồi Vốn") == 0);
    bool isPhaGia = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Phá Giá Thị Trường") == 0);
    bool isDoTimConMoi = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Dò Tìm Con Mồi") == 0);
    bool isTrietHa = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Triệt Hạ Con Mồi") == 0);
    bool isCanhBac = (activeAttacker == &entPlayer && activeSkill != NULL && strcmp(activeSkill->name, u8"Canh Bạc Tất Tay") == 0);
    // Biến cờ: Chỉ cho phép hoạt ảnh chạy khi state là ACTION và Sách đã gập hẳn (IDLE)
    bool isActionPlaying = (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE); 

    if (isActionPlaying) {
        if (isCuDamSamSet) {
            float targetDash = ePos.x - pPos.x - 120.0f;
            if (stateTimer >= 0.5f && stateTimer <= 1.2f) {
                float p = (stateTimer - 0.5f) / 0.7f;
                dashOffset = targetDash * (1.0f - powf(1.0f - p, 3.0f));
            } else if (stateTimer > 1.2f && stateTimer <= 3.0f) {
                dashOffset = targetDash;
            } else if (stateTimer > 3.0f && stateTimer <= 3.7f) {
                float p = (stateTimer - 3.0f) / 0.7f;
                dashOffset = targetDash * (1.0f - p);
            }
        } 
        else if (isRungChan) {
            float midX = (float)sw / 2.0f;
            float targetDash = midX - pPos.x; 
            if (stateTimer <= 1.2f) {
                float p = stateTimer / 1.2f;
                dashOffset = targetDash * p;
                jumpOffset = sinf(p * PI) * 150.0f;
            } else if (stateTimer <= 3.0f) {
                dashOffset = targetDash;
                jumpOffset = 0;
            } else if (stateTimer > 3.0f) {
                float p = (stateTimer - 3.0f) / 0.7f;
                dashOffset = targetDash * (1.0f - p);
            }
        }
        else if (isDotKich) {
            // Boss đứng bên phải, Player bên trái. Nên khoảng cách bay (targetDash) sẽ là một số ÂM.
            float targetDash = pPos.x - ePos.x + 180.0f; // +180.0f để Boss dừng lại ngay trước mặt Player chứ không đè lên người
            
            if (stateTimer < 1.0f) {
                // Gia tốc siêu nhanh lướt tới
                float p = stateTimer / 1.0f;
                bossDashOffset = targetDash * (p * p * p); 
            } else if (stateTimer <= 3.2f) {
                // Đứng im tại đó tụ lực đấm
                bossDashOffset = targetDash;
            } else if (stateTimer <= 3.7f) {
                // Lùi về lại vị trí cũ cực nhanh
                float p = (stateTimer - 3.2f) / 0.5f;
                bossDashOffset = targetDash * (1.0f - p);
            }
        }
        else if (isTrietHa) {
            float targetDash = -40.0f; // Lùi về sau 40px
            if (stateTimer <= 0.3f) {
                dashOffset = targetDash * (stateTimer / 0.3f);
            } else if (stateTimer <= 3.2f) {
                dashOffset = targetDash;
            } else if (stateTimer <= 3.7f) {
                dashOffset = targetDash * (1.0f - ((stateTimer - 3.2f) / 0.5f));
            }
        }
        else if (isVanTien) {
            float targetJump = 200.0f; // Bay bổng lên cao
            float targetDash = -150.0f; // Lùi về sát lề trái
            if (stateTimer <= 1.0f) {
                float p = stateTimer / 1.0f;
                // Bay lên cong cong cho đẹp (Easing)
                jumpOffset = targetJump * (1.0f - cosf(p * PI / 2.0f));
                dashOffset = targetDash * (1.0f - cosf(p * PI / 2.0f));
            } else if (stateTimer <= 3.0f) {
                // Treo lơ lửng trên không trung xả đạn
                jumpOffset = targetJump + sinf(stateTimer * 10.0f) * 10.0f; 
                dashOffset = targetDash;
            } else if (stateTimer <= 3.7f) {
                // Đáp đất
                float p = (stateTimer - 3.0f) / 0.7f;
                jumpOffset = targetJump * (1.0f - p);
                dashOffset = targetDash * (1.0f - p);
            }
        }
        else if (isThuHoiVon) {
            float targetDash = (SCREEN_WIDTH / 2.0f) - pPos.x - 30.0f; // Lao ra giữa map
            
            if (stateTimer <= 0.7f) { 
                // Lao ra cực gắt (Dùng EaseOutCubic cho cảm giác phanh gấp)
                float p = stateTimer / 0.7f;
                dashOffset = targetDash * (1.0f - powf(1.0f - p, 3.0f)); 
            } 
            else if (stateTimer <= 3.0f) { 
                // Đứng tụ lực: Ép nhân vật RUNG BẦN BẬT (cực kỳ bạo lực)
                dashOffset = targetDash + sinf(stateTimer * 50.0f) * 3.0f; 
                jumpOffset = sinf(stateTimer * 40.0f) * 2.0f;
            } 
            else if (stateTimer <= 3.7f) { 
                // Thu tiền xong lùi cái vèo về chỗ cũ
                float p = (stateTimer - 3.0f) / 0.7f;
                dashOffset = targetDash * (1.0f - powf(p, 3.0f)); // Lùi nhanh
                jumpOffset = 0.0f;
            }
        }
    }

    bool isHocBa = (activeAttacker == &entPlayer && (refPlayer->pClass == CLASS_HOC_BA || refPlayer->pClass == CLASS_WARRIOR) && activeSkill == &entPlayer.skills[0]);
    bool isDauGau = (activeAttacker == &entPlayer && (refPlayer->pClass == CLASS_DAU_GAU || refPlayer->pClass == CLASS_STUDENT) && activeSkill == &entPlayer.skills[0]);

    if (combatState == COMBAT_STATE_ACTION && isHocBa)
    {
        if (stateTimer < 3.0f)
        {
            float jumpProgress = stateTimer / 3.0f;
            jumpOffset = sin(jumpProgress * PI) * 60.0f;
        }
    }
    bool isLazerHuyDiet = (activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Lazer Hủy Diệt") == 0);
    bool isDichChuyen = (activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Dịch Chuyển Thời Không") == 0);
    bool isPhaoHoDen = (activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Pháo Hố Đen") == 0);

    bool isNghichLy = (activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Nghịch Lý Hủy Diệt") == 0);
    Vector2 currentEPos = { ePos.x + bossDashOffset, ePos.y };

    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE) {
        
        // Ép rung màn hình liên tục khi tụ chiêu Nghịch Lý (Giai đoạn 2.0s -> 3.0s)
        if (isNghichLy && stateTimer > 2.0f && stateTimer < 3.0f) {
            screenShakeTimer = 0.1f; 
        }

        if (isLazerHuyDiet) DrawSkillVFX_LaserHuyDiet(ePos, pPos, stateTimer);
        if (isDichChuyen) DrawSkillVFX_DichChuyen(pPos, stateTimer);
        if (isPhaoHoDen) DrawSkillVFX_PhaoHoDen(pPos, stateTimer);
        if (isDotKich) DrawSkillVFX_DotKichPhanRa(currentEPos, pPos, stateTimer); 
        
        // --- [GỌI HÀM VẼ CHIÊU CUỐI TOÀN MÀN HÌNH] ---
        if (isNghichLy) DrawSkillVFX_NghichLyHuyDiet(stateTimer, sw, sh);
    }
   
    bool isPhuNhiDai = (activeAttacker == &entPlayer && (refPlayer->pClass == CLASS_PHU_NHI_DAI || refPlayer->pClass == CLASS_ARCHER) && activeSkill == &entPlayer.skills[0]);

    if (combatState == COMBAT_STATE_ACTION && isPhuNhiDai)
    {
        if (stateTimer <= 0.5f)
        {
            float p = stateTimer / 0.5f;
            jumpOffset = sin(p * PI / 2.0f) * 80.0f;
        }
        else if (stateTimer <= 3.2f)
        {
            jumpOffset = 80.0f + sin(stateTimer * 5.0f) * 5.0f;
        }
        else
        {
            float p = (stateTimer - 3.2f) / 0.5f;
            jumpOffset = 80.0f * (1.0f - p);
        }
    }

   if (jumpOffset > 0 || dashOffset > 0) 
    {
        DrawEllipse(pPos.x + dashOffset, pPos.y + (refPlayer->spriteHeight * 2.5f) / 2.0f,
                    30 - (jumpOffset / 4.0f), 10 - (jumpOffset / 10.0f), Fade(BLACK, 0.6f));
    }
    //vẽ buff
     for (int i = 0; i < MAX_EFFECTS; i++) {
        // NẾU CÓ BUFF CHÍ MẠNG & LÀ CLASS PHÚ NHỊ ĐẠI -> Kích hoạt Hào quang & Tâm ngắm
        if (entPlayer.effects[i].type == EFFECT_CRIT_UP && entPlayer.effects[i].duration > 0) {
            if (refPlayer->pClass == CLASS_PHU_NHI_DAI || refPlayer->pClass == CLASS_ARCHER) {
                // Vẽ hiệu ứng liên tục, currentEPos là tọa độ thực tế của Boss
                DrawBuffVFX_DoTimConMoi_Continuous((Vector2){pPos.x, pPos.y - jumpOffset}, currentEPos);
            }
        }
        if (entPlayer.effects[i].type == EFFECT_DEF_UP && entPlayer.effects[i].duration > 0) 
            DrawBuffVFX_DefUp((Vector2){pPos.x + 30.0f, pPos.y - jumpOffset}, 1.0f);
        if (entPlayer.effects[i].type == EFFECT_ATK_UP && entPlayer.effects[i].duration > 0) 
            DrawBuffVFX_AtkUp((Vector2){pPos.x + 30.0f, pPos.y - jumpOffset}, 1.0f);
        if (entPlayer.effects[i].type == EFFECT_STUN && entPlayer.effects[i].duration > 0)
            DrawBuffVFX_Stun((Vector2){pPos.x + 30.0f, pPos.y - jumpOffset});
        if (entPlayer.effects[i].type == EFFECT_HEAL && entPlayer.effects[i].duration > 0) {
            // NẾU LÀ SOÁI CA / MAGE -> Bật Hào Quang Huyết Sắc duy trì
            if (refPlayer->pClass == CLASS_SOAI_CA || refPlayer->pClass == CLASS_MAGE) {
                // Vẽ ngang ngực (trừ đi 60)
                DrawBuffVFX_HaoQuangHuyetSac_Continuous((Vector2){pPos.x +0.0f, pPos.y - 20.0f - jumpOffset});
            } else {
                // CLASS KHÁC -> Bật dấu cộng xanh lá mặc định
                DrawBuffVFX_Heal_Universal((Vector2){pPos.x + 30.0f, pPos.y - jumpOffset});
            }
        }
        // GIAO KÈO ÁC QUỶ: Cấp EFFECT_SPD_UP -> Vẽ Aura lửa bám người
        if (entPlayer.effects[i].type == EFFECT_SPD_UP && entPlayer.effects[i].duration > 0) {
            if (refPlayer->pClass == CLASS_SOAI_CA || refPlayer->pClass == CLASS_MAGE) {
                DrawBuffVFX_GiaoKeoAcQuy_Continuous((Vector2){pPos.x + 10.0f, pPos.y - 20.0f - jumpOffset});
            }
        }
    }

    int pFrame = (int)(GetTime() * 8.0f) % refPlayer->maxFrames;
    DrawTexturePro(*entPlayer.texture,
                   (Rectangle){pFrame * refPlayer->spriteWidth, 1 * refPlayer->spriteHeight, (float)refPlayer->spriteWidth, (float)refPlayer->spriteHeight},
                   (Rectangle){pPos.x + dashOffset, pPos.y - jumpOffset, refPlayer->spriteWidth * 2.5f, refPlayer->spriteHeight * 2.5f},
                   (Vector2){(refPlayer->spriteWidth * 2.5f) / 2, (refPlayer->spriteHeight * 2.5f) / 2}, 0.0f, WHITE);
    // --- GỌI HÀM VẼ GIÁP GAI NẾU ĐANG CÓ BUFF ---
    for (int i = 0; i < MAX_EFFECTS; i++)
    {
        if (entPlayer.effects[i].type == EFFECT_THORN_ARMOR && entPlayer.effects[i].duration > 0)
        {
            DrawSkillVFX_GiapGai((Vector2){pPos.x, pPos.y - jumpOffset});
            break;
        }
    }
   

    if (currentPhase == 1 && phase1TexLoaded)
    {
        int bvFrame = (int)(GetTime() * 5.0f) % 4;
        float bv1W = texPhase1BV1.width / 4.0f, bv2W = texPhase1BV2.width / 4.0f, trW = texPhase1Trung.width / 4.0f;
        DrawTexturePro(texPhase1BV1, (Rectangle){bvFrame * bv1W, 0, -bv1W, (float)texPhase1BV1.height}, (Rectangle){ePos.x - 80, ePos.y + 20, bv1W * 2.5f, texPhase1BV1.height * 2.5f}, (Vector2){(bv1W * 2.5f) / 2, (texPhase1BV1.height * 2.5f) / 2}, 0, WHITE);
        DrawTexturePro(texPhase1BV2, (Rectangle){bvFrame * bv2W, 0, -bv2W, (float)texPhase1BV2.height}, (Rectangle){ePos.x + 80, ePos.y + 20, bv2W * 2.5f, texPhase1BV2.height * 2.5f}, (Vector2){(bv2W * 2.5f) / 2, (texPhase1BV2.height * 2.5f) / 2}, 0, WHITE);
        DrawTexturePro(texPhase1Trung, (Rectangle){bvFrame * trW, 0, -trW, (float)texPhase1Trung.height}, (Rectangle){ePos.x, ePos.y - 10, trW * 2.8f, texPhase1Trung.height * 2.8f}, (Vector2){(trW * 2.8f) / 2, (texPhase1Trung.height * 2.8f) / 2}, 0, WHITE);
    }
    else
   {
        int eFrame = (int)(GetTime() * (1.0f / refBoss->frameSpeed)) % refBoss->frameCount;
        float enemyW = (float)entBoss.texture->width / refBoss->frameCount;
        float scale = (currentPhase == 2) ? 3.0f : 2.5f;
        Vector2 currentEPos = { ePos.x + bossDashOffset, ePos.y };
       // Sửa ePos.x, ePos.y thành currentEPos.x, currentEPos.y
        Rectangle destE = {currentEPos.x, currentEPos.y, enemyW * scale, entBoss.texture->height * scale};

        // =========================================================
        // [VFX] VẼ AURA CHO QUỐC TRUNG PHASE 2
        // =========================================================
        if (currentPhase == 2 && texPhase2Aura.id != 0) {
            bool isSkill2Active = (combatState == COMBAT_STATE_ACTION && activeAttacker == &entBoss && activeSkill != NULL && strcmp(activeSkill->name, u8"Quá Tải Huyết Thanh") == 0);
            
            float baseAuraScale = 450.0f / 1024.0f; 
            float currentAuraScale = baseAuraScale;
            float auraRot = GetTime() * 40.0f; 
            Color auraTint = WHITE; 

            if (isSkill2Active) {
                // ... (Logic giai đoạn 1 và 2 của Aura giữ nguyên) ...
                if (stateTimer < 1.5f) {
                    float p = stateTimer / 1.5f;
                    currentAuraScale = baseAuraScale * (1.0f - p * 0.4f);
                    auraRot = GetTime() * (40.0f + p * 500.0f);
                    auraTint = (Color){255, (unsigned char)(255 * (1.0f - p)), (unsigned char)(255 * (1.0f - p)), 255}; 
                } else if (stateTimer < 3.0f) {
                    float p = (stateTimer - 1.5f) / 1.5f;
                    currentAuraScale = baseAuraScale * (0.6f + p * 3.0f);
                    auraRot = GetTime() * 540.0f;
                    auraTint = Fade(RED, 1.0f - p);
                }
            } else {
                currentAuraScale = baseAuraScale * (1.0f + sinf(GetTime() * 3.0f) * 0.08f);
            }

            // === [SỬA TỌA ĐỘ VẼ AURA THÀNH currentEPos] ===
            Rectangle destAura = {currentEPos.x, currentEPos.y, 1024.0f * currentAuraScale, 1024.0f * currentAuraScale};
            Vector2 originAura = {destAura.width / 2.0f, destAura.height / 2.0f};

            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(texPhase2Aura, (Rectangle){0, 0, 1024, 1024}, destAura, originAura, -auraRot * 0.5f, Fade(auraTint, 0.6f));
            DrawTexturePro(texPhase2Aura, (Rectangle){0, 0, 1024, 1024}, destAura, originAura, auraRot, auraTint);
            EndBlendMode();
        }

        // Vẽ hiệu ứng bóng ma giật giật (Afterimage) của Phase 2
        if (currentPhase == 2) {
            // Bóng ma cũng phải bay theo dòng currentEPos
            DrawTexturePro(*entBoss.texture, (Rectangle){eFrame * enemyW, 0, -enemyW, (float)entBoss.texture->height}, 
                           (Rectangle){currentEPos.x + sin(GetTime() * 20.0f) * 5.0f, currentEPos.y, destE.width, destE.height}, 
                           (Vector2){destE.width / 2, destE.height / 2}, 0.0f, Fade(RED, 0.7f));
        }
        
        // Vẽ Boss thật đè lên Aura
        DrawTexturePro(*entBoss.texture, (Rectangle){eFrame * enemyW, 0, -enemyW, (float)entBoss.texture->height}, destE, (Vector2){destE.width / 2, destE.height / 2}, 0.0f, WHITE);
    }
    for (int i = 0; i < MAX_EFFECTS; i++) {
        if (entBoss.effects[i].type == EFFECT_DEF_UP && entBoss.effects[i].duration > 0) 
            DrawBuffVFX_DefUp(currentEPos, 1.5f); // Scale 1.5 vì Boss to hơn
        if (entBoss.effects[i].type == EFFECT_ATK_UP && entBoss.effects[i].duration > 0) 
            DrawBuffVFX_AtkUp(currentEPos, 1.5f);
        if (entBoss.effects[i].type == EFFECT_STUN && entBoss.effects[i].duration > 0)
            DrawBuffVFX_Stun(currentEPos);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && activeAttacker == &entPlayer && activeSkill != NULL  && strcmp(activeSkill->name, u8"Kẻ Chịu Đòn") == 0) {
        DrawSkillVFX_KeChiuDon(pPos, stateTimer, jumpOffset);
    }
    // --- GỌI HOẠT ẢNH CÚ ĐẤM SẤM SÉT ---
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isCuDamSamSet) {
        DrawSkillVFX_CuDamSamSet(pPos, ePos, stateTimer, jumpOffset, dashOffset);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isRungChan) {
        DrawSkillVFX_RungChan(pPos, stateTimer, sw, sh);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isUltiDauGau) {
        DrawSkillVFX_UltiDauGau(ePos, stateTimer, sw, sh);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isPhanUng) {
        DrawSkillVFX_PhanUngHoaHoc(pPos, ePos, stateTimer, sw, sh);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isTapTrung) {
        DrawSkillVFX_TapTrungCaoDo(pPos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isCauTuTruong) {
        DrawSkillVFX_CauTuTruong(pPos, ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isTuDuy) {
        DrawSkillVFX_ChienThuatTuDuy(pPos, stateTimer, &entPlayer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isDinhLi) {
        DrawSkillVFX_DinhLiCuoiCung(pPos, ePos, stateTimer, &entPlayer, &entBoss);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isHaoQuang) {
        DrawSkillVFX_HaoQuangHuyetSac(pPos, stateTimer);
    }
    // --- [MỚI CHÈN] KÍCH HOẠT HOẠT ẢNH W1 PHÚ NHỊ ĐẠI ---
    
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isThuHoiVon) {
        DrawSkillVFX_ThuHoiVon(pPos, ePos, stateTimer); // Trả lại tên cho em nó
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isPhaGia) {
        DrawSkillVFX_PhaGiaThiTruong(ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isHuyetTien) {
        DrawSkillVFX_HuyetTien(pPos, ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isDauAn) {
        DrawSkillVFX_DauAnKiSinh(ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isGiaoKeo) {
        DrawSkillVFX_GiaoKeoAcQuy(pPos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isDoTimConMoi) {
        DrawSkillVFX_DoTimConMoi(pPos, currentEPos, stateTimer, sw, sh);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isTrietHa) {
        DrawSkillVFX_TrietHaConMoi(pPos, ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isCanhBac) {
        DrawSkillVFX_CanhBacTatTay(pPos, ePos, stateTimer);
    }
    if (combatState == COMBAT_STATE_ACTION && cb_bookState == CB_BOOK_IDLE && isVanTien) {
        DrawSkillVFX_VanTienXuyenTam(pPos, ePos, stateTimer);
    }
    

    if (combatState == COMBAT_STATE_ACTION && isHocBa)
    {
        if (stateTimer <= 3.0f)
        {
            float p = stateTimer / 3.0f;
            Vector2 startVFX = {pPos.x + 50, pPos.y - jumpOffset - 30};
            Vector2 endVFX = {ePos.x - 40, ePos.y};
            Vector2 currentVFXPos;

            float baseScale = 130.0f / (float)texBookVFX.width;
            float currentScale = 0.0f;

            if (p < 0.5f)
            {
                currentVFXPos = startVFX;
                currentScale = baseScale * (p / 0.5f);
            }
            else
            {
                float travelP = (p - 0.5f) / 0.5f;
                currentVFXPos.x = startVFX.x + (endVFX.x - startVFX.x) * travelP;
                currentVFXPos.y = startVFX.y + (endVFX.y - startVFX.y) * travelP;
                currentScale = baseScale * 3.0f;
            }
            float rotSpeed = stateTimer * 800.0f;
            if (texBookVFX.id != 0)
                DrawTexturePro(texBookVFX, (Rectangle){0, 0, texBookVFX.width, texBookVFX.height}, (Rectangle){currentVFXPos.x, currentVFXPos.y, texBookVFX.width * currentScale, texBookVFX.height * currentScale}, (Vector2){(texBookVFX.width * currentScale) / 2.0f, (texBookVFX.height * currentScale) / 2.0f}, rotSpeed, WHITE);
        }
        else if (stateTimer > 3.0f && stateTimer <= 3.7f)
        {
            float p = (stateTimer - 3.0f) / 0.7f;
            float baseExpScale = 300.0f / (float)texExplosionVFX.width;
            float expScale = baseExpScale * (1.0f + (p * 2.0f));
            float expAlpha = 1.0f - p;
            if (texExplosionVFX.id != 0)
                DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, texExplosionVFX.width, texExplosionVFX.height}, (Rectangle){ePos.x, ePos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale}, (Vector2){(texExplosionVFX.width * expScale) / 2.0f, (texExplosionVFX.height * expScale) / 2.0f}, GetTime() * 30.0f, Fade(WHITE, expAlpha));
        }
    }

    if (combatState == COMBAT_STATE_ACTION && isDauGau)
    {
        if (stateTimer <= 3.0f)
        {
            float p = stateTimer / 3.0f;
            Vector2 startVFX = {pPos.x + 50, pPos.y - 10};
            Vector2 endVFX = {ePos.x - 40, ePos.y};

            float travelP = p * p * p * p * p * p;
            Vector2 currentVFXPos = {
                startVFX.x + (endVFX.x - startVFX.x) * travelP,
                startVFX.y + (endVFX.y - startVFX.y) * travelP};

            float baseScale = 80.0f / (float)texPunchVFX.width;
            float currentScale = baseScale * (1.0f + p * 1.5f);

            if (texPunchVFX.id != 0)
            {
                DrawTexturePro(texPunchVFX,
                               (Rectangle){0, 0, texPunchVFX.width, texPunchVFX.height},
                               (Rectangle){currentVFXPos.x, currentVFXPos.y, texPunchVFX.width * currentScale, texPunchVFX.height * currentScale},
                               (Vector2){(texPunchVFX.width * currentScale) / 2.0f, (texPunchVFX.height * currentScale) / 2.0f},
                               0.0f, WHITE);
            }
        }
        else if (stateTimer > 3.0f && stateTimer <= 3.7f)
        {
            float t = (stateTimer - 3.0f) / 0.7f;

            float baseExpScale = 300.0f / (float)texExplosionVFX.width;
            float expScale = baseExpScale * (1.0f + (t * 2.0f));
            float expAlpha = 1.0f - t;

            if (texExplosionVFX.id != 0)
            {
                DrawTexturePro(texExplosionVFX, (Rectangle){0, 0, texExplosionVFX.width, texExplosionVFX.height},
                               (Rectangle){ePos.x, ePos.y, texExplosionVFX.width * expScale, texExplosionVFX.height * expScale},
                               (Vector2){(texExplosionVFX.width * expScale) / 2.0f, (texExplosionVFX.height * expScale) / 2.0f},
                               GetTime() * 30.0f, Fade(WHITE, expAlpha));
            }
        }
    }

    bool isSoaiCa = (activeAttacker == &entPlayer && (refPlayer->pClass == CLASS_SOAI_CA || refPlayer->pClass == CLASS_MAGE) && activeSkill == &entPlayer.skills[0]);

    if (combatState == COMBAT_STATE_ACTION && isSoaiCa)
    {
        Vector2 bowPos = {pPos.x + 90, pPos.y - 10};
        float bowScale = 0.4f;

        float bowAlpha = 0.0f;
        if (stateTimer <= 1.0f)
            bowAlpha = stateTimer;
        else if (stateTimer <= 3.0f)
            bowAlpha = 1.0f;
        else
            bowAlpha = 1.0f - ((stateTimer - 3.0f) / 0.7f);

        if (texBowVFX.id != 0 && bowAlpha > 0.0f)
        {
            DrawTexturePro(texBowVFX, (Rectangle){0, 0, -texBowVFX.width, texBowVFX.height},
                           (Rectangle){bowPos.x, bowPos.y, texBowVFX.width * bowScale, texBowVFX.height * bowScale},
                           (Vector2){(texBowVFX.width * bowScale) / 2.0f, (texBowVFX.height * bowScale) / 2.0f}, 0.0f, Fade(WHITE, bowAlpha));
        }

        if (stateTimer > 1.0f && stateTimer <= 3.0f)
        {
            float arrowAlpha = 1.0f;
            float pullOffset = 0.0f;
            float arrowX = bowPos.x;
            float arrowY = bowPos.y;

            if (stateTimer <= 1.5f)
            {
                arrowAlpha = (stateTimer - 1.0f) / 0.5f;
            }
            else if (stateTimer <= 2.8f)
            {
                pullOffset = ((stateTimer - 1.5f) / 1.3f) * 30.0f;
                arrowX = bowPos.x - pullOffset;
            }
            else
            {
                float flyProgress = (stateTimer - 2.8f) / 0.2f;
                Vector2 startFly = {bowPos.x - 30.0f, bowPos.y};
                Vector2 endFly = {ePos.x - 40.0f, ePos.y};
                arrowX = startFly.x + (endFly.x - startFly.x) * flyProgress;
                arrowY = startFly.y + (endFly.y - startFly.y) * flyProgress;
                pullOffset = 0.0f;
            }

            Vector2 bowTop = {bowPos.x - 5, bowPos.y - 60.0f * bowScale};
            Vector2 bowBottom = {bowPos.x - 5, bowPos.y + 60.0f * bowScale};
            Vector2 stringNock = {arrowX - 20.0f, arrowY};

            DrawLineEx(bowTop, stringNock, 2.0f, Fade(SKYBLUE, bowAlpha));
            DrawLineEx(bowBottom, stringNock, 2.0f, Fade(SKYBLUE, bowAlpha));

            float arrowScale = 0.2f;
            if (texArrowVFX.id != 0)
            {
                DrawTexturePro(texArrowVFX, (Rectangle){0, 0, texArrowVFX.width, texArrowVFX.height},
                               (Rectangle){arrowX, arrowY, texArrowVFX.width * arrowScale, texArrowVFX.height * arrowScale},
                               (Vector2){(texArrowVFX.width * arrowScale) / 2.0f, (texArrowVFX.height * arrowScale) / 2.0f}, 0.0f, Fade(WHITE, arrowAlpha));
            }
        }

        if (stateTimer > 3.0f && stateTimer <= 3.7f)
        {
            float expProgress = (stateTimer - 3.0f) / 0.7f;

            float radius = 20.0f + expProgress * 130.0f;
            float fadeOut = 1.0f - expProgress;

            Color pinkCore = (Color){255, 105, 180, 255};
            Color pinkEdge = (Color){255, 20, 147, 0};

            DrawCircleGradient(ePos.x, ePos.y, radius, Fade(pinkCore, fadeOut), pinkEdge);
            DrawCircleGradient(ePos.x, ePos.y, radius * 0.5f, Fade(WHITE, fadeOut), pinkEdge);
        }
    }

    if (combatState == COMBAT_STATE_ACTION && isPhuNhiDai)
    {

        Vector2 coinStartPos = {pPos.x + 60.0f, pPos.y - jumpOffset - 10.0f};
        Vector2 coinEndPos = {ePos.x - 40.0f, ePos.y};

        if (stateTimer <= 3.0f)
        {
            float coinAlpha = (stateTimer < 0.3f) ? (stateTimer / 0.3f) : 1.0f;
            float coinScale = 0.5f;
            float rotSpeed = stateTimer * 1500.0f;

            Vector2 currentCoinPos = coinStartPos;

            if (stateTimer > 1.5f)
            {
                float p = (stateTimer - 1.5f) / 1.5f;
                float travelP = p * p * p;

                currentCoinPos.x = coinStartPos.x + (coinEndPos.x - coinStartPos.x) * travelP;
                currentCoinPos.y = coinStartPos.y + (coinEndPos.y - coinStartPos.y) * travelP;
            }

            if (texCoinVFX.id != 0)
            {
                DrawTexturePro(texCoinVFX, (Rectangle){0, 0, texCoinVFX.width, texCoinVFX.height},
                               (Rectangle){currentCoinPos.x, currentCoinPos.y, texCoinVFX.width * coinScale, texCoinVFX.height * coinScale},
                               (Vector2){(texCoinVFX.width * coinScale) / 2.0f, (texCoinVFX.height * coinScale) / 2.0f}, rotSpeed, Fade(WHITE, coinAlpha));
            }
        }
        else if (stateTimer > 3.0f && stateTimer <= 3.7f)
        {
            float p = (stateTimer - 3.0f) / 0.7f;

            if (p < 0.2f)
            {
                float boomScale = 0.5f + (p / 0.2f) * 3.0f;
                if (texCoinVFX.id != 0)
                {
                    DrawTexturePro(texCoinVFX, (Rectangle){0, 0, texCoinVFX.width, texCoinVFX.height},
                                   (Rectangle){coinEndPos.x, coinEndPos.y, texCoinVFX.width * boomScale, texCoinVFX.height * boomScale},
                                   (Vector2){(texCoinVFX.width * boomScale) / 2.0f, (texCoinVFX.height * boomScale) / 2.0f}, stateTimer * 1500.0f, WHITE);
                }
            }

            float radius = 50.0f + p * 250.0f;
            float fadeOut = 1.0f - p;

            Color goldColor = (Color){255, 215, 0, 255};
            Color whiteColor = (Color){255, 255, 255, 255};
            Color transparent = (Color){255, 215, 0, 0};

            DrawCircleGradient(coinEndPos.x, coinEndPos.y, radius * 0.5f, Fade(whiteColor, fadeOut), transparent);
            DrawCircleGradient(coinEndPos.x, coinEndPos.y, radius, Fade(goldColor, fadeOut * 0.8f), transparent);

            DrawRectanglePro((Rectangle){coinEndPos.x, coinEndPos.y, radius * 2.5f, 15.0f}, (Vector2){radius * 1.25f, 7.5f}, 45.0f, Fade(whiteColor, fadeOut));
            DrawRectanglePro((Rectangle){coinEndPos.x, coinEndPos.y, radius * 2.5f, 15.0f}, (Vector2){radius * 1.25f, 7.5f}, -45.0f, Fade(goldColor, fadeOut));
        }
    }

    float hpPctB = (float)entBoss.hp / entBoss.maxHp;
    DrawRectangleRounded((Rectangle){ePos.x - 70, ePos.y - 120, 140, 15}, 0.5f, 5, Fade(RED, 0.8f));
    DrawRectangleRounded((Rectangle){ePos.x - 70, ePos.y - 120, 140 * hpPctB, 15}, 0.5f, 5, GREEN);
    DrawTextEx(globalFont, entBoss.name, (Vector2){ePos.x - MeasureTextEx(globalFont, entBoss.name, 22, 1).x / 2, ePos.y - 145}, 22, 1, WHITE);

    int barX = sw - 240 , barY = 20;//chỉnh vị trí thanh hành động
    DrawRectangleRounded((Rectangle){barX, barY, 220, 30}, 0.5f, 10, Fade(BLACK, 0.7f));
    DrawRectangleRoundedLines((Rectangle){barX, barY, 220, 30}, 0.5f, 10, WHITE);
    DrawTextEx(globalFont, "ACTION BAR", (Vector2){barX + 10, barY - 20}, 18, 1, YELLOW);
    float pLineX = barX + (entPlayer.actionValue / 200.0f) * 200.0f;
    if (pLineX > barX + 200)
        pLineX = barX + 200;
    DrawCircle(pLineX, barY + 15, 12, SKYBLUE);
    DrawTextEx(globalFont, "P", (Vector2){pLineX - 6, barY + 4}, 18, 1, WHITE);
    float bLineX = barX + (entBoss.actionValue / 200.0f) * 200.0f;
    if (bLineX > barX + 200)
        bLineX = barX + 200;
    DrawCircle(bLineX, barY + 15, 12, RED);
    DrawTextEx(globalFont, "B", (Vector2){bLineX - 6, barY + 4}, 18, 1, WHITE);

    if (strlen(messageLog) > 0)
    {
        DrawRectangleRounded((Rectangle){10, 10, MeasureTextEx(globalFont, messageLog, 18, 1).x + 20, 30}, 0.2f, 10, Fade(BLACK, 0.7f));
        DrawTextEx(globalFont, messageLog, (Vector2){20, 15}, 18, 1, WHITE);
    }
    // --- [NÂNG CẤP] VẼ BẢNG LỊCH SỬ LOG LỚN KHI BẤM X ---
    if (showLogWindow) {
        int panelW = 700;
        int panelH = 500;
        int panelX = (sw - panelW) / 2;
        int panelY = (sh - panelH) / 2 - 40; 
        
        // Vẽ lớp filter tối màn hình đằng sau
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f)); 
        
        // Khung UI
        DrawRectangleRounded((Rectangle){panelX, panelY, panelW, panelH}, 0.05f, 10, Fade((Color){20, 25, 30, 255}, 0.95f));
        DrawRectangleRoundedLines((Rectangle){panelX, panelY, panelW, panelH}, 0.05f, 10, GOLD);
        
        // Tiêu đề
        DrawTextEx(globalFont, u8"LỊCH SỬ CHIẾN ĐẤU (Bấm X để đóng)", (Vector2){panelX + 25, panelY + 20}, 24, 1, YELLOW);
        DrawLine(panelX + 20, panelY + 55, panelX + panelW - 20, panelY + 55, GRAY);
        
        // Tính toán hiển thị (Chỉ hiện tối đa 14 dòng mới nhất để không bị tràn khung)
        int maxDisplay = 14; 
        int startIdx = (logCount > maxDisplay) ? logCount - maxDisplay : 0;
        
        float textY = panelY + 70;
        for (int i = startIdx; i < logCount; i++) {
            // Dòng chữ mới nhất sẽ có màu Xanh lục để dễ phân biệt, dòng cũ màu Trắng
            Color textColor = (i == logCount - 1) ? LIME : WHITE;
            DrawTextEx(globalFont, logHistory[i], (Vector2){panelX + 25, textY}, 20, 1, textColor);
            textY += 30; // Khoảng cách giãn dòng
        }
    }

    DrawRectangle(0, uiY, sw, 180, Fade(DARKGRAY, 0.95f));
    DrawRectangleLinesEx((Rectangle){0, uiY, sw, 180}, 4.0f, BLACK);
    DrawTextEx(globalFont, u8"BẠN", (Vector2){40, uiY + 20}, 24, 1, WHITE);

    int hpFrame = (int)(((float)entPlayer.hp / entPlayer.maxHp) * 9.0f);
    if (hpFrame >= 9)
        hpFrame = 8;
    if (entPlayer.hp <= 0)
        hpFrame = 0;
    DrawTexturePro(texCombatUI, SRC_HEART[hpFrame], (Rectangle){40, uiY + 50, SRC_HEART[hpFrame].width * 2.5f, SRC_HEART[hpFrame].height * 2.5f}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTextEx(globalFont, TextFormat("%d/%d", entPlayer.hp, entPlayer.maxHp), (Vector2){40 + SRC_HEART[hpFrame].width * 2.5f + 15, uiY + 50 + SRC_HEART[hpFrame].height * 1.25f - 12}, 22, 1, WHITE);

    int stFrame = (int)(((float)entPlayer.stamina / entPlayer.maxStamina) * 7.0f);
    if (stFrame >= 7)
        stFrame = 6;
    if (entPlayer.stamina <= 0)
        stFrame = 0;
    DrawTexturePro(texCombatUI, SRC_STAMINA[stFrame], (Rectangle){40, uiY + 110, SRC_STAMINA[stFrame].width * 2.5f, SRC_STAMINA[stFrame].height * 2.5f}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTextEx(globalFont, TextFormat("%d/%d", entPlayer.stamina, entPlayer.maxStamina), (Vector2){40 + SRC_STAMINA[stFrame].width * 2.5f + 15, uiY + 110 + SRC_STAMINA[stFrame].height * 1.25f - 12}, 22, 1, WHITE);

    float defIconSize = 28.0f;
    Vector2 defPos = {200, uiY + 105};

    if (texIconUpDef.id != 0)
    {
        DrawTexturePro(texIconUpDef, (Rectangle){0, 0, texIconUpDef.width, texIconUpDef.height},
                       (Rectangle){defPos.x, defPos.y, defIconSize, defIconSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }

    int currentDef = GetTotalDef(&entPlayer);
    Color defColor = (currentDef > entPlayer.def) ? GREEN : WHITE;
    DrawTextEx(globalFont, TextFormat(u8"Thủ: %d", currentDef),
               (Vector2){defPos.x + defIconSize + 8.0f, defPos.y + 4.0f}, 22, 1, defColor);

    Vector2 mPos = GetMouseScaled();
    const char *keys[4] = {"Q", "W", "E", "R"};
    for (int i = 0; i < 4; i++)
    {
        Rectangle r = GetBtnRect(i);
        bool hover = CheckCollisionPointRec(mPos, r);

        if (i == 1)
        {
            DrawSkillButton(r, keys[i], u8"Kỹ Năng", 0, 0, hover, true);
        }
        else
        {
            int dataIndex = (i == 0) ? 0 : (i == 2 ? 1 : 2);
            bool canCast = (entPlayer.stamina >= entPlayer.skills[dataIndex].staminaCost) && (entPlayer.skills[dataIndex].currentCooldown <= 0);
            DrawSkillButton(r, keys[i], entPlayer.skills[dataIndex].name, entPlayer.skills[dataIndex].damageBase, entPlayer.skills[dataIndex].staminaCost, hover, canCast);
        }
    }

    if (showTurnAnim)
    {
        float textX = (turnAnimTimer < 0.3f) ? -400 + (sw / 2 + 400) * (turnAnimTimer / 0.3f) : ((turnAnimTimer < 1.2f) ? sw / 2 : sw / 2 + (sw / 2 + 400) * ((turnAnimTimer - 1.2f) / 0.3f));
        DrawTextEx(globalFont, turnText, (Vector2){textX - MeasureTextEx(globalFont, turnText, 50, 1).x / 2 + 4, sh / 2 - 80 + 4}, 50, 1, Fade(BLACK, 0.7f));
        DrawTextEx(globalFont, turnText, (Vector2){textX - MeasureTextEx(globalFont, turnText, 50, 1).x / 2, sh / 2 - 80}, 50, 1, turnColor);
    }

    if (cb_bookState != CB_BOOK_IDLE)
    {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f));

        float bookWidth = sw * 0.8f;
        float bookHeight = sh * 0.75f;
        float startX = (sw - bookWidth) / 2.0f;
        float startY = (sh - bookHeight) / 2.0f + 20.0f + cb_bookDropY;
        float spineX = startX + bookWidth / 2.0f;
        float halfWidth = bookWidth / 2.0f;

        float paddingX = 25.0f;
        float paddingY = 20.0f;
        float gap = -25.0f;
        float pageWidth = (bookWidth - (paddingX * 2) - gap) / 2.0f;
        float pageHeight = bookHeight - (paddingY * 2);

        Rectangle srcCover = {32, 32, 224, 160};
        Rectangle srcPageLeft = {272, 32, 104, 147};
        Rectangle srcPageRight = {392, 32, 104, 147};

        NPatchInfo patchCover = {.source = srcCover, .left = 14, .top = 14, .right = 14, .bottom = 14, .layout = NPATCH_NINE_PATCH};
        NPatchInfo patchRight = {.source = srcPageRight, .left = 0, .top = 16, .right = 8, .bottom = 16, .layout = NPATCH_NINE_PATCH};
        NPatchInfo patchLeft = {.source = srcPageLeft, .left = 8, .top = 16, .right = 0, .bottom = 16, .layout = NPATCH_NINE_PATCH};

        Rectangle destCover = {startX, startY, bookWidth, bookHeight};
        Rectangle destRight = {startX + paddingX + pageWidth + gap, startY + paddingY, pageWidth, pageHeight};
        Rectangle destLeft = {startX + paddingX, startY + paddingY, pageWidth, pageHeight};

        if (cb_bookState == CB_BOOK_DROPPING || cb_bookState == CB_BOOK_PAUSE || cb_bookState == CB_BOOK_OPEN_RIGHT || cb_bookState == CB_BOOK_CLOSE_RIGHT || cb_bookState == CB_BOOK_EXITING)
        {
            float progress = (cb_bookState == CB_BOOK_OPEN_RIGHT) ? cb_bookProgress : ((cb_bookState == CB_BOOK_CLOSE_RIGHT) ? 1.0f - cb_bookProgress : 0.0f);
            progress = progress * progress * (3.0f - 2.0f * progress);

            BeginScissorMode((int)spineX, (int)startY, (int)halfWidth, (int)bookHeight);
            DrawTextureNPatch(texCombatUI, patchCover, destCover, (Vector2){0, 0}, 0.0f, WHITE);
            DrawTextureNPatch(texCombatUI, patchRight, destRight, (Vector2){0, 0}, 0.0f, WHITE);
            EndScissorMode();

            Rectangle movingCover;
            if (progress < 0.5f)
            {
                float p = progress * 2.0f;
                movingCover = (Rectangle){spineX, startY, halfWidth * (1.0f - p), bookHeight};
                DrawTexturePro(texCombatUI, (Rectangle){srcCover.x + srcCover.width / 2.0f, srcCover.y, srcCover.width / 2.0f, srcCover.height}, movingCover, (Vector2){0, 0}, 0.0f, WHITE);

                float auraScale = movingCover.width / halfWidth;
                Rectangle destAura = {spineX + movingCover.width / 2.0f, startY + bookHeight / 2.0f, 250.0f * auraScale, 250.0f};
                BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(texAura, (Rectangle){0, 0, texAura.width, texAura.height}, destAura, (Vector2){destAura.width / 2, destAura.height / 2}, cb_auraRot, Fade(GOLD, 0.8f));
                EndBlendMode();
            }
            else
            {
                float p = (progress - 0.5f) * 2.0f;
                movingCover = (Rectangle){spineX - (halfWidth * p), startY, halfWidth * p, bookHeight};
                DrawTexturePro(texCombatUI, (Rectangle){srcCover.x, srcCover.y, srcCover.width / 2.0f, srcCover.height}, movingCover, (Vector2){0, 0}, 0.0f, CB_GetPageFlipTint(progress));
            }
        }
        else
        {
            float progress = (cb_bookState == CB_BOOK_OPEN_LEFT) ? cb_bookProgress : ((cb_bookState == CB_BOOK_CLOSE_LEFT) ? 1.0f - cb_bookProgress : 1.0f);
            progress = progress * progress * (3.0f - 2.0f * progress);

            DrawTextureNPatch(texCombatUI, patchCover, destCover, (Vector2){0, 0}, 0.0f, WHITE);
            DrawTextureNPatch(texCombatUI, patchRight, destRight, (Vector2){0, 0}, 0.0f, WHITE);
            if (cb_bookState == CB_BOOK_FULLY_OPEN)
                DrawTextureNPatch(texCombatUI, patchLeft, destLeft, (Vector2){0, 0}, 0.0f, WHITE);

            if (progress > 0.0f && cb_bookState != CB_BOOK_FULLY_OPEN)
            {
                Rectangle movingPage;
                if (progress < 0.5f)
                {
                    float p = progress * 2.0f;
                    movingPage = (Rectangle){spineX + (destRight.x - spineX) * (1.0f - p), destRight.y, destRight.width * (1.0f - p), destRight.height};
                }
                else
                {
                    float p = (progress - 0.5f) * 2.0f;
                    movingPage = (Rectangle){spineX - (spineX - destLeft.x) * p, destLeft.y, destLeft.width * p, destLeft.height};
                }
                DrawTexturePro(texCombatUI, srcPageLeft, movingPage, (Vector2){0, 0}, 0.0f, CB_GetPageFlipTint(progress));
            }
        }

        if (cb_titleState != 0)
        {
            float titleW = 330.0f;
            float titleH = 60.0f;
            float titleX = sw / 2.0f - titleW / 2.0f;
            float titleY = startY - 30.0f + cb_titleYOffset;

            NPatchInfo patchTitle = {.source = (Rectangle){25, 27, 330, 47}, .left = 10, .top = 10, .right = 10, .bottom = 10, .layout = NPATCH_NINE_PATCH};
            DrawTextureNPatch(texTitleFrame, patchTitle, (Rectangle){titleX, titleY, titleW, titleH}, (Vector2){0, 0}, 0.0f, WHITE);

            const char *titleString = (cb_bookMode == 0) ? u8"KỸ NĂNG" : u8"HỒ SƠ CHỈ SỐ";
            Vector2 titleSize = MeasureTextEx(globalFont, titleString, 28, 1);
            DrawTextEx(globalFont, titleString, (Vector2){titleX + (titleW - titleSize.x) / 2.0f, titleY + (titleH - titleSize.y) / 2.0f}, 28, 1, (Color){94, 65, 56, 255});
        }

        if (cb_bookState == CB_BOOK_FULLY_OPEN)
        {
            DrawTextEx(globalFont, "ESC - DONG", (Vector2){startX + 40, startY + 30}, 18, 1, GRAY);
        }

        if (cb_bookState == CB_BOOK_FULLY_OPEN)
        {
            float leftCenterX = startX + paddingX + pageWidth / 2.0f;
            float rightCenterX = startX + paddingX + pageWidth + gap + pageWidth / 2.0f;
            float contentY = startY + 80.0f;

            if (cb_bookMode == 0)
            {
                for (int i = 0; i < 4; i++)
                {
                    int col = i % 2;
                    int row = i / 2;
                    float slotSize = 75.0f;
                    float slotSpacing = 40.0f;
                    float startGridX = leftCenterX - ((slotSize * 2) + slotSpacing) / 2.0f;
                    Rectangle slotRec = {startGridX + col * (slotSize + slotSpacing), contentY + row * (slotSize + 30.0f), slotSize, slotSize};

                    bool isHover = CheckCollisionPointRec(mPos, slotRec);
                    Rectangle activeSrc = (selectedBookSkill == i || isHover) ? SRC_SLOT_HOVER : SRC_SLOT_NORMAL;
                    NPatchInfo nPatchSlot = {.source = activeSrc, .left = 4, .top = 4, .right = 4, .bottom = 4, .layout = NPATCH_NINE_PATCH};
                    DrawTextureNPatch(texCombatUI, nPatchSlot, slotRec, (Vector2){0, 0}, 0.0f, WHITE);
                    if (selectedBookSkill == i)
                        DrawRectangleLinesEx(slotRec, 2.0f, Fade(ORANGE, 0.8f));
                        // --- CHÈN THÊM ĐOẠN NÀY ĐỂ VẼ ICON VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Giáp Gai") == 0 && texThornVFX.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    // Dùng BLEND_ADDITIVE để lọc nền đen cho đẹp
    BeginBlendMode(BLEND_ADDITIVE); 
    DrawTexturePro(texThornVFX, 
        (Rectangle){ 0, 0, (float)texThornVFX.width, (float)texThornVFX.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}
// --- VẼ ICON DÒ TÌM CON MỒI VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Dò Tìm Con Mồi") == 0 && texDoTimConMoi.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texDoTimConMoi, 
        (Rectangle){ 0, 0, (float)texDoTimConMoi.width, (float)texDoTimConMoi.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON KẺ CHỊU ĐÒN VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Kẻ Chịu Đòn") == 0 && texKeChiuDonIcon.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    // Ảnh đã vuông và xóa nền nên cứ thế vẽ thẳng vào, không lo bị bẹp!
    DrawTexturePro(texKeChiuDonIcon, 
        (Rectangle){ 0, 0, (float)texKeChiuDonIcon.width, (float)texKeChiuDonIcon.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// ----------------------------------------------
// --- VẼ ICON CÚ ĐẤM SẤM SÉT VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Cú Đấm Sấm Sét") == 0 && texLightningIcon.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texLightningIcon, 
        (Rectangle){ 0, 0, (float)texLightningIcon.width, (float)texLightningIcon.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}

// --- VẼ ICON PHẢN ỨNG HÓA HỌC VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Phản Ứng Hóa Học") == 0 && texIconPoison.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texIconPoison, 
        (Rectangle){ 0, 0, (float)texIconPoison.width, (float)texIconPoison.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON TẬP TRUNG CAO ĐỘ VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Tập Trung Cao Độ") == 0 && texIconTapTrung.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    BeginBlendMode(BLEND_ADDITIVE); // Blend Additive cho não phát sáng cực đẹp trên nền Slot
    DrawTexturePro(texIconTapTrung, 
        (Rectangle){ 0, 0, (float)texIconTapTrung.width, (float)texIconTapTrung.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}

// --- VẼ ICON CẦU TỪ TRƯỜNG VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Cầu Từ Trường") == 0 && texCauTuTruong.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    BeginBlendMode(BLEND_ADDITIVE); 
    DrawTexturePro(texCauTuTruong, 
        (Rectangle){ 0, 0, (float)texCauTuTruong.width, (float)texCauTuTruong.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}
// --- VẼ ICON CHIẾN THUẬT TƯ DUY VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Chiến Thuật Tư Duy") == 0 && texTuDuyChienThuat.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texTuDuyChienThuat, 
        (Rectangle){ 0, 0, (float)texTuDuyChienThuat.width, (float)texTuDuyChienThuat.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON ĐỊNH LÝ CUỐI CÙNG VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Định lý cuối cùng") == 0 && texDinhLiCuoiCung.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(texDinhLiCuoiCung, 
        (Rectangle){ 0, 0, (float)texDinhLiCuoiCung.width, (float)texDinhLiCuoiCung.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}
// --- VẼ ICON HÀO QUANG HUYẾT SẮC VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Hào Quang Huyết Sắc") == 0 && texHaoQuangHuyetSac.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    BeginBlendMode(BLEND_ADDITIVE); // Phát sáng cực đẹp trong UI
    DrawTexturePro(texHaoQuangHuyetSac, 
        (Rectangle){ 0, 0, (float)texHaoQuangHuyetSac.width, (float)texHaoQuangHuyetSac.height },
        iconDest, (Vector2){0, 0}, 0.0f, MAROON); // Nhuộm luôn đỏ máu
    EndBlendMode();
}

// --- VẼ ICON DẤU ẤN KÝ SINH ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Dấu Ấn Ký Sinh") == 0 && texDauAnKiSinh.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texDauAnKiSinh, (Rectangle){ 0, 0, (float)texDauAnKiSinh.width, (float)texDauAnKiSinh.height }, iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON GIAO KÈO ÁC QUỶ ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Giao Kèo Ác Quỷ") == 0 && texGiaoKeoAcQuy.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texGiaoKeoAcQuy, (Rectangle){ 0, 0, (float)texGiaoKeoAcQuy.width, (float)texGiaoKeoAcQuy.height }, iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
//vẽ icon thu hồi vốn
if (strcmp(entPlayer.skills[3 + i].name, u8"Thu Hồi Vốn") == 0 && texThuHoiVonCoin.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texThuHoiVonCoin, 
        (Rectangle){ 0, 0, (float)texThuHoiVonCoin.width, (float)texThuHoiVonCoin.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON PHÁ GIÁ THỊ TRƯỜNG VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Phá Giá Thị Trường") == 0 && texPhaGia.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texPhaGia, 
        (Rectangle){ 0, 0, (float)texPhaGia.width, (float)texPhaGia.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
// --- VẼ ICON TRIỆT HẠ CON MỒI VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Triệt Hạ Con Mồi") == 0 && texTrietHa.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texTrietHa, 
        (Rectangle){ 0, 0, (float)texTrietHa.width, (float)texTrietHa.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}

// --- VẼ ICON HUYẾT TIỄN VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Huyết Tiễn") == 0 && texHuyetTien.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texHuyetTien, 
        (Rectangle){ 0, 0, (float)texHuyetTien.width, (float)texHuyetTien.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
 
// --- VẼ ICON RUNG CHẤN VÀO SLOT ---
if (strcmp(entPlayer.skills[3 + i].name, u8"Rung Chấn") == 0 && texRungChanIcon.id != 0) {
    Rectangle iconDest = { slotRec.x + 5, slotRec.y + 5, slotRec.width - 10, slotRec.height - 10 };
    DrawTexturePro(texRungChanIcon, 
        (Rectangle){ 0, 0, (float)texRungChanIcon.width, (float)texRungChanIcon.height },
        iconDest, (Vector2){0, 0}, 0.0f, WHITE);
}
                    Vector2 nameSize = MeasureTextEx(globalFont, entPlayer.skills[3 + i].name, 14, 1);
                    DrawTextEx(globalFont, entPlayer.skills[3 + i].name, (Vector2){slotRec.x + (slotSize - nameSize.x) / 2.0f, slotRec.y + slotSize + 15}, 14, 1, BLACK);
                }

                if (selectedBookSkill != -1)
                {
                    CombatSkill *chosen = &entPlayer.skills[3 + selectedBookSkill];
                    
                    // --- [BẮT ĐẦU CẬP NHẬT UI SÁCH KỸ NĂNG] ---
                    int skillNameFont = 22; // Giảm từ 36 xuống 26
                    int skillDescFont = 15; // Giảm từ 22 xuống 18
                    int skillStatFont = 20; // Giảm từ 24 xuống 20
                    
                    float alignLeftX = rightCenterX - pageWidth / 2.0f + 40; // Thụt lề giống Bảng Chỉ Số
                    float textY = contentY;

                    // 1. Vẽ Tên Kỹ Năng (Căn giữa)
                    Vector2 nSize = MeasureTextEx(globalFont, chosen->name, skillNameFont, 1);
                    DrawTextEx(globalFont, chosen->name, (Vector2){rightCenterX - nSize.x / 2.0f, textY}, skillNameFont, 1, DARKBLUE);
                    textY += 35;
                    
                    // 2. Kẻ vạch phân cách
                    DrawLine(rightCenterX - 120, textY, rightCenterX + 120, textY, GRAY);
                    textY += 15;

                    // 3. Vẽ Mô tả
                    DrawTextEx(globalFont, chosen->desc, (Vector2){alignLeftX, textY}, skillDescFont, 1, BLACK);
                    
                    // Tự động tính toán chiều cao của khối mô tả để đẩy các dòng dưới xuống
                    Vector2 descSize = MeasureTextEx(globalFont, chosen->desc, skillDescFont, 1);
                    textY += descSize.y + 25; // Cộng thêm 25 pixel khoảng cách

                    // 4. Vẽ Sát thương và Thể lực
                    DrawTextEx(globalFont, TextFormat(u8"Sát thương: %d", chosen->damageBase), (Vector2){alignLeftX, textY}, skillStatFont, 1, RED);
                    textY += 30;
                    DrawTextEx(globalFont, TextFormat(u8"Thể lực tốn: %d", chosen->staminaCost), (Vector2){alignLeftX, textY}, skillStatFont, 1, ORANGE);
                    // --- [KẾT THÚC CẬP NHẬT UI SÁCH KỸ NĂNG] ---

                    // Giữ nguyên nút DÙNG ở dưới cùng
                    Rectangle useBtn = {rightCenterX - 90, contentY + 200, 180, 50};
                    bool useHover = cb_waitingToCloseBook ? false : CheckCollisionPointRec(mPos, useBtn);

                    bool canCast = (entPlayer.stamina >= chosen->staminaCost) && (chosen->currentCooldown <= 0);
                    DrawSkillButton(useBtn, "CLICK", u8"DÙNG", 0, 0, useHover, canCast);
                }
                else
                {
                    Vector2 hintSize = MeasureTextEx(globalFont, u8"Chọn kỹ năng để xem", 24, 1);
                    DrawTextEx(globalFont, u8"Chọn kỹ năng để xem", (Vector2){rightCenterX - hintSize.x / 2.0f, startY + pageHeight / 2.0f}, 24, 1, Fade(DARKGRAY, 0.6f));
                }
            }
           else if (cb_bookMode == 1)
            {
                // ==========================================
                // 1. CHUẨN BỊ DỮ LIỆU TÓM TẮT (KHÔNG SỐ LIỆU)
                // ==========================================
                const char *className = "";
                const char *passiveName = "";
                const char *passiveDesc = "";
                const char *attackDesc = "";

                switch (refPlayer->pClass) {
                    case CLASS_DAU_GAU: case CLASS_STUDENT:
                        className = u8"Lớp: Đầu Gấu";
                        passiveName = u8"[Nội Tại] Càng Đánh Càng Hăng";
                        passiveDesc = u8"Tích lũy sát thương gánh chịu\nthành Giáp vĩnh viễn.";
                        attackDesc = u8"[Đánh Thường] Gây ST vật lý,\ntăng nhẹ Giáp vĩnh viễn.";
                        break;
                    case CLASS_HOC_BA: case CLASS_WARRIOR:
                        className = u8"Lớp: Học Bá";
                        passiveName = u8"[Nội Tại] Đặc Quyền Sửa Sai";
                        passiveDesc = u8"Sống sót với 1 HP, hồi đầy Thể Lực\nvà xóa mọi debuff (1 lần/trận).";
                        attackDesc = u8"[Đánh Thường] Gây ST vật lý,\nhồi phục lượng lớn Thể Lực.";
                        break;
                    case CLASS_SOAI_CA: case CLASS_MAGE:
                        className = u8"Lớp: Soái Ca (Pháp Sư Máu)";
                        passiveName = u8"[Nội Tại] Cuồng Huyết & Thức Tỉnh";
                        passiveDesc = u8"Tăng Max HP mỗi lượt. Tự động\nhồi phục mạnh khi HP xuống thấp.";
                        attackDesc = u8"[Đánh Thường] Gây ST vật lý\nkèm ST chuẩn theo Max HP.";
                        break;
                    case CLASS_PHU_NHI_DAI: case CLASS_ARCHER:
                        className = u8"Lớp: Phú Nhị Đại (Tư Bản)";
                        passiveName = u8"[Nội Tại] Lãi Suất Kép";
                        passiveDesc = u8"Tiêu hao Thể Lực giúp tăng\nvĩnh viễn Tỉ lệ Chí Mạng.";
                        attackDesc = u8"[Đánh Thường] Gây ST theo Công tổng.\nTăng vĩnh viễn Tấn Công.";
                        break;
                }

                // ==========================================
                // 2. VẼ TRANG TRÁI (HỒ SƠ & NỘI TẠI)
                // ==========================================
                int headerFont = 20; // Ép size tiêu đề từ 30 xuống 24
                DrawTextEx(globalFont, u8"HỒ SƠ NHÂN VẬT", (Vector2){leftCenterX - MeasureTextEx(globalFont, u8"HỒ SƠ NHÂN VẬT", headerFont, 1).x / 2, contentY-25.0f}, headerFont, 1, DARKBLUE);
                DrawLine(leftCenterX - 100, contentY + 30, leftCenterX + 100, contentY, GRAY);

                // Ép Avatar nhỏ lại (Scale từ 3.5 xuống 2.0)
                int pFrame = (int)(GetTime() * 8.0f) % refPlayer->maxFrames;
                DrawTexturePro(*entPlayer.texture,
                               (Rectangle){pFrame * refPlayer->spriteWidth, 1 * refPlayer->spriteHeight, (float)refPlayer->spriteWidth, (float)refPlayer->spriteHeight},
                               (Rectangle){leftCenterX, contentY + 40, refPlayer->spriteWidth * 2.0f, refPlayer->spriteHeight * 2.0f},
                               (Vector2){(refPlayer->spriteWidth * 1.8f) / 2, (refPlayer->spriteHeight * 2.0f) / 2}, 0.0f, WHITE);

                float leftLineY = contentY + 80; // Kéo text lên sát hình hơn
                float alignLeftX = startX + paddingX + 30; // Căn lề trái
                
                DrawTextEx(globalFont, className, (Vector2){alignLeftX, leftLineY}, 18, 1, MAROON); // Size 20
                leftLineY += 35;
                
                DrawTextEx(globalFont, passiveName, (Vector2){alignLeftX, leftLineY}, 16, 1, DARKGREEN); // Size 18
                leftLineY += 22;
                DrawTextEx(globalFont, passiveDesc, (Vector2){alignLeftX + 10, leftLineY}, 14, 1, DARKGRAY); // Size 16
                leftLineY += 45;

                DrawTextEx(globalFont, attackDesc, (Vector2){alignLeftX, leftLineY}, 18, 1, DARKBLUE);

                // ==========================================
                // 3. TÍNH TOÁN CÁC CHỈ SỐ LIVE CHO TRANG PHẢI
                // ==========================================
                int currentAtk = GetTotalAtk(&entPlayer);
                int atkBonus = currentAtk - entPlayer.atk;

                int currentDef = GetTotalDef(&entPlayer);
                int defBonus = currentDef - entPlayer.def;

                int speedBonus = 0;
                int critBuff = 0;
                for (int k = 0; k < MAX_EFFECTS; k++) {
                    if (entPlayer.effects[k].duration > 0) {
                        if (entPlayer.effects[k].type == EFFECT_SPD_UP) speedBonus += entPlayer.effects[k].value;
                        if (entPlayer.effects[k].type == EFFECT_SPD_DOWN) speedBonus -= entPlayer.effects[k].value;
                        if (entPlayer.effects[k].type == EFFECT_CRIT_UP) critBuff += entPlayer.effects[k].value;
                    }
                }
                int currentSpeed = entPlayer.speed + speedBonus;
                int currentCrit = entPlayer.critRate + entPlayer.permanentCritBonus + critBuff;
                int critBonus = currentCrit - entPlayer.critRate;

                // ==========================================
                // 4. VẼ TRANG PHẢI (BẢNG CHỈ SỐ CHI TIẾT)
                // ==========================================
                DrawTextEx(globalFont, u8"BẢNG CHỈ SỐ (LIVE)", (Vector2){rightCenterX - MeasureTextEx(globalFont, u8"BẢNG CHỈ SỐ (LIVE)", headerFont, 1).x / 2, contentY}, headerFont, 1, DARKGREEN);
               

                float lineY = contentY + 30;
                float spacing = 30.0f; // Ép khoảng cách dòng từ 45 xuống 35
                float alignX = rightCenterX - pageWidth / 2.0f + 45; // Thụt lề vào trong cho an toàn
                int mainStatFont = 16; // Ép size chỉ số chính xuống 20
                int subStatFont = 14;  // Ép size chỉ số phụ xuống 16

                // [MÁU - HP]
                DrawTextEx(globalFont, TextFormat(u8"Sinh Lực (HP): %d / %d", entPlayer.hp, entPlayer.maxHp), (Vector2){alignX, lineY}, mainStatFont, 1, RED);
                if (entPlayer.maxHp > refPlayer->stats.maxHp) {
                    DrawTextEx(globalFont, TextFormat(u8"(%d,b+%d)", refPlayer->stats.maxHp, entPlayer.maxHp - refPlayer->stats.maxHp), (Vector2){alignX + 140, lineY + 3}, subStatFont, 1, GREEN);
                }
                lineY += spacing;

                // [THỂ LỰC - STA]
                DrawTextEx(globalFont, TextFormat(u8"Thể Lực (STA): %d / %d", entPlayer.stamina, entPlayer.maxStamina), (Vector2){alignX, lineY}, mainStatFont, 1, ORANGE);
                lineY += spacing;

                // [TẤN CÔNG - ATK]
                DrawTextEx(globalFont, TextFormat(u8"Tấn Công (ATK): %d", currentAtk), (Vector2){alignX, lineY}, mainStatFont, 1, MAROON);
                if (atkBonus != 0) {
                    Color c = atkBonus > 0 ? GREEN : RED;
                    const char* sign = atkBonus > 0 ? "+" : "";
                    DrawTextEx(globalFont, TextFormat(u8"(%d, b+%s%d)", entPlayer.atk, sign, atkBonus), (Vector2){alignX + 140, lineY + 3}, subStatFont, 1, c);
                }
                lineY += spacing;

                // [PHÒNG THỦ - DEF]
                DrawTextEx(globalFont, TextFormat(u8"Phòng Thủ (DEF): %d", currentDef), (Vector2){alignX, lineY}, mainStatFont, 1, DARKGREEN);
                if (defBonus != 0) {
                    Color c = defBonus > 0 ? GREEN : RED;
                    const char* sign = defBonus > 0 ? "+" : "";
                    DrawTextEx(globalFont, TextFormat(u8"(%d, b%+s%d)", entPlayer.def, sign, defBonus), (Vector2){alignX + 140, lineY + 3}, subStatFont, 1, c);
                }
                lineY += spacing;

                // [TỐC ĐỘ - SPD]
                DrawTextEx(globalFont, TextFormat(u8"Tốc Độ (SPD): %d", currentSpeed), (Vector2){alignX, lineY}, mainStatFont, 1, SKYBLUE);
                if (speedBonus != 0) {
                    Color c = speedBonus > 0 ? GREEN : RED;
                    const char* sign = speedBonus > 0 ? "+" : "";
                    DrawTextEx(globalFont, TextFormat(u8"(%d, b%+s%d)", entPlayer.speed, sign, speedBonus), (Vector2){alignX + 140, lineY + 3}, subStatFont, 1, c);
                }
                lineY += spacing;

                // [CHÍ MẠNG - CRIT RATE]
                DrawTextEx(globalFont, TextFormat(u8"Chí Mạng (CRIT): %d%%", currentCrit), (Vector2){alignX, lineY}, mainStatFont, 1, PURPLE);
                if (critBonus != 0) {
                    Color c = critBonus > 0 ? GREEN : RED;
                    const char* sign = critBonus > 0 ? "+" : "";
                    DrawTextEx(globalFont, TextFormat(u8"(%d%%, b+%s%d%%)", entPlayer.critRate, sign, critBonus), (Vector2){alignX + 140, lineY + 3}, subStatFont, 1, c);
                }
                lineY += spacing;

                // [SÁT THƯƠNG CHÍ MẠNG - CRIT DMG]
                DrawTextEx(globalFont, TextFormat(u8"ST Chí Mạng: x%.1f", entPlayer.critDamage), (Vector2){alignX, lineY}, mainStatFont, 1, MAGENTA);
            }
        }
    }

    // =========================================================
    // [CẬP NHẬT] VẼ CHỮ NỔI TO VÀ ĐỔ BÓNG
    // =========================================================
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++)
    {
        if (fTexts[i].lifetime > 0)
        {
            fTexts[i].lifetime -= GetFrameTime();
            fTexts[i].pos.y += fTexts[i].velocityY * GetFrameTime();

            float alpha = fTexts[i].lifetime / fTexts[i].maxLifetime;
            int fontSize = 70; // Size thường
            if (fTexts[i].isCrit)
                fontSize = 100; // Size Chí mạng

            // Vẽ viền đổ bóng (Đen) lệch xuống 3px để số luôn rõ ràng
            DrawTextEx(globalFont, fTexts[i].text, (Vector2){fTexts[i].pos.x + 3, fTexts[i].pos.y + 3}, fontSize, 1, Fade(BLACK, alpha * 0.7f));
            // Vẽ chữ thật
            DrawTextEx(globalFont, fTexts[i].text, fTexts[i].pos, fontSize, 1, Fade(fTexts[i].color, alpha));
        }
    }

    // =========================================================
    // [MỚI] VẼ HOẠT ẢNH ICON HIỆU ỨNG BAY TỪ GIỮA MÀN HÌNH
    // =========================================================
    for (int i = 0; i < 10; i++)
    {
        if (iconAnims[i].active)
        {
            iconAnims[i].timer -= GetFrameTime();
            if (iconAnims[i].timer <= 0)
            {
                iconAnims[i].active = false;
                continue;
            }

            float p = 1.0f - (iconAnims[i].timer / iconAnims[i].maxTime); // Từ 0.0 -> 1.0

            Texture2D *tex = NULL;
            bool isDown = false;
            switch (iconAnims[i].type)
            {
            case EFFECT_POISON:
                tex = &texIconPoison;
                break;
            case EFFECT_SILENCE:
                tex = &texIconSilence;
                break;
            case EFFECT_HEAL:
                tex = &texIconUpHp;
                break;
            case EFFECT_DEF_UP:
                tex = &texIconUpDef;
                break;
            case EFFECT_DEF_DOWN:
                tex = &texIconUpDef;
                isDown = true;
                break;
            case EFFECT_ATK_DOWN:
                tex = &texIconUpAtk; 
                isDown = true;       
                break;
            case EFFECT_ATK_UP:
                tex = &texIconUpAtk;
                break;
            case EFFECT_STUN:
                tex = &texIconStun;
                break;
            case EFFECT_THORN_ARMOR:
                tex = &texIconUpDef;
                break;
            default:
                break;
            }

            if (tex != NULL && tex->id != 0)
            {
                // Tâm điểm nổ
                Vector2 startPos = {sw / 2.0f, sh / 2.0f - 50.0f};
                // Đích đến
                Vector2 targetPos = iconAnims[i].isPlayerTarget ? (Vector2){pPos.x - 50, pPos.y - 120 - jumpOffset} : (Vector2){ePos.x - 70, ePos.y - 100};

                float scale = 0.0f;
                Vector2 currentPos = startPos;

                if (p < 0.25f)
                {
                    // Giai đoạn 1: Phình to lên cực độ tại giữa màn hình
                    scale = (p / 0.25f) * 5.0f; // Scale gấp 3 lần
                }
                else
                {
                    // Giai đoạn 2: Thu nhỏ dần và bay về mục tiêu
                    float subP = (p - 0.25f) / 0.75f;
                    float smoothP = subP * subP * (3.0f - 2.0f * subP);

                    currentPos.x = startPos.x + (targetPos.x - startPos.x) * smoothP;
                    currentPos.y = startPos.y + (targetPos.y - startPos.y) * smoothP;

                    scale = 3.0f - (2.0f * smoothP); // Thu dần về size 1x
                }

                float baseSize = 50.0f;
                float drawSize = baseSize * scale;
                float animAlpha = (p > 0.9f) ? 1.0f - ((p - 0.9f) / 0.1f) : 1.0f; // Mờ dần ở khúc cuối cho mượt

                DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                               (Rectangle){currentPos.x - drawSize / 2.0f, currentPos.y - drawSize / 2.0f, drawSize, drawSize},
                               (Vector2){0, 0}, 0.0f, Fade(WHITE, animAlpha));

                if (isDown)
                {
                    DrawLineEx((Vector2){currentPos.x - drawSize / 2.0f, currentPos.y - drawSize / 2.0f},
                               (Vector2){currentPos.x + drawSize / 2.0f, currentPos.y + drawSize / 2.0f},
                               3.0f * scale, Fade(RED, animAlpha));
                }
            }
        }
    }

    DrawStatusIcons(&entPlayer, (Vector2){pPos.x - 50, pPos.y - 120 - jumpOffset});
    DrawStatusIcons(&entBoss, (Vector2){ePos.x - 70, ePos.y - 100});

    if (reviveVFXTimer > 0.0f) {
        reviveVFXTimer += GetFrameTime(); 
        DrawReviveVFX(pPos, reviveVFXTimer);
    }

    if (combatState == COMBAT_STATE_VICTORY || combatState == COMBAT_STATE_DEFEAT)
    {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, fadeAlpha));
        const char *endText = (combatState == COMBAT_STATE_VICTORY) ? u8"CHIẾN THẮNG!" : u8"BẠN ĐÃ BỊ ĐÁNH BẠI";
        Color endColor = (combatState == COMBAT_STATE_VICTORY) ? YELLOW : RED;
        DrawTextEx(globalFont, endText, (Vector2){(sw - MeasureTextEx(globalFont, endText, 60, 1).x) / 2, (sh - 60) / 2}, 60, 1, Fade(endColor, fadeAlpha));
    }
}