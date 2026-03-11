#ifndef STORY_MANAGER_H
#define STORY_MANAGER_H

#include "player.h"
#include "map.h"
#include "npc.h"

// Hàm kiểm tra và kích hoạt các sự kiện cốt truyện
void Story_Update(Player *player, GameMap *map, Npc *npcList, int npcCount);

#endif