// FILE: src/combat.c
#include "combat.h"
#include "raylib.h"   
#include "player.h"   
#include "npc.h"      
#include "settings.h"
#include "audio_manager.h" 
#include <stdio.h>
#include <string.h>

// --- HỆ THỐNG SỐ SÁT THƯƠNG BAY (FLOATING TEXT) ---
typedef struct {
    char text[16];      
    Vector2 position;   
    float lifeTime;     
    bool active;        
    Color color;        
} FloatingText;

#define MAX_FLOAT_TEXTS 20
static FloatingText floatTexts[MAX_FLOAT_TEXTS]; 

// --- BIẾN TOÀN CỤC ---
static bool combatActive = false;
static CombatState combatState;
static float stateTimer = 0.0f;
static Player *pPlayer = NULL;
static Npc *pEnemy = NULL;
static Texture2D texBackground;
static bool resourcesLoaded = false;
static char messageLog[64] = "";

// CẤU HÌNH UI
#define UI_HEIGHT 180
#define AVATAR_SIZE 60
#define BTN_W 140
#define BTN_H 60        
#define BTN_PAD 10

// --- HÀM LẤY TỌA ĐỘ CHUỘT (SCALED) ---
Vector2 GetMouseScaled() {
    float scale = (float)GetScreenWidth() / SCREEN_WIDTH;
    if ((float)GetScreenHeight() / SCREEN_HEIGHT < scale) {
        scale = (float)GetScreenHeight() / SCREEN_HEIGHT;
    }
    
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = { 0 };
    
    virtualMouse.x = (mouse.x - (GetScreenWidth() - (SCREEN_WIDTH*scale))*0.5f) / scale;
    virtualMouse.y = (mouse.y - (GetScreenHeight() - (SCREEN_HEIGHT*scale))*0.5f) / scale;
    
    return virtualMouse;
}

// --- HÀM TẠO SỐ BAY ---
void SpawnFloatingText(int value, Vector2 pos, Color color) {
    for(int i = 0; i < MAX_FLOAT_TEXTS; i++) {
        if (!floatTexts[i].active) {
            sprintf(floatTexts[i].text, "-%d", value);
            floatTexts[i].position.x = pos.x + GetRandomValue(-20, 20);
            floatTexts[i].position.y = pos.y;
            floatTexts[i].lifeTime = 0.8f; 
            floatTexts[i].active = true;
            floatTexts[i].color = color;
            return; 
        }
    }
}

void Combat_Init() {
    if (!resourcesLoaded) {
        if (FileExists("resources/game_map/map6/phonglab.png")) {
            texBackground = LoadTexture("resources/game_map/map6/phonglab.png");
        } else {
            Image img = GenImageColor(800, 600, DARKGRAY);
            texBackground = LoadTextureFromImage(img);
            UnloadImage(img);
        }
        resourcesLoaded = true;
        for(int i=0; i<MAX_FLOAT_TEXTS; i++) floatTexts[i].active = false;
    }
}

void Combat_Shutdown() {
    if (resourcesLoaded) {
        UnloadTexture(texBackground);
        resourcesLoaded = false;
    }
}

void Combat_Start(Player *player, Npc *enemy) {
    pPlayer = player;
    pEnemy = enemy;
    pEnemy->stats.currentHp = pEnemy->stats.maxHp;
    pPlayer->stats.stamina = 100; 

    combatActive = true;
    combatState = COMBAT_STATE_START;
    stateTimer = 0.0f;
    
    for(int i=0; i<MAX_FLOAT_TEXTS; i++) floatTexts[i].active = false;

    Audio_PlayMusic(MUSIC_MAP_DEN); 
    sprintf(messageLog, "VAO TRAN: %s!", pEnemy->name);
}

bool Combat_IsActive() { return combatActive; }

