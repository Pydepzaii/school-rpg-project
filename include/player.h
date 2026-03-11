#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "map.h"

typedef struct Npc Npc;

// Phân loại nghề nghiệp nhân vật (Dễ mở rộng sau này)
typedef enum {
    CLASS_STUDENT = 0,
    CLASS_WARRIOR,     
    CLASS_MAGE,        
    CLASS_ARCHER,

    //Combat bang mom
    CLASS_DAU_GAU,
    CLASS_HOC_BA,     
    CLASS_SOAI_CA,        
    CLASS_PHU_NHI_DAI       
} PlayerClass;

//hướng nhìn của player
typedef enum {
    FACE_DOWN = 0,
    FACE_UP,
    FACE_LEFT,
    FACE_RIGHT
} Direction;

// [MỚI] Chỉ số độc quyền cho hệ thống Hỏi Đáp (CBC)
typedef struct CBC_Stats {
    int hp;             // Máu (Lượt sai tối đa)
    int maxHp;          // Máu tối đa của hệ phái
    int skillUses;      // Số lượt dùng kỹ năng còn lại
    int comboCorrect;   // Đếm chuỗi trả lời đúng liên tiếp
    int comboWrong;     // Đếm chuỗi trả lời sai liên tiếp
    bool canRetry;      // Nội tại Học bá: Sai được chọn lại
    bool skipNext;      // Nội tại: Bỏ qua câu hỏi tiếp theo
} CBC_Stats;

// [QUAN TRỌNG] Giữ nguyên tên PlayerStats để gameplay.h không lỗi
typedef struct PlayerStats {
    int currentHp;     
    int maxHp;         
    int stamina;       // <--- [ĐỔI TÊN] Thể lực thay vì Mana
    int damage;        
    int magicPower;    // (Có thể bỏ qua biến này nếu không dùng)
    float moveSpeed;   
    int defense;       
    int expReward; 
    int storyProgress;    
} PlayerStats;

// Struct chưa skill (Dự phòng)
typedef struct {
    char name[30];
    int damage;    
    int staminaCost;   // <--- [ĐỔI TÊN]
    int cooldown;
} Skill;

typedef struct {
    Vector2 position;     // Tọa độ người chơi (X, Y)
    //bộ ảnh main
    Texture2D textureWalk;     // Chứa ảnh đi (main1walk.png)
    Texture2D textureIdle;     // Chứa ảnh đứng (main1idle.png)
    Texture2D *currentTexture; // Con trỏ thông minh: Lúc thì trỏ vào Walk, lúc thì trỏ vào Idle
    
    PlayerClass pClass;   // Nghề nghiệp
    PlayerStats stats;    // Chỉ số

    CBC_Stats cbcStats;   // Combat bang mom
    
    Skill skills[4]; 
    Direction currentDir;// lưu hướng hiện tại    
    //Vẽ ảnh toàn cục
    float drawWidth;  // Kích thước vẽ ra màn hình
    float drawHeight; // Kích thước vẽ ra màn hình
    //Debug only
    float hitWidth;   // Chiều rộng va chạm
    float hitHeight;  // Chiều cao va chạm (dẹt)
    //render support
    float paddingBottom; // Khoảng cách từ đáy ảnh lên chân (để Renderer sắp xếp lớp vẽ)
    // --- ANIMATION CONTROLLER ---
    Rectangle frameRec;   // Khung hình chữ nhật đang cắt từ ảnh gốc
    int currentFrame;     // Số thứ tự frame hiện tại (0, 1, 2...)
    int framesCounter;    // Đếm frame để điều chỉnh tốc độ
    int framesSpeed;      // Tốc độ chuyển động ảnh
    int spriteWidth;      // Chiều rộng 1 frame đơn lẻ
    int spriteHeight;     // Chiều cao 1 frame đơn lẻ
    int maxFrames; // Số frame tối đa của hành động hiện tại
} Player;

void InitPlayer(Player *player, PlayerClass chosenClass); 

// Hàm Update cần Map và NPC list để kiểm tra va chạm không đi xuyên qua được
void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount);

void DrawPlayer(Player *player);        
void UnloadPlayer(Player *player);      

#endif