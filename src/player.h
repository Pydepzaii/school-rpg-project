#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "map.h"
#include "npc.h"

// 1. Định nghĩa các Lớp Nhân Vật
typedef enum {
    CLASS_STUDENT = 0, // Cân bằng
    CLASS_WARRIOR,     // Máu trâu, đánh gần
    CLASS_MAGE,        // Máu giấy, đánh xa
    CLASS_ARCHER       // Tốc độ cao
} PlayerClass;

// 2. Struct chứa chỉ số (Stats)
typedef struct {
    int hp;            // Máu hiện tại
    int maxHp;         // Máu tối đa
    int mana;          // Năng lượng
    int damage;        // Sát thương vật lý
    int magicPower;    // Sát thương phép
    float moveSpeed;   // Tốc độ chạy
} PlayerStats;

// 3. Struct Skill (Dự phòng cho sau này)
typedef struct {
    char name[30];
    int manaCost;
    int cooldown;
} Skill;

// 4. Struct Player Chính
typedef struct {
    Vector2 position;     
    Texture2D texture;    
    
    // --- HỆ THỐNG CLASS & STATS MỚI ---
    PlayerClass pClass;   // Lớp nhân vật
    PlayerStats stats;    // Các chỉ số
    Skill skills[4];      // Mảng chứa 4 kỹ năng
    
    // --- ANIMATION ---
    Rectangle frameRec;   
    int currentFrame;     
    int framesCounter;    
    int framesSpeed;      
    int spriteWidth;      
    int spriteHeight;     

} Player;

// Khởi tạo Player cần biết chọn Class nào
void InitPlayer(Player *player, PlayerClass chosenClass); 

// Update cần biết Map và Danh sách NPC để xử lý va chạm
void UpdatePlayer(Player *player, GameMap *map, Npc *npcList, int npcCount);

void DrawPlayer(Player *player);        
void UnloadPlayer(Player *player);      

#endif