// --- LOGIC PLAYER ĐÁNH ---
bool ExecutePlayerSkill(int skillIndex) {
    Skill *s = &pPlayer->skills[skillIndex];

    if (pPlayer->stats.stamina < s->staminaCost) {
        sprintf(messageLog, "Met qua! Can %d TL.", s->staminaCost);
        Audio_PlaySoundEffect(SFX_UI_HOVER); 
        return false;
    }

    pPlayer->stats.stamina -= s->staminaCost;

    int totalDmg = (pPlayer->stats.damage + s->damage) - pEnemy->stats.defense;
    if (totalDmg <= 0) totalDmg = 1;

    pEnemy->stats.currentHp -= totalDmg; 
    if (pEnemy->stats.currentHp < 0) pEnemy->stats.currentHp = 0;
    
    sprintf(messageLog, "Dung %s! Gay %d Sat thuong.", s->name, totalDmg);
    Audio_PlaySoundEffect(SFX_ATTACK); 

    Vector2 enemyHeadPos = { (float)SCREEN_WIDTH - 150, (float)(SCREEN_HEIGHT - UI_HEIGHT)/2 - 50 };
    SpawnFloatingText(totalDmg, enemyHeadPos, RED);

    return true;
}

// --- LOGIC ENEMY ĐÁNH ---
void ExecuteEnemyAttack() {
    int dmg = pEnemy->stats.damage - pPlayer->stats.defense;
    if (dmg <= 0) dmg = 1;
    
    pPlayer->stats.currentHp -= dmg; 
    if (pPlayer->stats.currentHp < 0) pPlayer->stats.currentHp = 0;
    
    sprintf(messageLog, "%s phan don! Gay %d Sat thuong.", pEnemy->name, dmg);
    Audio_PlaySoundEffect(SFX_HIT); 

    Vector2 playerHeadPos = { 150.0f, (float)(SCREEN_HEIGHT - UI_HEIGHT)/2 - 50 };
    SpawnFloatingText(dmg, playerHeadPos, ORANGE);
}

// --- HÀM TÍNH VỊ TRÍ NÚT (ĐÃ SỬA CHO THẤP XUỐNG) ---
Rectangle GetBtnRect(int index) {
    int sw = SCREEN_WIDTH;
    int sh = SCREEN_HEIGHT;
    int uiY = sh - UI_HEIGHT;
    
    int startX = sw - (BTN_W * 2 + BTN_PAD) - 50; 
    
    // [SỬA] Đẩy nút xuống thấp hơn (45px từ trên xuống) 
    // để nhường chỗ cho dòng chữ thông báo ở trên
    int startY = uiY + 45; 

    if (index == 0) return (Rectangle){ startX, startY, BTN_W, BTN_H }; // Q
    if (index == 1) return (Rectangle){ startX, startY + BTN_H + BTN_PAD, BTN_W, BTN_H }; // W
    if (index == 2) return (Rectangle){ startX + BTN_W + BTN_PAD, startY, BTN_W, BTN_H }; // E
    if (index == 3) return (Rectangle){ startX + BTN_W + BTN_PAD, startY + BTN_H + BTN_PAD, BTN_W, BTN_H }; // R
    
    return (Rectangle){0,0,0,0};
}

