#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "map.h"
#include "npc.h"

// Phân loại nghề nghiệp nhân vật (Dễ mở rộng sau này)
typedef enum {
    CLASS_STUDENT = 0,
    CLASS_WARRIOR,     
    CLASS_MAGE,        
    CLASS_ARCHER       
} PlayerClass;
//hướng nhìn của player
typedef enum {
    FACE_DOWN = 0,
    FACE_UP,
    FACE_LEFT,
    FACE_RIGHT
} Direction;
// Chứa toàn bộ chỉ số sức mạnh
typedef struct {
    int hp;            // Máu
    int maxHp;         
    int mana;          
    int damage;        
    int magicPower;    
    float moveSpeed;   // Tốc độ chạy (Pixel/Frame)
} PlayerStats;

// Struct chưa skill (Dự phòng)
typedef struct {
    char name[30];
    int manaCost;
    int cooldown;
} Skill;

typedef struct {
    Vector2 position;     // Tọa độ người chơi (X, Y)
    Texture2D texture;    // Ảnh Sprite Sheet nhân vật
    
    PlayerClass pClass;   // Nghề nghiệp
    PlayerStats stats;    // Chỉ số
    Skill skills[4]; 
    Direction currentDir;// lưu hướng hiện tại    
    //Vẽ ảnh toàn cục
    float drawWidth;  // Kích thước vẽ ra màn hình
    float drawHeight; // Kích thước vẽ ra màn hình
    //Debug only
    float hitWidth;   // Chiều rộng va chạm
    float hitHeight;  // Chiều cao va chạm (dẹt)
    // --- ANIMATION CONTROLLER ---
    Rectangle frameRec;   // Khung hình chữ nhật đang cắt từ ảnh gốc
    int currentFrame;     // Số thứ tự frame hiện tại (0, 1, 2...)
    int framesCounter;    // Đếm frame để điều chỉnh tốc độ
    int framesSpeed;      // Tốc độ chuyển động ảnh
    int spriteWidth;      // Chiều rộng 1 frame đơn lẻ
    int spriteHeight;     // Chiều cao 1 frame đơn lẻ

} Player;

void InitPlayer(Player *player, PlayerClass chosenClass); 

// Hàm Update cần Map và NPC list để kiểm tra va chạm không đi xuyên qua được
void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount);

void DrawPlayer(Player *player);        
void UnloadPlayer(Player *player);      

#endif