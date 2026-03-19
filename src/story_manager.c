#include "story_manager.h"
#include "dialog_system.h"
#include "inventory.h"
#include "interact.h"
#include "ui_style.h"
#include "raymath.h"
#include "audio_manager.h"
#include "combatbychatting.h"
#include "combat.h"
#include "gameplay.h"
#include <stdio.h>
#include <string.h>


extern int Inventory_GetItemCount(ItemID id);
// --- BIẾN DÙNG CHO MINIGAME HACKING (CHAPTER 6) ---
char hackInput[7] = {0};
int hackInputLen = 0;
bool isPointOfNoReturn = false; // [MỚI] Cờ khóa vĩnh viễn nút ESC
void Story_Update(Player *player, GameMap *map, Npc *npcList, int npcCount)
{
    // ==============================================================
    // CHAPTER 1: TÒA ALPHA
    // ==============================================================
    if (map->currentMapID == MAP_TOA_ALPHA)
    {
        static Vector2 bangThanhTichPos = {420.0f, 230.0f};
        static Vector2 gatePos = {26.0f, 159.0f};
        static bool isGateOpened = false;
        static bool isInspectingGate = false;

        if (player->stats.storyProgress >= 2)
            isGateOpened = true;

        Npc *coThuKy  = NULL;
        Npc *thayTuan = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == NPC_CO_THU_KY)   coThuKy  = &npcList[i];
            if (npcList[i].id == NPC_THAY_TUAN_VM) thayTuan = &npcList[i];
        }

        // --- CBC COMBAT: Cô Thư Ký ---
        if (coThuKy && strcmp(coThuKy->dialogKey, "COMBAT_IN") == 0) {
            coThuKy->isTalking = false;
            if (!CBC_IsActive() && !coThuKy->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(coThuKy->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, coThuKy);
                }
            }
            if (coThuKy->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 1;
                Inventory_SpawnItem(ITEM_2, player->position, map->currentMapID);
                strcpy(coThuKy->dialogKey, "COMBAT_WON");
                coThuKy->currentDialogLine = 0;
                coThuKy->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- CBC COMBAT: Thầy Tuấn ---
        if (thayTuan && strcmp(thayTuan->dialogKey, "COMBAT_IN") == 0) {
            thayTuan->isTalking = false;
            if (!CBC_IsActive() && !thayTuan->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(thayTuan->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, thayTuan);
                }
            }
            if (thayTuan->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 2;
                Inventory_SpawnItem(ITEM_KEY_ALPHA, player->position, map->currentMapID);
                strcpy(thayTuan->dialogKey, "COMBAT_WON");
                thayTuan->currentDialogLine = 0;
                thayTuan->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- ĐIỀU PHỐI KỊCH BẢN ---
        switch (player->stats.storyProgress) {
            case 0:
                if (coThuKy && !coThuKy->isTalking && strcmp(coThuKy->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(coThuKy->dialogKey, "BEFORE_COMBAT");
                }
                break;

            case 1:
                if (coThuKy && !coThuKy->isTalking) {
                    if (strcmp(coThuKy->dialogKey, "COMBAT_WON_2") != 0) strcpy(coThuKy->dialogKey, "COMBAT_WON");
                }
                if (thayTuan && !thayTuan->isTalking && strcmp(thayTuan->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(thayTuan->dialogKey, "BEFORE_COMBAT");
                }
                break;

            case 2:
                if (coThuKy && !coThuKy->isTalking)
                    strcpy(coThuKy->dialogKey, "COMBAT_WON_2");
                if (thayTuan && !thayTuan->isTalking) {
                    if (strcmp(thayTuan->dialogKey, "MORE_INFOR") != 0)
                        strcpy(thayTuan->dialogKey, "COMBAT_WON_2");
                }
                break;
        }
    }
    // ==============================================================
    // CHAPTER 2: NHÀ VÕ
    // ==============================================================
    else if (map->currentMapID == MAP_NHA_VO) {

        Npc *thayChinh = NULL;
        Npc *thayHung  = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 3) thayChinh = &npcList[i];
            if (npcList[i].id == 4) thayHung  = &npcList[i];
        }

        // --- CBC COMBAT: Thầy Chính ---
        if (thayChinh && strcmp(thayChinh->dialogKey, "COMBAT_IN") == 0) {
            thayChinh->isTalking = false;
            if (!CBC_IsActive() && !thayChinh->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(thayChinh->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, thayChinh);
                }
            }
            if (thayChinh->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 3;
                strcpy(thayChinh->dialogKey, "COMBAT_WON");
                thayChinh->currentDialogLine = 0;
                thayChinh->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- CBC COMBAT: Thầy Hùng ---
        if (thayHung && strcmp(thayHung->dialogKey, "COMBAT_IN") == 0) {
            thayHung->isTalking = false;
            if (!CBC_IsActive() && !thayHung->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(thayHung->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, thayHung);
                }
            }
            if (thayHung->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 4;
                Inventory_SpawnItem(ITEM_KEY_BETA, player->position, map->currentMapID);
                strcpy(thayHung->dialogKey, "COMBAT_WON");
                thayHung->currentDialogLine = 0;
                thayHung->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- ĐIỀU PHỐI LOGIC ---
        switch (player->stats.storyProgress) {
            case 2:
                if (thayChinh && !thayChinh->isTalking && strcmp(thayChinh->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(thayChinh->dialogKey, "BEFORE_COMBAT");
                }
                if (thayHung && !thayHung->isTalking) {
                    strcpy(thayHung->dialogKey, "BEFORE_TALK_MR_CHINH");
                }
                break;

            case 3:
                if (thayChinh && !thayChinh->isTalking) strcpy(thayChinh->dialogKey, "COMBAT_WON");
                if (thayHung && !thayHung->isTalking && strcmp(thayHung->dialogKey, "COMBAT_IN") != 0) {
                    if (strcmp(thayHung->dialogKey, "BEFORE_TALK_MR_CHINH") == 0 || strcmp(thayHung->dialogKey, "DEFAULT") == 0) {
                        strcpy(thayHung->dialogKey, "BEFORE_COMBAT");
                    }
                }
                break;

            case 4:
                if (thayChinh && !thayChinh->isTalking) strcpy(thayChinh->dialogKey, "COMBAT_WON");
                if (thayHung && !thayHung->isTalking) {
                    if (strcmp(thayHung->dialogKey, "EXIT_DIALOG") != 0) strcpy(thayHung->dialogKey, "COMBAT_WON");
                }
                break;
        }
    }
    // ==============================================================
    // CHAPTER 3: CĂNG TIN
    // ==============================================================
    else if (map->currentMapID == MAP_NHA_AN) {

        Npc *coDauBep  = NULL;
        Npc *chuDauBep = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 6) coDauBep  = &npcList[i];
            if (npcList[i].id == 5) chuDauBep = &npcList[i];
        }

        // --- CBC COMBAT: Cô Đầu Bếp ---
        if (coDauBep && strcmp(coDauBep->dialogKey, "COMBAT_IN") == 0) {
            coDauBep->isTalking = false;
            if (!CBC_IsActive() && !coDauBep->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(coDauBep->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, coDauBep);
                }
            }
            if (coDauBep->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 5;
                strcpy(coDauBep->dialogKey, "COMBAT_WON");
                coDauBep->currentDialogLine = 0;
                coDauBep->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- CBC COMBAT: Chú Đầu Bếp ---
        if (chuDauBep && strcmp(chuDauBep->dialogKey, "COMBAT_IN") == 0) {
            chuDauBep->isTalking = false;
            if (!CBC_IsActive() && !chuDauBep->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(chuDauBep->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, chuDauBep);
                }
            }
            if (chuDauBep->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 6;
                Inventory_SpawnItem(ITEM_KEY_DELTA, player->position, map->currentMapID);
                strcpy(chuDauBep->dialogKey, "COMBAT_WON");
                chuDauBep->currentDialogLine = 0;
                chuDauBep->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- ĐIỀU PHỐI LOGIC ---
        switch (player->stats.storyProgress) {
            case 4:
                if (coDauBep && !coDauBep->isTalking && strcmp(coDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(coDauBep->dialogKey, "BEFORE_COMBAT");
                }
                if (chuDauBep && !chuDauBep->isTalking && strcmp(chuDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuDauBep->dialogKey, "BEFORE_COMBAT");
                }
                break;

            case 5:
                if (coDauBep && !coDauBep->isTalking) strcpy(coDauBep->dialogKey, "COMBAT_WON");
                if (chuDauBep && !chuDauBep->isTalking && strcmp(chuDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuDauBep->dialogKey, "BEFORE_COMBAT");
                }
                break;

            case 6:
                if (coDauBep  && !coDauBep->isTalking)  strcpy(coDauBep->dialogKey,  "COMBAT_WON");
                if (chuDauBep && !chuDauBep->isTalking) strcpy(chuDauBep->dialogKey, "COMBAT_WON");
                break;
        }
    }
    // ==============================================================
    // CHAPTER 4: THƯ VIỆN
    // ==============================================================
    else if (map->currentMapID == MAP_THU_VIEN) {

        Npc *chuLaoCong = NULL;
        Npc *coThuThu   = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 7) chuLaoCong = &npcList[i];
            if (npcList[i].id == 8) coThuThu   = &npcList[i];
        }

        

        // --- CBC COMBAT: Chú Lao Công ---
        if (chuLaoCong && strcmp(chuLaoCong->dialogKey, "COMBAT_IN") == 0) {
            chuLaoCong->isTalking = false;
            if (!CBC_IsActive() && !chuLaoCong->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(chuLaoCong->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, chuLaoCong);
                }
            }
            if (chuLaoCong->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 7;
                strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                chuLaoCong->currentDialogLine = 0;
                chuLaoCong->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- CBC COMBAT: Cô Thủ Thư ---
        if (coThuThu && strcmp(coThuThu->dialogKey, "COMBAT_IN") == 0) {
            coThuThu->isTalking = false;
            if (!CBC_IsActive() && !coThuThu->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(coThuThu->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, coThuThu);
                }
            }
            if (coThuThu->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 8;
                Inventory_RemoveItem(ITEM_NOTE_PAPER, 3);
                Inventory_SpawnItem(ITEM_KEY_SPECIAL, player->position, map->currentMapID);
                strcpy(coThuThu->dialogKey, "COMBAT_WON");
                coThuThu->currentDialogLine = 0;
                coThuThu->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- ĐIỀU PHỐI LOGIC ---
        switch (player->stats.storyProgress) {
            case 6:
                if (chuLaoCong && !chuLaoCong->isTalking && strcmp(chuLaoCong->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuLaoCong->dialogKey, "BEFORE_COMBAT");
                }
                if (coThuThu && !coThuThu->isTalking) {
                    strcpy(coThuThu->dialogKey, "BEFORE_EVENT");
                }
                break;

           case 7:
                if (chuLaoCong && !chuLaoCong->isTalking) strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                if (coThuThu) {
                    // Bắt trạng thái DEFAULT do Load Save hoặc chuyển Map
                    if (!coThuThu->isTalking && (strcmp(coThuThu->dialogKey, "BEFORE_EVENT") == 0 || strcmp(coThuThu->dialogKey, "DEFAULT") == 0)) {
                        strcpy(coThuThu->dialogKey, "BEFORE_COMBAT");
                    }
                    
                    // [ĐÃ FIX TRÁNH SOFTLOCK] Chỉ đếm xem đủ giấy chưa, KHÔNG xóa giấy
                    if (strcmp(coThuThu->dialogKey, "ACCEPT_QUEST") == 0 && coThuThu->currentDialogLine == 1) {
                        if (Inventory_GetItemCount(ITEM_NOTE_PAPER) < 3) {
                            strcpy(coThuThu->dialogKey, "FAKE_COMPULATE");
                            coThuThu->currentDialogLine = 0;
                        }
                    }
                    
                    // Reset lại thoại nếu bị mắng do thiếu giấy
                    if (!coThuThu->isTalking) {
                        if (strcmp(coThuThu->dialogKey, "FAKE_COMPULATE") == 0) {
                            strcpy(coThuThu->dialogKey, "ACCEPT_QUEST");
                        }
                    }
                }
                break;  

            case 8:
                if (chuLaoCong && !chuLaoCong->isTalking) strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                if (coThuThu   && !coThuThu->isTalking)   strcpy(coThuThu->dialogKey,   "COMBAT_WON");
                break;
        }
    }
    // ==============================================================
    // CHAPTER 5: TÒA BETA
    // ==============================================================
    else if (map->currentMapID == MAP_BETA) {

        Npc *chuLaoCong2 = NULL;
        Npc *thayAnh     = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 9)  chuLaoCong2 = &npcList[i];
            if (npcList[i].id == 10) thayAnh     = &npcList[i];
        }

        // --- CBC COMBAT: Chú Lao Công 2 ---
        if (chuLaoCong2 && strcmp(chuLaoCong2->dialogKey, "COMBAT_IN") == 0) {
            chuLaoCong2->isTalking = false;
            if (!CBC_IsActive() && !chuLaoCong2->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(chuLaoCong2->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, chuLaoCong2);
                }
            }
            if (chuLaoCong2->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 9;
                strcpy(chuLaoCong2->dialogKey, "COMBAT_WON");
                chuLaoCong2->currentDialogLine = 0;
                chuLaoCong2->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- CBC COMBAT: Thầy Anh ---
        if (thayAnh && strcmp(thayAnh->dialogKey, "COMBAT_IN") == 0) {
            thayAnh->isTalking = false;
            if (!CBC_IsActive() && !thayAnh->isDead) {
                if (CBC_IsJustLost()) {
                    strcpy(thayAnh->dialogKey, "BEFORE_COMBAT");
                } else {
                    CBC_Start(player, thayAnh);
                }
            }
            if (thayAnh->isDead && !CBC_IsActive()) {
                player->stats.storyProgress = 10;
                player->stats.currentHp = player->stats.maxHp;
                Inventory_SpawnItem(ITEM_1, player->position, map->currentMapID);
                strcpy(thayAnh->dialogKey, "COMBAT_WON");
                thayAnh->currentDialogLine = 0;
                thayAnh->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- ĐIỀU PHỐI LOGIC ---
        switch (player->stats.storyProgress) {
            case 8:
                if (chuLaoCong2 && !chuLaoCong2->isTalking && strcmp(chuLaoCong2->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuLaoCong2->dialogKey, "BEFORE_COMBAT");
                }
                if (thayAnh && !thayAnh->isTalking) strcpy(thayAnh->dialogKey, "BEFORE_COMBAT");
                break;

            case 9:
                if (chuLaoCong2 && !chuLaoCong2->isTalking) strcpy(chuLaoCong2->dialogKey, "COMBAT_WON");
                if (thayAnh && !thayAnh->isTalking && strcmp(thayAnh->dialogKey, "COMBAT_IN") != 0) {
                    if (strcmp(thayAnh->dialogKey, "MORE_INFOR") != 0 && strcmp(thayAnh->dialogKey, "BEFORE_COMBAT_2") != 0) {
                        strcpy(thayAnh->dialogKey, "BEFORE_COMBAT");
                    }
                }
                break;

            case 10:
                if (chuLaoCong2 && !chuLaoCong2->isTalking) strcpy(chuLaoCong2->dialogKey, "COMBAT_WON");
                if (thayAnh) {
                    if (!thayAnh->isTalking) strcpy(thayAnh->dialogKey, "COMBAT_WON");
                    static int lastTriggeredLine = -1;
                    if (thayAnh->isTalking && strcmp(thayAnh->dialogKey, "COMBAT_WON") == 0) {
                        if (thayAnh->currentDialogLine == 4) {
                            if (lastTriggeredLine != 4) {
                                Inventory_ShowSecretMap();
                                lastTriggeredLine = 4;
                            }
                        } else lastTriggeredLine = thayAnh->currentDialogLine;
                    } else lastTriggeredLine = -1;
                }
                break;
        }
    }
    // ==============================================================
    // CHAPTER 6: PHÒNG LAB (TRẬN CHIẾN CUỐI CÙNG & HACKING)
    // ==============================================================
    else if (map->currentMapID == MAP_LAB) {

        Npc *thayHieuTruong = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 12) thayHieuTruong = &npcList[i];
        }

        if (thayHieuTruong) {

            // --- CBC COMBAT: Thầy Hiệu Trưởng ---
            if (strcmp(thayHieuTruong->dialogKey, "COMBAT_IN") == 0) {
                thayHieuTruong->isTalking = false;
                if (!CBC_IsActive() && !thayHieuTruong->isDead) {
                    if (CBC_IsJustLost()) {
                        strcpy(thayHieuTruong->dialogKey, "BEFORE_COMBAT");
                    } else {
                        CBC_Start(player, thayHieuTruong);
                    }
                }
                if (thayHieuTruong->isDead && !CBC_IsActive()) {
                    player->stats.storyProgress = 11;
                    strcpy(thayHieuTruong->dialogKey, "AFTER_COMBAT");
                    thayHieuTruong->currentDialogLine = 0;
                    thayHieuTruong->isTalking = true;
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }
            }

            // --- MINIGAME HACKING (Giữ nguyên) ---
            if (strcmp(thayHieuTruong->dialogKey, "HACK_START") == 0) {
                thayHieuTruong->isTalking = false;
               isPointOfNoReturn = true; // [MỚI] BẬT KHÓA ESC VĨNH VIỄN TỪ GIÂY PHÚT NÀY

                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0') && (key <= '9') && (hackInputLen < 6)) {
                        hackInput[hackInputLen] = (char)key;
                        hackInputLen++;
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (hackInputLen > 0) {
                        hackInputLen--;
                        hackInput[hackInputLen] = '\0';
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                }

                if (hackInputLen == 6) {
                    if (strcmp(hackInput, "367183") == 0) {
                        
                        player->stats.storyProgress = 12;
                        strcpy(thayHieuTruong->dialogKey, "AFTER_HACKING");
                        thayHieuTruong->currentDialogLine = 0;
                        thayHieuTruong->isTalking = true;
                        hackInputLen = 0; memset(hackInput, 0, sizeof(hackInput));
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    } else {
                        // [MỚI] NHẬP SAI -> CHUYỂN SANG ENDING HẮC ÁM (PROGRESS = 20)
                        player->stats.storyProgress = 20;
                        // ĐÓNG BĂNG CAMERA TẠI CHỖ ĐỂ XEM TOÀN CẢNH
                        extern bool isDarkEndingCutscene;
                        isDarkEndingCutscene = true; 
                        strcpy(thayHieuTruong->dialogKey, "HACK_ERROR"); // Kích hoạt màn hình Đỏ
                        thayHieuTruong->currentDialogLine = 0;
                        hackInputLen = 0; memset(hackInput, 0, sizeof(hackInput));
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                }
            }

            // --- ĐIỀU PHỐI LOGIC ---
            switch (player->stats.storyProgress) {
                case 10:
                    if (!thayHieuTruong->isTalking &&
                        strcmp(thayHieuTruong->dialogKey, "COMBAT_IN") != 0 &&
                        strcmp(thayHieuTruong->dialogKey, "HACK_START") != 0) {
                        strcpy(thayHieuTruong->dialogKey, "BEFORE_COMBAT");
                    }
                    break;

                case 11:
                    if (!thayHieuTruong->isTalking && strcmp(thayHieuTruong->dialogKey, "HACK_START") != 0) {
                        strcpy(thayHieuTruong->dialogKey, "AFTER_COMBAT");
                    }
                    break;

               case 12:
                    if (!thayHieuTruong->isTalking) {
                        // Nếu chưa chuyển thoại AFTER_HACKING thì chuyển
                        if (strcmp(thayHieuTruong->dialogKey, "AFTER_HACKING") != 0) {
                            strcpy(thayHieuTruong->dialogKey, "AFTER_HACKING");
                            thayHieuTruong->currentDialogLine = 0;
                            thayHieuTruong->isTalking = true;
                        } else {
                            // Đã đọc xong thoại AFTER_HACKING -> PHÁ ĐẢO GAME!
                           // Đã đọc xong thoại AFTER_HACKING -> PHÁ ĐẢO GAME!
                            Gameplay_StartEnding(ENDING_TRUE);
                            player->stats.storyProgress = 13;
                             // Khóa mạch truyện lại
                           
                        }
                    }
                    break;
                // ... (Các case 10, 11, 12 cũ giữ nguyên)

            case 13: // TRUE ENDING ĐÃ HOÀN THÀNH
            case 20: // DARK ENDING ĐÃ HOÀN THÀNH
            {
                // Kéo 2 cờ trạng thái Ending từ file gameplay.c sang
                extern bool isDarkEndingCutscene;
                extern bool Gameplay_IsEnding();

                // [FIX LỖI KẸT GAME]: CHỈ dọn dẹp NPC khi KHÔNG PHẢI đang chiếu Cutscene.
                // Điều này đảm bảo khi vừa Hack xong, NPC vẫn ở lại để diễn nốt kịch bản.
                // Khi bạn thoát game và Load Save lại, cờ Cutscene = false, NPC mới bị xóa.
                if (!isDarkEndingCutscene && !Gameplay_IsEnding()) {
                    for (int i = 0; i < npcCount; i++) {
                        if (npcList[i].id == 12 || npcList[i].id == 11 || npcList[i].id == 14 || npcList[i].id == 15) {
                            npcList[i].mapID = -1; // Tống cổ khỏi phòng
                            npcList[i].position = (Vector2){-1000, -1000};
                            npcList[i].isTalking = false;
                        }
                    }
                }
                break;
            }
        }
           // ==============================================================
            // [MỚI] GỌI COMBAT THẬT (ATB) VÀ NHẬN KẾT QUẢ
            // ==============================================================
            if (map->currentMapID == MAP_LAB) {
                // 1. KIỂM TRA ĐỂ BẬT COMBAT
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].isTalking) {
                        // Phase 1: Đánh 3 người
                        if (strcmp(npcList[i].dialogKey, "COMBAT_IN_1") == 0) {
                            npcList[i].isTalking = false; // Tắt thoại để nhường màn hình
                            Combat_Start(player, &npcList[i], 1);
                        }
                        // Phase 2: Đánh Quốc Trung Hóa Điên
                        else if (npcList[i].id == 11 && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                            npcList[i].isTalking = false;
                            Combat_Start(player, &npcList[i], 2);
                        }
                        // Đánh Thầy Hiệu Trưởng
                        else if (npcList[i].id == 12 && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                            npcList[i].isTalking = false;
                            Combat_Start(player, &npcList[i], 0);
                        }
                    }
                }

               // 2. NHẬN KẾT QUẢ TỪ COMBAT THẬT TRẢ VỀ
                int res = Combat_GetResult();
                if (res != 0) {
                    for (int i = 0; i < npcCount; i++) {
                        
                        // --- Boss Quốc Trung Phase 1 ---
                        // [ĐÃ SỬA LỖI ĐỨT MẠCH]: Nhận diện cả Bảo Vệ 2 (ID 15) hoặc Quốc Trung (ID 11) đang cầm Key
                        if ((npcList[i].id == 11 || npcList[i].id == 15) && strcmp(npcList[i].dialogKey, "COMBAT_IN_1") == 0) {
                            
                            // 1. Tịch thu toàn bộ Key của 2 thằng bảo vệ để chống kẹt
                            for (int k=0; k<npcCount; k++) {
                                if (npcList[k].id == 14 || npcList[k].id == 15) {
                                    npcList[k].isTalking = false;
                                    strcpy(npcList[k].dialogKey, "DEFAULT");
                                }
                            }
                            
                            // 2. Nhét Mic lại vào tay Quốc Trung (ID 11) để nó nói tiếp
                            for (int k=0; k<npcCount; k++) {
                                if (npcList[k].id == 11) {
                                    if (res == 1) strcpy(npcList[k].dialogKey, "BOSSPHASE_2"); // Thắng
                                    else strcpy(npcList[k].dialogKey, "BAD_ENDING");           // Thua
                                    
                                    npcList[k].currentDialogLine = 0;
                                    npcList[k].isTalking = true;
                                }
                            }
                        }
                        // --- Boss Quốc Trung Phase 2 ---
                        else if (npcList[i].id == 11 && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                            if (res == 1) strcpy(npcList[i].dialogKey, "TRUE_ENDING");
                            else strcpy(npcList[i].dialogKey, "BAD_ENDING");
                            
                            npcList[i].currentDialogLine = 0;
                            npcList[i].isTalking = true;
                        }
                        // --- Thầy Hiệu Trưởng ---
                        else if (npcList[i].id == 12 && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                            if (res == 1) strcpy(npcList[i].dialogKey, "AFTER_COMBAT");
                            else strcpy(npcList[i].dialogKey, "BEFORE_COMBAT"); // Thua bắt đánh lại
                            
                            npcList[i].currentDialogLine = 0;
                            npcList[i].isTalking = true;
                        }
                    }
                    Combat_ResetResult(); // Quan trọng: Đã nhận kết quả xong thì phải Reset
                }

                // 3. BẮT CỜ ENDING TỪ FILE DIALOGS.TXT
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].id == 11 && npcList[i].isTalking) {
                      if (strcmp(npcList[i].dialogKey, "TRUE_END") == 0) {
                            npcList[i].isTalking = false; 
                            Gameplay_StartEnding(ENDING_DARK); // [MỚI] Phát True Ending
                        }
                        else if (strcmp(npcList[i].dialogKey, "AFTER_BAD_ENDING") == 0) {
                            npcList[i].isTalking = false; 
                            Gameplay_StartEnding(ENDING_BAD); // [MỚI] Phát Bad Ending
                        }
                    }
                }
            }
          // =========================================================
            // [MỚI] ĐIỀU PHỐI KỸ XẢO DARK ENDING (TỰ ĐỘNG THEO THOẠI)
            // =========================================================
            extern bool isSpawningDarkBoss;
            extern bool isBlackholeActive;
            extern float blackholeTimer;
            
            static bool hasTriggeredAura = false;
            static bool hasTriggeredBlackhole = false;
            static bool hasPassedMic = false; // [MỚI] Khóa chuyền mic tự động
            // [THÊM ĐOẠN NÀY VÀO NGAY BÊN DƯỚI]
            // Nếu không phải đang chiếu Cutscene (do vừa Load Save), ép toàn bộ biến về False!
            extern bool isDarkEndingCutscene;
            if (!isDarkEndingCutscene) {
                hasTriggeredAura = false;
                hasTriggeredBlackhole = false;
                hasPassedMic = false;
            }
            if (thayHieuTruong->isTalking && strcmp(thayHieuTruong->dialogKey, "HACK_FAIL") == 0) {
                // Dòng 2: Bật Aura
                if (thayHieuTruong->currentDialogLine == 2 && !hasTriggeredAura) {
                    hasTriggeredAura = true;
                    isSpawningDarkBoss = true; 
                    extern float darkBossSpawnTimer;
                    darkBossSpawnTimer = 0.0f;
                }
                // Dòng 4: Bật Hố Đen
                else if (thayHieuTruong->currentDialogLine == 4 && !hasTriggeredBlackhole) {
                    hasTriggeredBlackhole = true;
                    isBlackholeActive = true;
                    blackholeTimer = 0.0f;
                }
            }

            // [SỬA LỖI TÀN DƯ]: TỰ ĐỘNG XÓA THẦY VÀ CHUYỀN MIC SAU KHI NỔ HỐ ĐEN (2.5 Giây)
            if (hasTriggeredBlackhole && !isBlackholeActive && blackholeTimer > 2.5f && !hasPassedMic) {
                hasPassedMic = true; // Chốt lại, chỉ chạy 1 lần
                
                // 1. Tiêu diệt Hiệu Trưởng lập tức, không chờ bấm phím
                thayHieuTruong->mapID = -1; 
                thayHieuTruong->position = (Vector2){-1000, -1000};
                thayHieuTruong->isTalking = false; 

                // 2. Tự động bật Mic cho Boss Quốc Trung (ID 11) nhảy vào thoại luôn
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].id == 11) {
                        strcpy(npcList[i].dialogKey, "BEGIN_ENDING_2");
                        npcList[i].currentDialogLine = 0;
                        npcList[i].isTalking = true;
                        break;
                    }
                }
            }

            // CHỐT CHẶN CHUYỂN PHASE: Fix lỗi hiện dấu "..." và đứng game
            // Khi người chơi bấm vào lựa chọn "Hình như có ai đó sắp tới|BEGIN_ENDING_2"
            if (strcmp(thayHieuTruong->dialogKey, "BEGIN_ENDING_2") == 0 && thayHieuTruong->mapID != -1) {
                
                // 1. Thủ tiêu hoàn toàn xác của Thầy Hiệu Trưởng
                thayHieuTruong->mapID = -1; 
                thayHieuTruong->position = (Vector2){-1000, -1000};
                thayHieuTruong->isTalking = false; 

                // 2. Chuyển quyền phát ngôn (Mở mic) cho Boss Quốc Trung (ID 11)
                for (int i = 0; i < npcCount; i++) {
                    if (npcList[i].id == 11) {
                        strcpy(npcList[i].dialogKey, "BEGIN_ENDING_2");
                        npcList[i].currentDialogLine = 0;
                        npcList[i].isTalking = true;
                        break;
                    }
                }
            }
            // ==============================================================
            // [QUAN TRỌNG] TRẠM TRUNG CHUYỂN MIC (SỬA LỖI KẸT THOẠI QUỐC TRUNG)
            // ==============================================================
            int currentSpeakerID = -1;
            char currentKey[32] = "";
            
            for (int i = 0; i < npcCount; i++) {
                if (npcList[i].isTalking) {
                    currentSpeakerID = npcList[i].id;
                    strcpy(currentKey, npcList[i].dialogKey);
                    break;
                }
            }
            
            // 1. Quốc Trung (11) -> Bảo vệ 1 (14)
            if (currentSpeakerID == 11 && strcmp(currentKey, "MOVE_TO_BAOVE") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "MOVE_TO_BAOVE"); npcList[i].currentDialogLine = 0; }
            }
            // 2. Bảo vệ 1 (14) -> Bảo vệ 2 (15)
            else if (currentSpeakerID == 14 && strcmp(currentKey, "MOVE_TO_BAOVE2") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "MOVE_TO_BAOVE2"); npcList[i].currentDialogLine = 0; }
            }
            // 3. Bảo vệ 2 (15) -> Quốc Trung (11)
            else if (currentSpeakerID == 15 && strcmp(currentKey, "MOVE_TO_TRUNG") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "MOVE_TO_TRUNG"); npcList[i].currentDialogLine = 0; }
            }
            // 4. Quốc Trung (11) -> Bảo vệ 1 (Bắt lấy nó)
            else if (currentSpeakerID == 11 && strcmp(currentKey, "COMBAT_IN_WITH_BOSS") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "COMBAT_IN_WITH_BOSS"); npcList[i].currentDialogLine = 0; }
            }
            // 5. Bảo vệ 1 (14) -> Bảo vệ 2 (Lên luôn)
            else if (currentSpeakerID == 14 && strcmp(currentKey, "COMBAT_IN_WITH_BOSS_2") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "COMBAT_IN_WITH_BOSS_2"); npcList[i].currentDialogLine = 0; }
            }
            // 5.5 Bảo vệ 2 (15) -> Quốc Trung (11) (Nhường mic để bật Fake Combat Phase 1)
            else if (currentSpeakerID == 15 && strcmp(currentKey, "COMBAT_IN_1") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "COMBAT_IN_1"); npcList[i].currentDialogLine = 0; }
            }
            // 6. Quốc Trung (11) -> Bảo vệ 1 (Giết bảo vệ 1)
            else if (currentSpeakerID == 11 && strcmp(currentKey, "KILL_BAOVE") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "KILL_BAOVE"); npcList[i].currentDialogLine = 0; }
            }
            // 7. Bảo vệ 1 (14) -> Bảo vệ 2 (15) (Bảo vệ 2 hét lên)
            else if (currentSpeakerID == 14 && strcmp(currentKey, "KILL_BAOVE2") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 14) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "KILL_BAOVE2"); npcList[i].currentDialogLine = 0; }
            }
            // 8. Bảo vệ 2 (15) -> Quốc Trung (11) (Bảo vệ 2 chết, Trung cười)
            else if (currentSpeakerID == 15 && strcmp(currentKey, "BOSSPHASE_2_2") == 0) {
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 15) npcList[i].isTalking = false;
                for (int i=0; i<npcCount; i++) if (npcList[i].id == 11) { npcList[i].isTalking = true; strcpy(npcList[i].dialogKey, "BOSSPHASE_2_2"); npcList[i].currentDialogLine = 0; }
            }
        }
    }
}