void Combat_Update() {
    if (!combatActive) return;
    stateTimer += GetFrameTime();

    for(int i=0; i<MAX_FLOAT_TEXTS; i++) {
        if (floatTexts[i].active) {
            floatTexts[i].lifeTime -= GetFrameTime();
            floatTexts[i].position.y -= 1.5f; 
            if (floatTexts[i].lifeTime <= 0) floatTexts[i].active = false;
        }
    }

    Vector2 mousePos = GetMouseScaled();
    bool isClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    switch (combatState) {
        case COMBAT_STATE_START:
            if (stateTimer > 1.0f) {
                combatState = COMBAT_STATE_PLAYER_TURN;
                sprintf(messageLog, "Luot cua BAN! (Q, W, E)");
            }
            break;

        case COMBAT_STATE_PLAYER_TURN:
            // Xử lý Input (Phím + Chuột)
            if (IsKeyPressed(KEY_Q) || (CheckCollisionPointRec(mousePos, GetBtnRect(0)) && isClick)) { 
                if (ExecutePlayerSkill(0)) { combatState = COMBAT_STATE_ACTION; stateTimer = 0.0f; }
            }
            else if (IsKeyPressed(KEY_W) || (CheckCollisionPointRec(mousePos, GetBtnRect(1)) && isClick)) { 
                if (ExecutePlayerSkill(1)) { combatState = COMBAT_STATE_ACTION; stateTimer = 0.0f; }
            }
            else if (IsKeyPressed(KEY_E) || (CheckCollisionPointRec(mousePos, GetBtnRect(2)) && isClick)) { 
                if (ExecutePlayerSkill(2)) { combatState = COMBAT_STATE_ACTION; stateTimer = 0.0f; }
            }
            else if (IsKeyPressed(KEY_R) || (CheckCollisionPointRec(mousePos, GetBtnRect(3)) && isClick)) { 
                combatActive = false;
                if (pEnemy->id == 99) {
                    pPlayer->stats.currentHp = pPlayer->stats.maxHp;
                    pPlayer->stats.stamina = 100; 
                }
                Audio_PlayMusic(MUSIC_THU_VIEN); 
            }
            break;

        case COMBAT_STATE_ACTION:
            if (stateTimer > 0.8f) { 
                if (pEnemy->stats.currentHp <= 0) {
                    combatState = COMBAT_STATE_VICTORY;
                    sprintf(messageLog, "CHIEN THANG!");
                } else {
                    combatState = COMBAT_STATE_ENEMY_TURN;
                    sprintf(messageLog, "Doi thu dang chuan bi...");
                    stateTimer = 0.0f;
                }
            }
            break;

        case COMBAT_STATE_ENEMY_TURN:
            if (stateTimer > 1.0f) {
                ExecuteEnemyAttack();
                if (pPlayer->stats.currentHp <= 0) {
                    combatState = COMBAT_STATE_DEFEAT;
                    sprintf(messageLog, "BAN DA BI HA GUC...");
                } else {
                    combatState = COMBAT_STATE_PLAYER_TURN;
                    sprintf(messageLog, "Luot cua BAN!");
                }
            }
            break;

        case COMBAT_STATE_VICTORY:
        case COMBAT_STATE_DEFEAT:
            if (IsKeyPressed(KEY_ENTER)) {
                if (combatState == COMBAT_STATE_DEFEAT) {
                    pPlayer->stats.currentHp = pPlayer->stats.maxHp;
                    pPlayer->stats.stamina = 100;
                }
                combatActive = false;
                Audio_PlayMusic(MUSIC_THU_VIEN);
            }
            break;
    }
}

// --- VẼ NÚT SKILL (CÓ CĂN CHỈNH CHỮ) ---
void DrawSkillButton(Rectangle rect, const char* key, Skill *skill, bool isHover) {
    Color bgColor = isHover ? Fade(SKYBLUE, 0.3f) : WHITE;
    
    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, isHover ? 2.0f : 1.0f, BLACK); 
    
    // Tên Skill (Cỡ 16) - Dịch vào 8px
    DrawText(TextFormat("%s (%s)", skill->name, key), rect.x + 8, rect.y + 8, 16, BLACK);
    
    // Chỉ số (Cỡ 14) - Dịch xuống 35px
    DrawText(TextFormat("ATK:%d  TL:%d", skill->damage, skill->staminaCost), rect.x + 8, rect.y + 35, 14, DARKBLUE);
}

