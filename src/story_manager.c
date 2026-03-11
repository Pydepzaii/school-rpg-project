#include "story_manager.h"
#include "dialog_system.h"
#include "inventory.h"
#include "interact.h"
#include "ui_style.h"
#include "raymath.h"
#include "audio_manager.h"
#include <stdio.h>
#include <string.h>
// --- BIẾN DÙNG CHO MINIGAME HACKING (CHAPTER 6) ---
char hackInput[7] = {0}; // Mảng chứa 6 ký tự + 1 ký tự kết thúc chuỗi
int hackInputLen = 0;

void Story_Update(Player *player, GameMap *map, Npc *npcList, int npcCount)
{
    if (map->currentMapID == MAP_TOA_ALPHA)
    {

        // --- 1. SETUP TỌA ĐỘ VÀ TRẠNG THÁI ---
        static Vector2 bangThanhTichPos = {420.0f, 230.0f};

        // Tọa độ chốt chặn (Cửa kính)
        static Vector2 gatePos = {26.0f, 159.0f}; // NHỚ CHỈNH LẠI BẰNG TOOL PHÍM 0 NHÉ
        static bool isGateOpened = false;
        static bool isInspectingGate = false; // [MỚI] Trạng thái đang đứng xem cửa

        if (player->stats.storyProgress >= 2)
            isGateOpened = true;

        Npc *coThuKy = NULL;
        Npc *thayTuan = NULL;
        for (int i = 0; i < npcCount; i++)
        {
            if (npcList[i].id == NPC_CO_THU_KY)
                coThuKy = &npcList[i];
            if (npcList[i].id == NPC_THAY_TUAN_VM)
                thayTuan = &npcList[i];
        }
        // --- 4. ĐIỀU PHỐI KỊCH BẢN & AUTO-TALK ---
        switch (player->stats.storyProgress)
        {
       case 0:
            // Ép về BEFORE_COMBAT nếu chưa vào combat
            if (coThuKy && !coThuKy->isTalking && strcmp(coThuKy->dialogKey, "COMBAT_IN") != 0) {
                strcpy(coThuKy->dialogKey, "BEFORE_COMBAT");
            }

            // Bắt sự kiện COMBAT_IN để tắt thoại và chờ bấm C
            if (coThuKy && strcmp(coThuKy->dialogKey, "COMBAT_IN") == 0) {
                coThuKy->isTalking = false; 
                if (IsKeyPressed(KEY_C)) {
                    player->stats.storyProgress = 1;
                    Inventory_SpawnItem(ITEM_2, player->position, map->currentMapID);
                    strcpy(coThuKy->dialogKey, "COMBAT_WON");
                    coThuKy->currentDialogLine = 0;
                    coThuKy->isTalking = true;
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }
            }
            break;

        case 1:
            if (coThuKy && !coThuKy->isTalking) {
                if (strcmp(coThuKy->dialogKey, "COMBAT_WON_2") != 0) strcpy(coThuKy->dialogKey, "COMBAT_WON");
            }
            
            if (thayTuan && !thayTuan->isTalking && strcmp(thayTuan->dialogKey, "COMBAT_IN") != 0) {
                strcpy(thayTuan->dialogKey, "BEFORE_COMBAT");
            }

            if (thayTuan && strcmp(thayTuan->dialogKey, "COMBAT_IN") == 0) {
                thayTuan->isTalking = false; 
                if (IsKeyPressed(KEY_C)) {
                    player->stats.storyProgress = 2;
                    Inventory_SpawnItem(ITEM_KEY_ALPHA, player->position, map->currentMapID);
                    strcpy(thayTuan->dialogKey, "COMBAT_WON");
                    thayTuan->currentDialogLine = 0;
                    thayTuan->isTalking = true;
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }
            }
            break;

        case 2:
            if (coThuKy && !coThuKy->isTalking)
                strcpy(coThuKy->dialogKey, "COMBAT_WON_2");
            if (thayTuan && !thayTuan->isTalking)
            {
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
        Npc *thayHung = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 3) thayChinh = &npcList[i];
            if (npcList[i].id == 4) thayHung = &npcList[i];
        }

        // --- 1. XỬ LÝ PHÍM C (AUTO-WIN COMBAT) VÀ TẮT THOẠI ---
        if (thayChinh && strcmp(thayChinh->dialogKey, "COMBAT_IN") == 0) {
            thayChinh->isTalking = false; // Tắt bảng thoại để nhường chỗ cho màn đen
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 3; // Thắng thầy Chính -> Lên phase 3
                strcpy(thayChinh->dialogKey, "COMBAT_WON");
                thayChinh->currentDialogLine = 0;
                thayChinh->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
        
        if (thayHung && strcmp(thayHung->dialogKey, "COMBAT_IN") == 0) {
            thayHung->isTalking = false;
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 4; // Thắng thầy Hùng -> Lên phase 4
               Inventory_SpawnItem(ITEM_KEY_BETA, player->position, map->currentMapID);
                strcpy(thayHung->dialogKey, "COMBAT_WON");
                thayHung->currentDialogLine = 0;
                thayHung->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- 2. ĐIỀU PHỐI LOGIC HỘI THOẠI THEO STORY PROGRESS ---
        switch (player->stats.storyProgress) {
            case 2: // Đang ở Phase tìm thầy Chính
                if (thayChinh && !thayChinh->isTalking && strcmp(thayChinh->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(thayChinh->dialogKey, "BEFORE_COMBAT");
                }
                if (thayHung && !thayHung->isTalking) {
                    strcpy(thayHung->dialogKey, "BEFORE_TALK_MR_CHINH");
                }
                break;
                
            case 3: // Đã thắng Thầy Chính -> Chuyển sang Thầy Hùng
                if (thayChinh && !thayChinh->isTalking) strcpy(thayChinh->dialogKey, "COMBAT_WON");
                
                if (thayHung && !thayHung->isTalking && strcmp(thayHung->dialogKey, "COMBAT_IN") != 0) {
                    // Nếu thầy Hùng đang kẹt ở câu cũ thì chuyển sang câu mới
                    if (strcmp(thayHung->dialogKey, "BEFORE_TALK_MR_CHINH") == 0) {
                        strcpy(thayHung->dialogKey, "BEFORE_COMBAT");
                    }
                }
                break;
                
            case 4: // Đã thắng Thầy Hùng -> Kết thúc map
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
        
        Npc *coDauBep = NULL;
        Npc *chuDauBep = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 6) coDauBep = &npcList[i]; // ID 6 là Cô Đầu Bếp
            if (npcList[i].id == 5) chuDauBep = &npcList[i]; // ID 5 là Chú Đầu Bếp
        }

        // --- 1. XỬ LÝ PHÍM C (AUTO-WIN COMBAT) VÀ RỚT CHÌA KHÓA ---
        if (coDauBep && strcmp(coDauBep->dialogKey, "COMBAT_IN") == 0) {
            coDauBep->isTalking = false; 
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 5; // Thắng Boss phụ -> Mở rào (Phase 5)
                strcpy(coDauBep->dialogKey, "COMBAT_WON");
                coDauBep->currentDialogLine = 0;
                coDauBep->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
        
        if (chuDauBep && strcmp(chuDauBep->dialogKey, "COMBAT_IN") == 0) {
            chuDauBep->isTalking = false;
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 6; // Thắng Boss chính -> Phase 6
                
                // [HIỆU ỨNG NHẶT ĐỒ] Rớt chìa khóa Thư Viện ngay dưới chân
                Inventory_SpawnItem(ITEM_KEY_DELTA, player->position, map->currentMapID); 
                
                strcpy(chuDauBep->dialogKey, "COMBAT_WON");
                chuDauBep->currentDialogLine = 0;
                chuDauBep->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- 2. ĐIỀU PHỐI LOGIC HỘI THOẠI THEO PHASE ---
        switch (player->stats.storyProgress) {
            case 4: // Mới vào Căng Tin -> Gặp Cô Đầu Bếp
                if (coDauBep && !coDauBep->isTalking && strcmp(coDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(coDauBep->dialogKey, "BEFORE_COMBAT");
                }
                if (chuDauBep && !chuDauBep->isTalking && strcmp(chuDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuDauBep->dialogKey, "BEFORE_COMBAT"); // Dù bị chặn nhưng lỡ có trick qua được thì vẫn chuẩn
                }
                break;
                
            case 5: // Đã thắng Cô -> Mở rào -> Gặp Chú Đầu Bếp
                if (coDauBep && !coDauBep->isTalking) strcpy(coDauBep->dialogKey, "COMBAT_WON");
                
                if (chuDauBep && !chuDauBep->isTalking && strcmp(chuDauBep->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuDauBep->dialogKey, "BEFORE_COMBAT");
                }
                break;
                
            case 6: // Đã thắng Chú -> Rớt Key Delta -> Xong Map
                if (coDauBep && !coDauBep->isTalking) strcpy(coDauBep->dialogKey, "COMBAT_WON");
                if (chuDauBep && !chuDauBep->isTalking) strcpy(chuDauBep->dialogKey, "COMBAT_WON");
                break;
        }
    }
    // ==============================================================
    // CHAPTER 4: THƯ VIỆN
    // ==============================================================
    else if (map->currentMapID == MAP_THU_VIEN) {
        
        Npc *chuLaoCong = NULL;
        Npc *coThuThu = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 7) chuLaoCong = &npcList[i]; // ID 7
            if (npcList[i].id == 8) coThuThu = &npcList[i];   // ID 8
        }

        static bool hasRemovedPapers = false;

        // --- 1. XỬ LÝ PHÍM C (AUTO-WIN) VÀ RỚT CHÌA KHÓA ---
        if (chuLaoCong && strcmp(chuLaoCong->dialogKey, "COMBAT_IN") == 0) {
            chuLaoCong->isTalking = false; 
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 7; // Thắng Lao Công -> Sang Phase 7
                strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                chuLaoCong->currentDialogLine = 0;
                chuLaoCong->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
        
        if (coThuThu && strcmp(coThuThu->dialogKey, "COMBAT_IN") == 0) {
            coThuThu->isTalking = false;
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 8; // Thắng Thủ Thư -> Phase 8
                
                // Rớt chìa khóa đặc biệt sang Tòa Beta
                Inventory_SpawnItem(ITEM_KEY_SPECIAL, player->position, map->currentMapID); 
                
                strcpy(coThuThu->dialogKey, "COMBAT_WON");
                coThuThu->currentDialogLine = 0;
                coThuThu->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

        // --- 2. ĐIỀU PHỐI LOGIC HỘI THOẠI THEO PHASE ---
        switch (player->stats.storyProgress) {
            case 6: // Mới vào Map -> Phải đánh Chú Lao Công
                if (chuLaoCong && !chuLaoCong->isTalking && strcmp(chuLaoCong->dialogKey, "COMBAT_IN") != 0) {
                    strcpy(chuLaoCong->dialogKey, "BEFORE_COMBAT");
                }
                
                // Nếu chưa đánh Lao Công mà dám ra gặp Thủ Thư -> Bị đuổi đi
                if (coThuThu && !coThuThu->isTalking) {
                    strcpy(coThuThu->dialogKey, "BEFORE_EVENT"); 
                }
                break;
                
            case 7: // Đã thắng Lao Công -> Quest Cô Thủ Thư
                if (chuLaoCong && !chuLaoCong->isTalking) strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                
                if (coThuThu) {
                    // Chuyển từ BEFORE_EVENT (đuổi đi) sang câu chào hỏi chính thức
                    if (!coThuThu->isTalking && strcmp(coThuThu->dialogKey, "BEFORE_EVENT") == 0) {
                        strcpy(coThuThu->dialogKey, "BEFORE_COMBAT");
                    }

                    // --- [HỆ THỐNG CHỐNG NÓI DỐI] ---
                    // Khi người chơi bấm "Em tìm đủ rồi", currentDialogLine sẽ nhảy sang 1 (Dòng: BẠN ĐƯA CHO CÔ THỦ THƯ...)
                    if (strcmp(coThuThu->dialogKey, "ACCEPT_QUEST") == 0 && coThuThu->currentDialogLine == 1) {
                        if (!hasRemovedPapers) {
                            // Thử thu hồi 3 tờ giấy trong túi (ITEM_1 x 3)
                            if (Inventory_RemoveItem(ITEM_NOTE_PAPER, 3)) {
                                hasRemovedPapers = true; // Đã đủ và đã thu hồi -> Cho đi tiếp mạch truyện
                            } else {
                                // KHÔNG ĐỦ GIẤY MÀ DÁM BẤM -> Bị bắt quả tang!
                                strcpy(coThuThu->dialogKey, "FAKE_COMPULATE");
                                coThuThu->currentDialogLine = 0;
                            }
                        }
                    }

                    // Reset biến khi đóng thoại để người chơi đi nhặt tiếp và quay lại trả
                    if (!coThuThu->isTalking) {
                        hasRemovedPapers = false;
                        
                        // Nếu vừa bị chửi xong, đóng thoại và mở lại -> Hiện bảng chọn như cũ
                        if (strcmp(coThuThu->dialogKey, "FAKE_COMPULATE") == 0) {
                            strcpy(coThuThu->dialogKey, "ACCEPT_QUEST");
                        }
                    }
                }
                break;
                
            case 8: // Đã thắng Thủ Thư -> Xong Map
                if (chuLaoCong && !chuLaoCong->isTalking) strcpy(chuLaoCong->dialogKey, "COMBAT_WON");
                if (coThuThu && !coThuThu->isTalking) strcpy(coThuThu->dialogKey, "COMBAT_WON");
                break;
        }
    }
    // ==============================================================
    // CHAPTER 5: TÒA BETA
    // ==============================================================
    else if (map->currentMapID == MAP_BETA) {
        Npc *chuLaoCong2 = NULL;
        Npc *thayAnh = NULL;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 9) chuLaoCong2 = &npcList[i];
            if (npcList[i].id == 10) thayAnh = &npcList[i];
        }

        if (chuLaoCong2 && strcmp(chuLaoCong2->dialogKey, "COMBAT_IN") == 0) {
            chuLaoCong2->isTalking = false; 
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 9; 
                strcpy(chuLaoCong2->dialogKey, "COMBAT_WON");
                chuLaoCong2->currentDialogLine = 0;
                chuLaoCong2->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
        
        if (thayAnh && strcmp(thayAnh->dialogKey, "COMBAT_IN") == 0) {
            thayAnh->isTalking = false;
            if (IsKeyPressed(KEY_C)) {
                player->stats.storyProgress = 10; 
                player->stats.currentHp = player->stats.maxHp; 
                Inventory_SpawnItem(ITEM_1, player->position, map->currentMapID); 
                
                strcpy(thayAnh->dialogKey, "COMBAT_WON");
                thayAnh->currentDialogLine = 0;
                thayAnh->isTalking = true;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }

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
                    
                    // AUTO-BẬT BẢN ĐỒ THEO DÒNG THOẠI
                    static int lastTriggeredLine = -1;
                    if (thayAnh->isTalking && strcmp(thayAnh->dialogKey, "COMBAT_WON") == 0) {
                        if (thayAnh->currentDialogLine == 4) { // Dòng: *thầy đưa cho mình...
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
            if (npcList[i].id == 12) thayHieuTruong = &npcList[i]; // ID 12
        }

        if (thayHieuTruong) {
            
            // --- 1. COMBAT AUTO-WIN ---
            if (strcmp(thayHieuTruong->dialogKey, "COMBAT_IN") == 0) {
                thayHieuTruong->isTalking = false; // Tạm tắt khung thoại
                if (IsKeyPressed(KEY_C)) {
                    player->stats.storyProgress = 11; // Đã xong phase combat
                    strcpy(thayHieuTruong->dialogKey, "AFTER_COMBAT");
                    thayHieuTruong->currentDialogLine = 0;
                    thayHieuTruong->isTalking = true; // Bật lại thoại
                    Audio_PlaySoundEffect(SFX_UI_CLICK);
                }
            }

            // --- 2. LOGIC MINIGAME HACKING ---
            if (strcmp(thayHieuTruong->dialogKey, "HACK_START") == 0) {
                thayHieuTruong->isTalking = false; // Tắt thoại để chơi minigame

                // Quét phím gõ từ bàn phím (Chỉ lấy số từ 0 -> 9)
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0') && (key <= '9') && (hackInputLen < 6)) {
                        hackInput[hackInputLen] = (char)key;
                        hackInputLen++;
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                    key = GetCharPressed(); // Lấy tiếp nếu gõ nhanh
                }

                // Chức năng nút XÓA (Backspace)
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (hackInputLen > 0) {
                        hackInputLen--;
                        hackInput[hackInputLen] = '\0';
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    }
                }

                // XỬ LÝ KHI ĐÃ NHẬP ĐỦ 6 SỐ
                if (hackInputLen == 6) {
                    // Mật khẩu cứng hiện tại là 123456 (Sẽ sửa sau theo 6 quyển sách)
                    if (strcmp(hackInput, "123456") == 0) {
                        // NHẬP ĐÚNG: CHẠY TIẾP CỐT TRUYỆN
                        player->stats.storyProgress = 12; // Phá đảo minigame
                        strcpy(thayHieuTruong->dialogKey, "AFTER_HACKING");
                        thayHieuTruong->currentDialogLine = 0;
                        thayHieuTruong->isTalking = true;
                        
                        // Reset mảng để lần sau chơi lại không bị lỗi
                        hackInputLen = 0; memset(hackInput, 0, sizeof(hackInput));
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                    } else {
                        // NHẬP SAI: XÓA TRẮNG BẮT NHẬP LẠI TỪ ĐẦU
                        hackInputLen = 0; memset(hackInput, 0, sizeof(hackInput));
                        // (Có thể thêm âm thanh Error vào đây sau)
                    }
                }
            }
        }
        // --- 3. ĐIỀU PHỐI LOGIC HỘI THOẠI THEO PHASE ---
            switch (player->stats.storyProgress) {
                case 10: // Mới vào Lab -> Mở màn nói chuyện
                    if (!thayHieuTruong->isTalking && strcmp(thayHieuTruong->dialogKey, "COMBAT_IN") != 0 && strcmp(thayHieuTruong->dialogKey, "HACK_START") != 0) {
                        strcpy(thayHieuTruong->dialogKey, "BEFORE_COMBAT");
                    }
                    break;
                    
                case 11: // Đã đánh xong -> Chờ bấm lệnh Hack
                    if (!thayHieuTruong->isTalking && strcmp(thayHieuTruong->dialogKey, "HACK_START") != 0) {
                        strcpy(thayHieuTruong->dialogKey, "AFTER_COMBAT");
                    }
                    break;
                    
                case 12: // Đã Hack thành công -> Xem kết cục
                    if (!thayHieuTruong->isTalking) {
                        strcpy(thayHieuTruong->dialogKey, "AFTER_HACKING");
                    }
                    break;
            }
    }
}