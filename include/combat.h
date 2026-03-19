// FILE: src/combat.h
#ifndef COMBAT_H
#define COMBAT_H

#include <stdbool.h>
#include "raylib.h"
#include "player.h"
#include "npc.h"

// --- 1. ĐỊNH NGHĨA CÁC LOẠI HIỆU ỨNG (BUFF/DEBUFF) ---
typedef enum {
    EFFECT_NONE = 0,
    // Buff
    EFFECT_HEAL,            // Hồi máu mỗi turn
    EFFECT_REGEN_STAMINA,   // Hồi thể lực mỗi turn
    EFFECT_SPD_UP,          // Tăng tốc độ
    EFFECT_ATK_UP,          // Tăng Tấn công
    EFFECT_DEF_UP,          // Tăng Phòng thủ
    EFFECT_CRIT_UP,
    EFFECT_ENDURE,          // Sống sót với 1 máu
    // Debuff
    EFFECT_POISON,          // Mất máu mỗi turn
    EFFECT_STUN,            // Choáng (Bỏ qua lượt)
    EFFECT_SPD_DOWN,        // Giảm tốc độ
    EFFECT_SILENCE,          // Cấm dùng skill
    EFFECT_DEF_DOWN,        // Giảm Giáp địch
    EFFECT_ATK_DOWN,        //GIẢM CÔNG ĐỊCH
    EFFECT_THORN_ARMOR      // Giáp gai (Phản sát thương)
} EffectType;

// --- 2. CẤU TRÚC 1 HIỆU ỨNG ĐANG ÁP DỤNG ---
typedef struct {
    EffectType type;
    int duration;           // Số lượt còn lại
    int value;              // Giá trị (VD: Độc trừ 20 máu)
} CombatEffect;

#define MAX_EFFECTS 5


// --- 3. CẤU TRÚC KỸ NĂNG (SKILL) ---
typedef struct {
    char name[32];
    char desc[128];         // [MỚI] Miêu tả chi tiết kỹ năng hiện trong sách
    int damageBase;         // Sát thương gốc
    int staminaCost;        // Tiêu hao thể lực
    float actionDelay;      // Trọng lượng hành động 
    
    EffectType applyEffect; // Hiệu ứng đi kèm (nếu có)
    int effectDuration;
    int effectValue;
    bool isUltimate;        // True nếu là Chiêu cuối
    bool scaleWithDef;      // Dành cho Đầu Gấu: Sát thương cộng thêm dựa trên Giáp
    bool scaleWithMaxHp;    // Dành cho Soái Ca: Sát thương cộng thêm dựa trên Máu Tối Đa
    // --- [MỚI] CÁC THÔNG SỐ ĐẶC BIỆT ---
    int effectChance;            // Tỷ lệ trúng hiệu ứng (0 - 100%)
    float lifestealDmgPercent;   // Tỷ lệ hút máu từ sát thương (VD: 1.2 = 120%)
    float healByDefPercent;      // Hồi máu theo % Giáp bản thân (VD: 1.5 = 150%)
    float damageByDefPercent;    // Gây sát thương theo % Giáp (VD: 3.0 = 300%)
    
    int maxCooldown;             // Số hiệp hồi chiêu tối đa
    int currentCooldown;
} CombatSkill;

// --- 4. THỰC THỂ CHIẾN ĐẤU (BỌC PLAYER VÀ NPC LẠI) ---
typedef struct {
    char name[32];
    bool isPlayer;          
    Texture2D* texture;     
    Rectangle frameRec;   
    // --- [MỚI CHÈN THÊM] ĐỊNH DANH ĐỂ GỌI NỘI TẠI (PASSIVE) ---
    PlayerClass pClass;     // Xác định rõ đang cầm Class nào để cuối turn tự buff nội tại  
    
    int hp;
    int maxHp;
    int stamina;
    int maxStamina;
    int atk;
    int def;
    int speed;
    int critRate;       // Tỉ lệ chí mạng (0 - 100%)
    float critDamage;   // Sát thương chí mạng (VD: 1.5f là x1.5 sát thương)
    
    float actionValue;      
    
    CombatEffect effects[MAX_EFFECTS];
    
    // [MỚI] 7 Kỹ năng: 
    // [0]: Đánh (Q) | [1]: Thủ (E) | [2]: Ulti (R) 
    // [3]->[6]: 4 Kỹ năng nằm trong Sách (W)
    CombatSkill skills[7];  
    int dmgTakenLastTurn;        // Lưu tổng sát thương nhận vào ở lượt trước
    int permanentDefBonus;       // Giáp vĩnh viễn cộng dồn từ Nội tại
    int permanentAtkBonus;       // [MỚI] ATK cộng dồn vĩnh viễn
    int permanentCritBonus;      // [MỚI] Chí mạng cộng dồn vĩnh viễn
    int staminaSpent;            // [MỚI] Tích lũy tiêu hao Thể lực
    bool isResilienceActive;     // Cờ báo hiệu đang bật mode "Quật Cường" (<40% máu)
} CombatEntity;

// --- 5. QUẢN LÝ TRẠNG THÁI TRẬN ĐẤU ---
typedef enum {
    COMBAT_STATE_INTRO,         // Chờ hiện quái
    COMBAT_STATE_CALC_TURN,     // Hệ thống tính toán xem ai được đi tiếp
    COMBAT_STATE_PLAYER_TURN,   // Đợi Player bấm nút
    COMBAT_STATE_ENEMY_TURN,    // Boss tự động đánh
    COMBAT_STATE_ACTION,        // Chiếu hoạt ảnh chém nhau
    COMBAT_STATE_VICTORY,       
    COMBAT_STATE_DEFEAT         
} CombatState;

// --- KHAI BÁO HÀM ---
void Combat_Init();
void Combat_Shutdown();

// phase: 0 (Hiệu trưởng), 1 (Phase 1 đánh 3), 2 (Phase 2 Quốc Trung)
void Combat_Start(Player *player, Npc *enemy, int phase); 

void Combat_Update();
void Combat_Draw();

bool Combat_IsActive();
int Combat_GetResult();     // Trả về: 0 (Đang đánh), 1 (Thắng), -1 (Thua)
void Combat_ResetResult();

#endif