void Combat_Draw() {
    if (!combatActive) return;

    int sw = SCREEN_WIDTH;
    int sh = SCREEN_HEIGHT;

    // 1. SÂN ĐẤU
    DrawTexturePro(texBackground, 
        (Rectangle){0, 0, (float)texBackground.width, (float)texBackground.height},
        (Rectangle){0, 0, (float)sw, (float)(sh - UI_HEIGHT)}, 
        (Vector2){0, 0}, 0.0f, WHITE);

    // PLAYER
    Vector2 playerPos = { 150, (sh - UI_HEIGHT) / 2 + 50 }; 
    Rectangle srcP = pPlayer->frameRec; 
    if (srcP.width < 0) srcP.width = -srcP.width; 
    Rectangle destP = { playerPos.x, playerPos.y, pPlayer->drawWidth * 2, pPlayer->drawHeight * 2 };
    DrawTexturePro(*pPlayer->currentTexture, srcP, destP, (Vector2){destP.width/2, destP.height/2}, 0.0f, WHITE);

    // ENEMY
    Vector2 enemyPos = { sw - 150, (sh - UI_HEIGHT) / 2 + 50 };
    float enemyFrameW = (float)pEnemy->texture.width / (pEnemy->frameCount > 0 ? pEnemy->frameCount : 1);
    Rectangle srcE = { 0, 0, enemyFrameW, (float)pEnemy->texture.height };
    srcE.width = -srcE.width; 
    Rectangle destE = { enemyPos.x, enemyPos.y, enemyFrameW * 2, srcE.height * 2 }; 
    DrawTexturePro(pEnemy->texture, srcE, destE, (Vector2){destE.width/2, destE.height/2}, 0.0f, WHITE);

    // Thanh Máu Enemy
    DrawRectangle(enemyPos.x - 40, enemyPos.y - 80, 80, 10, RED);
    float hpPct = (float)pEnemy->stats.currentHp / pEnemy->stats.maxHp;
    if (hpPct < 0) hpPct = 0;
    DrawRectangle(enemyPos.x - 40, enemyPos.y - 80, (int)(80 * hpPct), 10, GREEN);
    DrawRectangleLines(enemyPos.x - 40, enemyPos.y - 80, 80, 10, BLACK);
    DrawText(pEnemy->name, enemyPos.x - 20, enemyPos.y - 100, 20, WHITE);

    // SỐ SÁT THƯƠNG
    for(int i=0; i<MAX_FLOAT_TEXTS; i++) {
        if (floatTexts[i].active) {
            DrawText(floatTexts[i].text, (int)floatTexts[i].position.x + 2, (int)floatTexts[i].position.y + 2, 40, BLACK);
            DrawText(floatTexts[i].text, (int)floatTexts[i].position.x, (int)floatTexts[i].position.y, 40, floatTexts[i].color);
        }
    }

    // 2. UI PANEL
    int uiY = sh - UI_HEIGHT;
    DrawRectangle(0, uiY, sw, UI_HEIGHT, LIGHTGRAY);
    DrawRectangleLines(0, uiY, sw, UI_HEIGHT, BLACK); 

    // AVATAR & STATS
    DrawCircle(60, uiY + 60, AVATAR_SIZE/2, RAYWHITE);
    DrawCircleLines(60, uiY + 60, AVATAR_SIZE/2, BLACK);
    DrawText("AVT", 45, uiY + 50, 10, BLACK);
    DrawText(TextFormat("HP: %d/%d", pPlayer->stats.currentHp, pPlayer->stats.maxHp), 110, uiY + 25, 20, RED);
    DrawText(TextFormat("TL: %d/100", pPlayer->stats.stamina), 110, uiY + 50, 20, ORANGE);
    DrawRectangle(30, uiY + 110, 100, 40, WHITE);
    DrawRectangleLines(30, uiY + 110, 100, 40, BLACK);
    DrawText("Tui Do", 50, uiY + 120, 20, BLACK);

    // --- VẼ 4 NÚT SKILL ---
    Vector2 mousePos = GetMouseScaled();
    
    // Q, W, E
    Rectangle rectQ = GetBtnRect(0);
    bool hoverQ = CheckCollisionPointRec(mousePos, rectQ);
    DrawSkillButton(rectQ, "Q", &pPlayer->skills[0], hoverQ);

    Rectangle rectW = GetBtnRect(1);
    bool hoverW = CheckCollisionPointRec(mousePos, rectW);
    DrawSkillButton(rectW, "W", &pPlayer->skills[1], hoverW);

    Rectangle rectE = GetBtnRect(2);
    bool hoverE = CheckCollisionPointRec(mousePos, rectE);
    DrawSkillButton(rectE, "E", &pPlayer->skills[2], hoverE);
    
    // Run (R)
    Rectangle rectR = GetBtnRect(3);
    bool hoverR = CheckCollisionPointRec(mousePos, rectR);
    Color runBg = hoverR ? Fade(PINK, 0.3f) : WHITE;
    DrawRectangleRec(rectR, runBg);
    DrawRectangleLinesEx(rectR, hoverR ? 2.0f : 1.0f, BLACK);
    const char* runText = "Run (R)";
    int textW = MeasureText(runText, 20);
    DrawText(runText, rectR.x + (BTN_W - textW)/2, rectR.y + 20, 20, RED);

    // VẼ DÒNG THÔNG BÁO Ở GIỮA (ĐÃ THOÁNG HƠN)
    int msgWidth = MeasureText(messageLog, 20);
    // Vẽ ở vị trí Y = 10 (cách mép trên 10px)
    DrawText(messageLog, (sw - msgWidth)/2, uiY + 15, 20, DARKBLUE);
}