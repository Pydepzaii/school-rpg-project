// FILE: src/interact.c
#include "interact.h"
#include "raymath.h" 
#include "ui_style.h" 
#include "settings.h" // Cần để dùng MAP_ID
#include "dialog_system.h"
#include "transition.h"
#include "ui_style.h"
#include <string.h>
#include "combat.h"
#include "debug.h"
#include <stdio.h>  // [THÊM MỚI] Để dùng printf debug
#include "combatbychatting.h"
#include "inventory.h"
#include "audio_manager.h"
#include "camera.h"

// [MỚI] Tọa độ cửa (Lấy từ code Dev)
// [CẦN CHÚ Ý]: Các biến này đang Hard-code (gán cứng). Nếu bạn thay đổi Map, nhớ vào đây sửa tọa độ cửa.
static Vector2 exitToBlack = { 78.0f, 300.0f };
static Vector2 exitToWhite = { 720.0f, 300.0f };
static Vector2 exitToBeta = { 462.0f, 187.0f };
static Vector2 exitToPhongLab = { 17.0f, 416.0f };
static Vector2 exitToThuVien = { 22.0f, 377.0f };
static Vector2 exitToNhaVo = { 600.0f,80.0f };
static Vector2 exitToCangtin = { 405.0f, 15.0f };
static Vector2 exitToAlpha = { 500.0f, 400.0f };
//nha alpha chặn
static bool isConfirmingNhaVo = false;
// [THÊM 3 DÒNG NÀY VÀO DƯỚI isConfirmingNhaVo]
static Vector2 gatePos = { 22.0f, 159.0f }; // Tọa độ chốt chặn
static bool isGateOpened = false;
static bool isInspectingGate = false;
static bool isTeleporting = false;
static float teleTimer = 0.0f;
static float teleCooldown = 0.0f;
static Vector2 teleSource = {0};
static Vector2 teleDest = {0};
static Vector2 elevatorDest = { 38.0f, 18.0f }; 
static Texture2D texTeleAura = {0};

// --- BIẾN CỦA NHÀ VÕ CHAPTER 2 ---
static bool isConfirmingCangTin = false; 
static bool isNotePicked = false; 
static Vector2 notePos = { 520.0f, 210.0f }; // TỌA ĐỘ MẢNH GIẤY (Dùng phím 0 để tự chỉnh lại nhé)

// --- BIẾN CỦA CĂNG TIN CHAPTER 3 ---
static Vector2 fencePos = { 60.0f, 335.0f }; // TỌA ĐỘ HÀNG RÀO CHẶN (Nhớ dùng phím 0 để đo lại)
static bool isFenceOpened = false;
static bool isInspectingFence = false;
static bool isConfirmingThuVien = false;

//CHAPTER 4 THU VIEN
// --- BIẾN CỦA THƯ VIỆN CHAPTER 4 ---
static bool isConfirmingBeta = false; // Biến kiểm tra bảng thông báo cửa Beta
static Vector2 paper1Pos = { 595.0f, 75.0f }; // Mảnh giấy 1 
static Vector2 paper2Pos = { 577.0f, 388.0f }; // Mảnh giấy 2 
static Vector2 paper3Pos = { 79.0f, 269.0f }; // Mảnh giấy 3
static bool isPaper1Picked = false;
static bool isPaper2Picked = false;
static bool isPaper3Picked = false;

// --- BIẾN CỦA TÒA BETA CHAPTER 5 ---
static bool isConfirmingLab = false;
static Vector2 betaGatePos = { 105.0f, 390.0f };      // Tọa độ ngoài cửa lớp
static Vector2 betaElevatorDest = { 105.0f, 240.0f }; // Tọa độ trong lớp
static bool isBetaGateOpened = false;
static bool isInspectingBetaGate = false; // Thêm biến kiểm tra xem có đang đọc thông báo không
static bool isTeleportingBeta = false;
static float teleTimerBeta = 0.0f;
static float teleCooldownBeta = 0.0f;
static Vector2 teleSourceBeta = {0};
static Vector2 teleDestBeta = {0};
// Hàm logic kiểm tra cửa
void Interact_CheckExits(Player *player, GameMap *map, Npc *npcList, int npcCount) {
   //LẤY HITBOX LÀM CHUẨN TÍNH KHOẢNG CÁCH TƯƠNG TÁC
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f), // Tâm X của hitbox
        player->position.y + offsetY + (player->hitHeight / 2.0f) // Tâm Y của hitbox
    };

    // [BƯỚC 2: DÙNG playerHitCenter ĐỂ TÍNH KHOẢNG CÁCH]
    if (map->currentMapID == MAP_THU_VIEN) {
        // Nhặt 3 mảnh giấy ẩn (Side Quest)
        // --- CHỈ CHO NHẶT GIẤY KHI ĐÃ NHẬN QUEST ---
        bool isQuestActive = false;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 8 && (strcmp(npcList[i].dialogKey, "ACCEPT_QUEST") == 0 || player->stats.storyProgress >= 8)) {
                isQuestActive = true; break;
            }
        }

        if (isQuestActive) {
            // Nhặt 3 mảnh giấy ẩn (Side Quest)
            if (!isPaper1Picked && Vector2Distance(playerHitCenter, paper1Pos) < INTERACT_DISTANCE) {
                if (IsKeyPressed(KEY_E)) { isPaper1Picked = true; Inventory_SpawnItem(ITEM_NOTE_PAPER, paper1Pos, map->currentMapID); }
            }
            if (!isPaper2Picked && Vector2Distance(playerHitCenter, paper2Pos) < INTERACT_DISTANCE) {
                if (IsKeyPressed(KEY_E)) { isPaper2Picked = true; Inventory_SpawnItem(ITEM_NOTE_PAPER, paper2Pos, map->currentMapID); }
            }
            if (!isPaper3Picked && Vector2Distance(playerHitCenter, paper3Pos) < INTERACT_DISTANCE) {
                if (IsKeyPressed(KEY_E)) { isPaper3Picked = true; Inventory_SpawnItem(ITEM_NOTE_PAPER, paper3Pos, map->currentMapID); }
            }
        }
        // --- LOGIC CỬA SANG TÒA BETA ---
        if (Vector2Distance(playerHitCenter, exitToBeta) < INTERACT_DISTANCE) {
            if (!isConfirmingBeta && IsKeyPressed(KEY_E)) isConfirmingBeta = true;
        } else {
            isConfirmingBeta = false;
        }

        // Cửa sang Map Đen
        if (Vector2Distance(playerHitCenter, exitToBlack) < INTERACT_DISTANCE) {
            if (IsKeyPressed(KEY_E)) {
                Transition_StartToMap(MAP_TRANG, (Vector2){ 400, 200 });
            }
        }
        // Cửa sang Map Trắng
        if (Vector2Distance(playerHitCenter, exitToWhite) < INTERACT_DISTANCE) {
            if (Transition_IsActive()) return;
            if (IsKeyPressed(KEY_E)) {
                Transition_StartToMap(MAP_DEN, (Vector2){ 400, 200 });
            }
        }
    }
    // --- CỬA CỦA NHÀ ALPHA ---
    else if (map->currentMapID == MAP_TOA_ALPHA) {
        
        // ==========================================
        // 1. LOGIC CỬA SANG NHÀ VÕ
        // ==========================================
        if (Vector2Distance(playerHitCenter, exitToNhaVo) < INTERACT_DISTANCE) {
            // Cho phép bấm E để bật bảng thoại DÙ CÓ CHÌA HAY KHÔNG
            if (!isConfirmingNhaVo && IsKeyPressed(KEY_E)) {
                isConfirmingNhaVo = true; 
            }
        } else {
            isConfirmingNhaVo = false; 
        }

        // ==========================================
        // 2. LOGIC CHỐT CHẶN THẦY TUẤN (CỬA KÍNH)
        // ==========================================
        if (player->stats.storyProgress >= 2) isGateOpened = true;

        if (!isGateOpened) {
            float dist = Vector2Distance(playerHitCenter, gatePos);
            
            if (dist < 60.0f) {
                // Bấm E để mở thoại
                if (!isInspectingGate && IsKeyPressed(KEY_E)) isInspectingGate = true;
                
                // Cố đâm vào lõi vật lý -> bị đẩy lùi (Không đẩy văng khỏi bán kính 60 nên thoại KHÔNG bị tắt)
                if (dist < 25.0f) {
                    player->position.y += 6.0f; 
                }
            } else {
                isInspectingGate = false; // Đi xa tự đóng thoại
            }

            // Xử lý click chuột cho Chốt chặn
            if (isInspectingGate) {
                if (Inventory_HasItem(ITEM_2)) {
                    Vector2 mousePos = GetVirtualMousePos();
                    Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                    Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };
                    
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                       if (CheckCollisionPointRec(mousePos, btnYes)) {
                            isGateOpened = true;      // MỞ CỬA!
                            isInspectingGate = false;
                            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                            Audio_PlaySoundEffect(SFX_UI_CLICK);
                            
                            // [MỚI] KÍCH HOẠT DỊCH CHUYỂN LẦN ĐẦU TIÊN
                            isTeleporting = true;
                            teleTimer = 0.0f;
                            teleCooldown = 1.0f;
                            teleSource = player->position;
                            teleDest = elevatorDest;
                        } else if (CheckCollisionPointRec(mousePos, btnNo)) {
                            isInspectingGate = false;
                            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        }
                    }
                } else {
                    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        isInspectingGate = false;
                    }
                }
            }
        }
        // ==========================================
        // LOGIC THỰC THI DỊCH CHUYỂN MA PHÁP
        // ==========================================
        if (teleCooldown > 0.0f) teleCooldown -= GetFrameTime();

        if (isTeleporting) {
            teleTimer += GetFrameTime();
            
            // Khóa di chuyển nhân vật bằng cách ép vị trí
            if (teleTimer < 1.0f) {
                player->position = teleSource; // Giữ nguyên ở điểm đầu
            } else if (teleTimer >= 1.0f && teleTimer < 2.0f) {
                player->position = teleDest;   // Giây thứ 1: Bắn sang điểm đích
            } else {
                isTeleporting = false; // Xong 2 giây -> Kết thúc
                teleCooldown = 0.5f;   // Chống kẹt teleport liên hoàn (bay qua bay lại mãi)
            }
        }
        
        // Cảm biến dẫm vào vòng phép (chỉ hoạt động khi cửa đã mở)
        if (isGateOpened && !isTeleporting && teleCooldown <= 0.0f) {
            if (Vector2Distance(playerHitCenter, gatePos) < 25.0f) {
                isTeleporting = true; teleTimer = 0.0f;
                teleSource = player->position; teleDest = elevatorDest;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            } else if (Vector2Distance(playerHitCenter, elevatorDest) < 25.0f) {
                isTeleporting = true; teleTimer = 0.0f;
                teleSource = player->position; teleDest = gatePos;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
    }
    // --- CỬA CỦA NHÀ VÕ ---
    else if (map->currentMapID == MAP_NHA_VO) {
        
        // 1. Tương tác Cửa Căng Tin (Cho phép bấm E mọi lúc)
        if (Vector2Distance(playerHitCenter, exitToCangtin) < INTERACT_DISTANCE) {
            if (!isConfirmingCangTin && IsKeyPressed(KEY_E)) isConfirmingCangTin = true;
        } else {
            isConfirmingCangTin = false;
        }
    }
    else if (map->currentMapID == MAP_NHA_AN) {
       // 1. Tương tác Cửa Thư Viện (Cho phép bấm E để check chìa khóa)
        if (Vector2Distance(playerHitCenter, exitToThuVien) < INTERACT_DISTANCE) {
            if (!isConfirmingThuVien && IsKeyPressed(KEY_E)) isConfirmingThuVien = true;
        } else {
            isConfirmingThuVien = false;
        }

        // 2. Logic Hàng Rào Chặn (Cô Đầu Bếp)
        if (player->stats.storyProgress >= 5) isFenceOpened = true;

        if (!isFenceOpened) {
            float distFence = Vector2Distance(playerHitCenter, fencePos);
            
            if (distFence < 60.0f) {
                // Bấm E để đọc biển báo
                if (!isInspectingFence && IsKeyPressed(KEY_E)) isInspectingFence = true;
                
                // Lõi vật lý đẩy lùi nhân vật khi cố đâm vào rào
                if (distFence < 30.0f) {
                    player->position.y += 6.0f; // Đẩy dội ra (Có thể đổi thành +/- x,y tùy hướng chặn)
                }
            } else {
                isInspectingFence = false;
            }

            if (isInspectingFence && (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                isInspectingFence = false;
            }
        }
    }
    // --- CỬA CỦA MAP BETA (CHAPTER 5) ---
    else if (map->currentMapID == MAP_BETA) {
        // 1. LOGIC TELEPORT CỬA LỚP BETA
        if (player->stats.storyProgress >= 9) isBetaGateOpened = true; // Thắng Lao Công 2

        if (!isBetaGateOpened) {
            float dist = Vector2Distance(playerHitCenter, betaGatePos);
            
            if (dist < 60.0f) {
                // Bấm E để mở bảng thông báo
                if (!isInspectingBetaGate && IsKeyPressed(KEY_E)) isInspectingBetaGate = true;

                // Lõi vật lý đẩy lùi (ngăn đi xuyên cửa)
                if (dist < 40.0f) player->position.y += 6.0f; 
            } else {
                isInspectingBetaGate = false; // Đi xa tự đóng thông báo
            }

            // Bấm Enter hoặc Click chuột để đóng thông báo
            if (isInspectingBetaGate && (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                isInspectingBetaGate = false;
            }
        }

        if (teleCooldownBeta > 0.0f) teleCooldownBeta -= GetFrameTime();

        if (isTeleportingBeta) {
            teleTimerBeta += GetFrameTime();
            if (teleTimerBeta < 1.0f) player->position = teleSourceBeta;
            else if (teleTimerBeta >= 1.0f && teleTimerBeta < 2.0f) player->position = teleDestBeta;
            else { isTeleportingBeta = false; teleCooldownBeta = 0.5f; }
        }

        if (isBetaGateOpened && !isTeleportingBeta && teleCooldownBeta <= 0.0f) {
            if (Vector2Distance(playerHitCenter, betaGatePos) < 25.0f) {
                isTeleportingBeta = true; teleTimerBeta = 0.0f;
                teleSourceBeta = player->position; teleDestBeta = betaElevatorDest;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            } else if (Vector2Distance(playerHitCenter, betaElevatorDest) < 25.0f) {
                isTeleportingBeta = true; teleTimerBeta = 0.0f;
                teleSourceBeta = player->position; teleDestBeta = betaGatePos;
                Audio_PlaySoundEffect(SFX_UI_CLICK);
            }
        }
        if (Vector2Distance(playerHitCenter, exitToPhongLab) < INTERACT_DISTANCE) {
            if (!isConfirmingLab && IsKeyPressed(KEY_E)) isConfirmingLab = true;
        } else {
            isConfirmingLab = false;
        }
    }
    // Các map phụ về thư viện
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
         if (IsKeyPressed(KEY_R)) {
            Transition_StartToMap(MAP_THU_VIEN, (Vector2){ 400, 250 });
         }
    }
}

//PHẠM VI TƯƠNG TÁC NPC
void Interact_Update(Player *player, Npc *npcList, int npcCount, GameMap *map) {
    //  lẤY HITBOX LÀM MỐC ĐỂ TÍNH TOÁN PHẠM VI TƯƠNG TÁC
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f),
        player->position.y + offsetY + (player->hitHeight / 2.0f)
    };

    // VÒNG LẶP] CHECK NPC CÓ CÙNG MAP KHÔNG
    for (int i = 0; i < npcCount; i++) {
        if (npcList[i].mapID != map->currentMapID) continue;
       // 1. Tính kích thước 1 khung hình của NPC
        float npcFrameW = (float)npcList[i].texture.width / npcList[i].frameCount;
        float npcFrameH = (float)npcList[i].texture.height;

        // 2. Tính khoảng cách từ góc ảnh đến góc Hitbox (Offset)
        float npcOffsetX = (npcFrameW - npcList[i].hitWidth) / 2.0f;
        float npcOffsetY = npcFrameH - npcList[i].hitHeight - npcList[i].paddingBottom;

        // 3. Tính TÂM HITBOX NPC (Tọa độ gốc + Offset + Bán kính Hitbox)
        Vector2 npcHitCenter = {
            npcList[i].position.x + npcOffsetX + (npcList[i].hitWidth / 2.0f),
            npcList[i].position.y + npcOffsetY + (npcList[i].hitHeight / 2.0f)
        };
        // 4. Đo khoảng cách giữa 2 cái TÂM HITBOX
        float distance = Vector2Distance(playerHitCenter, npcHitCenter);

       if (distance < INTERACT_DISTANCE) {
        if (isShowingSecretMap) return;
            // 1. NẾU CHƯA NÓI CHUYỆN -> Bấm E để bắt đầu
            if (!npcList[i].isTalking) {
                if (IsKeyPressed(KEY_E)) {
                    npcList[i].isTalking = true;
                    npcList[i].currentDialogLine = 0;
                    continue; 
                }
            } 
            // 2. NẾU ĐANG NÓI CHUYỆN -> Xử lý riêng
            else {
                DialogEvent* ev = Dialog_GetEvent(npcList[i].id, npcList[i].dialogKey);
                if (ev != NULL && npcList[i].currentDialogLine < ev->lineCount) {
                    DialogLine *curLine = &ev->lines[npcList[i].currentDialogLine];

                    // --- NẾU LÀ CÂU LỰA CHỌN (DÙNG CHUỘT ĐỂ CLICK) ---
                    if (curLine->choiceCount > 0) {
                        Vector2 mousePos = GetVirtualMousePos();
                        int boxWidth = 500;
                        int itemHeight = 35;
                        int boxX = (SCREEN_WIDTH - boxWidth) / 2;
                        int boxY = 80;

                        // Quét xem chuột có click trúng ô lựa chọn nào không
                        for (int c = 0; c < curLine->choiceCount; c++) {
                            Rectangle choiceRec = { boxX, boxY + 10 + c * itemHeight, boxWidth, itemHeight };
                            
                            if (CheckCollisionPointRec(mousePos, choiceRec)) {
                               if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    // [FIX LỖI RẼ NHÁNH] - Lọc bỏ khoảng trắng thừa trong file txt
                                    char nextKey[32] = {0};
                                    sscanf(curLine->nextKeys[c], "%s", nextKey);

                                    if (strcmp(nextKey, "CONTINUE") == 0) {
                                        // Đọc tiếp câu dưới, không nhảy nhánh
                                        npcList[i].currentDialogLine++;
                                        if (npcList[i].currentDialogLine >= ev->lineCount) {
                                            npcList[i].isTalking = false;
                                            npcList[i].currentDialogLine = 0;
                                        }
                                    } 
                                    else if (strcmp(nextKey, "EXIT_DIALOG") == 0 || strcmp(nextKey, "QUIT_DIALOG") == 0) {
                                        // Đóng hội thoại ngay lập tức
                                        npcList[i].isTalking = false;
                                        npcList[i].currentDialogLine = 0;
                                    } 
                                    else {
                                        // Đổi sang nhánh mới
                                        strcpy(npcList[i].dialogKey, nextKey); 
                                        npcList[i].currentDialogLine = 0; 
                                    }
                                    break; // Thoát vòng lặp để tránh click đúp
                                }
                            }
                        }
                    } 
                    // --- NẾU LÀ THOẠI BÌNH THƯỜNG (Bấm E HOẶC Click chuột trái để qua câu) ---
                    else {
                        if (IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            npcList[i].currentDialogLine++;
                            if (npcList[i].currentDialogLine >= ev->lineCount) {
                                npcList[i].isTalking = false;
                                npcList[i].currentDialogLine = 0;
                            }
                        }
                    }
                }
            }

            // --- MENU CHỌN CHẾ ĐỘ CHO NGƯỜI RƠM (99) ---
            if (npcList[i].id == 99 && npcList[i].isTalking) {
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btn1A = { 180, 330, 180, 50 }; 
                Rectangle btn2A = { 440, 330, 180, 50 };

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (CheckCollisionPointRec(mousePos, btn1A)) { npcList[i].isTalking = false; CBC_Start(player, &npcList[i]); return; }
                    if (CheckCollisionPointRec(mousePos, btn2A)) { npcList[i].isTalking = false; Combat_Start(player, &npcList[i]); return; }
                }
                if (IsKeyPressed(KEY_ONE)) { npcList[i].isTalking = false; CBC_Start(player, &npcList[i]); return; }
                if (IsKeyPressed(KEY_TWO)) { npcList[i].isTalking = false; Combat_Start(player, &npcList[i]); return; }
                
                if (IsKeyPressed(KEY_E)) { npcList[i].isTalking = false; }
            }

        // --- BẬT DIALOG TOOL BẰNG SHIFT + D (Giữ nguyên) ---
                if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && IsKeyPressed(KEY_D)) {
                    Debug_OpenDialogTool(&npcList[i]);
                    npcList[i].isTalking = false; // Đóng thoại để mở bảng sửa
                    continue; 
                }    

        } else {
            // Đi ra xa tự động đóng thoại
            npcList[i].isTalking = false;
        }
    }
    
    Interact_CheckExits(player, map, npcList, npcCount);
}

void Interact_DrawUI(Player *player, Npc *npcList, int npcCount, GameMap *map) {
   
    float offsetX = (player->drawWidth - player->hitWidth) / 2.0f;
    float offsetY = player->drawHeight - player->hitHeight - player->paddingBottom;

    Vector2 playerHitCenter = {
        player->position.x + offsetX + (player->hitWidth / 2.0f),
        player->position.y + offsetY + (player->hitHeight / 2.0f)
    };

    // 1. UI NPC
    for (int i = 0; i < npcCount; i++) {
        // Chỉ vẽ NPC đang ở cùng map
        if (npcList[i].mapID != map->currentMapID) continue;

        
       float npcFrameW = (float)npcList[i].texture.width / npcList[i].frameCount;
        float npcFrameH = (float)npcList[i].texture.height;

        float npcOffsetX = (npcFrameW - npcList[i].hitWidth) / 2.0f;
        float npcOffsetY = npcFrameH - npcList[i].hitHeight - npcList[i].paddingBottom;

        Vector2 npcHitCenter = {
            npcList[i].position.x + npcOffsetX + (npcList[i].hitWidth / 2.0f),
            npcList[i].position.y + npcOffsetY + (npcList[i].hitHeight / 2.0f)
        };

        float distance = Vector2Distance(playerHitCenter, npcHitCenter);

        // UI: Gợi ý nút bấm (Hiện chữ [E] trên đầu NPC)
        if (distance < INTERACT_DISTANCE && !npcList[i].isTalking) {
            Vector2 textPos = { npcList[i].position.x - 10, npcList[i].position.y - 40 };
            DrawTextEx(globalFont, "[E] Nói chuyện", textPos, 24, 1, YELLOW);
        }

        // UI: Hộp thoại
        if (npcList[i].isTalking) {
            if (npcList[i].id == 99) { // Ngoại lệ: Người rơm
                UI_DrawDialog(npcList[i].name, ""); // Vẽ khung thoại rỗng
                
                DrawTextEx(globalFont, u8"-> CHỌN CHẾ ĐỘ TẬP LUYỆN", (Vector2){ 320, 110 }, 20, 1, YELLOW);

                Rectangle btn1 = { 180, 330, 180, 45 }; 
                Rectangle btn2 = { 440, 330, 180, 45 };
                Vector2 mPos = GetVirtualMousePos();

                bool hov1 = CheckCollisionPointRec(mPos, btn1);
                bool hov2 = CheckCollisionPointRec(mPos, btn2);

                DrawRectangleRec(btn1, hov1 ? Fade(YELLOW, 0.2f) : Fade(BLACK, 0.4f));
                DrawRectangleRec(btn2, hov2 ? Fade(ORANGE, 0.2f) : Fade(BLACK, 0.4f));
                DrawRectangleLinesEx(btn1, 1, hov1 ? YELLOW : GRAY);
                DrawRectangleLinesEx(btn2, 1, hov2 ? ORANGE : GRAY);

                DrawTextEx(globalFont, u8"1. HỎI ĐÁP", (Vector2){ btn1.x + 40, btn1.y + 12 }, 20, 1, hov1 ? YELLOW : WHITE);
                DrawTextEx(globalFont, u8"2. THỰC CHIẾN", (Vector2){ btn2.x + 25, btn2.y + 12 }, 20, 1, hov2 ? ORANGE : WHITE);
            } 
            else {
                DialogEvent* ev = Dialog_GetEvent(npcList[i].id, npcList[i].dialogKey);
                
                // Kiểm tra xem sự kiện có tồn tại và dòng thoại hiện tại có hợp lệ không
                if (ev != NULL && ev->lineCount > 0 && npcList[i].currentDialogLine < ev->lineCount) {
                    
                    // Lấy ra câu thoại hiện tại
                    DialogLine currentLine = ev->lines[npcList[i].currentDialogLine];
                    
                    // Nếu speakerType == 1 là Player nói, 0 là NPC nói
                    const char* displayName = (currentLine.speakerType == 1) ? "Bạn" : npcList[i].name;
                    
                    UI_DrawDialog(displayName, currentLine.content);

                    // VẼ CÁC LỰA CHỌN (NẾU CÓ) ĐÈ LÊN TRÊN KHUNG THOẠI
                    if (currentLine.choiceCount > 0) {
                        // Khung hiển thị lựa chọn nằm chính giữa màn hình
                        int boxWidth = 500;
                        int itemHeight = 35;
                        int boxHeight = currentLine.choiceCount * itemHeight + 20;
                        int boxX = (SCREEN_WIDTH - boxWidth) / 2;
                        int boxY = 80; // Vẽ phía trên để không đè vào Dialog Box của bạn ở Y=90

                        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, 0.9f));
                        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, YELLOW);

                        Vector2 mousePos = GetVirtualMousePos();

                        for (int c = 0; c < currentLine.choiceCount; c++) {
                            Rectangle choiceRec = { boxX, boxY + 10 + c * itemHeight, boxWidth, itemHeight };
                            
                            // Kiểm tra chuột đang lướt qua ô này
                            bool isHover = CheckCollisionPointRec(mousePos, choiceRec);
                            if (isHover) {
                                DrawRectangleRec(choiceRec, Fade(BLUE, 0.7f)); // Làm sáng nền
                                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);    // Đổi trỏ chuột thành hình bàn tay
                            }

                            // Vẽ text (Bỏ chữ "Phím" đi vì giờ ta xài chuột rồi)
                            DrawTextEx(globalFont, currentLine.choices[c], (Vector2){ boxX + 20, boxY + 15 + c * itemHeight }, 24, 1, isHover ? YELLOW : WHITE);
                        }
                        
                        // Nếu chuột không lướt qua vùng hộp thoại nào thì trả lại con trỏ mặc định
                        if (!CheckCollisionPointRec(mousePos, (Rectangle){boxX, boxY, (float)boxWidth, (float)boxHeight})) {
                            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        }
                    }
                } 
                else {
                    UI_DrawDialog(npcList[i].name, "..."); // Dự phòng lỗi
                }
            }
        }
    }

    // 2. UI Cửa ra vào
    if (map->currentMapID == MAP_THU_VIEN) {
      // Vẽ 3 dấu chấm than cho 3 mảnh giấy
        Vector2 papers[3] = {paper1Pos, paper2Pos, paper3Pos};
        bool picked[3] = {isPaper1Picked, isPaper2Picked, isPaper3Picked};
        
        // --- CHỈ VẼ DẤU ! KHI ĐÃ NHẬN QUEST ---
        bool isQuestActiveDraw = false;
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].id == 8 && (strcmp(npcList[i].dialogKey, "ACCEPT_QUEST") == 0 || player->stats.storyProgress >= 8)) {
                isQuestActiveDraw = true; break;
            }
        }

        if (isQuestActiveDraw) {
            Vector2 papers[3] = {paper1Pos, paper2Pos, paper3Pos};
            bool picked[3] = {isPaper1Picked, isPaper2Picked, isPaper3Picked};
            
            for(int p = 0; p < 3; p++) {
                if (!picked[p]) {
                    Vector2 screenNotePos = GetWorldToScreen2D(papers[p], gameCamera);
                    float distNote = Vector2Distance(playerHitCenter, papers[p]);
                    
                    if (distNote >= INTERACT_DISTANCE) {
                        float scaledSize = 40.0f * gameCamera.zoom;
                        float scaledHoverY = 40.0f * gameCamera.zoom;
                        float floatOffset = sin(GetTime() * 6.0f + p) * (5.0f * gameCamera.zoom); 
                        DrawTextEx(globalFont, "!", (Vector2){screenNotePos.x, screenNotePos.y - scaledHoverY + floatOffset}, scaledSize, 1, RED);
                    } else {
                        float eSize = 20.0f * gameCamera.zoom;
                        float eHoverY = 40.0f * gameCamera.zoom;
                        float eOffsetX = 50.0f * gameCamera.zoom;
                        DrawTextEx(globalFont, "[E] Nhat manh giay", (Vector2){screenNotePos.x - eOffsetX, screenNotePos.y - eHoverY}, eSize, 1, YELLOW);
                    }
                }
            }
        }
        //MAP TEST 
        if (Vector2Distance(playerHitCenter, exitToBlack) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Den", (Vector2){exitToBlack.x - 40, exitToBlack.y - 40}, 24, 1, GREEN);
        }
        if (Vector2Distance(playerHitCenter, exitToWhite) < INTERACT_DISTANCE) {
            DrawTextEx(globalFont, "[E] Vao Phong Trang", (Vector2){exitToWhite.x - 40, exitToWhite.y - 40}, 24, 1, GREEN);
        }
        // ==========================================
        // VẼ UI CỬA SANG TÒA BETA
        // ==========================================
        float distToBeta = Vector2Distance(playerHitCenter, exitToBeta);
        if (distToBeta >= INTERACT_DISTANCE) {
            Vector2 screenPosBeta = GetWorldToScreen2D(exitToBeta, gameCamera);
            // Có áp dụng Zoom chống lệch UI như mảnh giấy
            float scaledSizeBeta = 40.0f * gameCamera.zoom;
            float offsetBeta = sin(GetTime() * 6.0f) * (5.0f * gameCamera.zoom);
            DrawTextEx(globalFont, "!", (Vector2){screenPosBeta.x, screenPosBeta.y - scaledSizeBeta + offsetBeta}, scaledSizeBeta, 1, RED);
        } else {
            if (!isConfirmingBeta) {
                Vector2 screenPosBeta = GetWorldToScreen2D(exitToBeta, gameCamera);
                float eSizeBeta = 20.0f * gameCamera.zoom;
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){screenPosBeta.x - 50 * gameCamera.zoom, screenPosBeta.y - 40 * gameCamera.zoom}, eSizeBeta, 1, YELLOW);
            }
        }

        // BẬT BẢNG THOẠI HỆ THỐNG CỬA BETA
        if (isConfirmingBeta) {
            if (!Inventory_HasItem(ITEM_KEY_SPECIAL)) {
                UI_DrawDialog("Hệ Thống", "Cửa sang Tòa Beta đã bị khóa chặt.\nBạn cần lấy được [Chìa Khóa Đặc Biệt] từ Cô Thủ Thư.\n[Click chuột] hoặc [ENTER] để đóng");
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isConfirmingBeta = false; 
            } else {
                UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Chìa Khóa Đặc Biệt].\nBạn có muốn mở cửa sang Tòa Beta không?");
                
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Có, sang Tòa Beta", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Không, ở lại", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (hoverYes) {
                        isConfirmingBeta = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                        
                        // [CHÚ Ý] Căn chỉnh lại tọa độ (400, 300) này cho đúng vị trí cửa lúc mới sang Tòa Beta nhé!
                        Transition_StartToMap(MAP_BETA, (Vector2){ 720, 370 }); 
                    } else if (hoverNo) {
                        isConfirmingBeta = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }
        // ==========================================
        // VẼ MÀN HÌNH ĐEN FAKE COMBAT CHAP 4
        // ==========================================
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == MAP_THU_VIEN && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f; // Nhấp nháy chữ
                DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                break; 
            }
        }
    }
    //test map
    else if (map->currentMapID == MAP_DEN || map->currentMapID == MAP_TRANG) {
        DrawTextEx(globalFont, "[E] Ve Thu Vien", (Vector2){10, 10}, 24, 1, GREEN);
    }
    else if (map->currentMapID == MAP_TOA_ALPHA) {
        
        // ==========================================
        // 1. VẼ UI CỬA SANG NHÀ VÕ
        // ==========================================
        // Hiện dấu chấm than khi ở xa cửa Nhà Võ
        float distToNhaVo = Vector2Distance(playerHitCenter, exitToNhaVo);
        if (distToNhaVo >= INTERACT_DISTANCE) {
            Vector2 screenPosNhaVo = GetWorldToScreen2D(exitToNhaVo, gameCamera);
            float floatOffset = sin(GetTime() * 6.0f) * 5.0f;
            DrawTextEx(globalFont, "!", (Vector2){screenPosNhaVo.x, screenPosNhaVo.y - 40 + floatOffset}, 40, 1, RED);
        }
        if (Vector2Distance(playerHitCenter, exitToNhaVo) < INTERACT_DISTANCE) {
            if (!isConfirmingNhaVo) {
                // Chỉ hiện 1 dòng duy nhất báo bấm E để kiểm tra
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){exitToNhaVo.x - 40, exitToNhaVo.y - 40}, 24, 1, YELLOW);
            }
        }

        // BẬT BẢNG THOẠI HỆ THỐNG
        if (isConfirmingNhaVo) {
            
            // TRƯỜNG HỢP 1: KHÔNG CÓ CHÌA KHÓA
            if (!Inventory_HasItem(ITEM_KEY_ALPHA)) {
                UI_DrawDialog("Hệ Thống", "Cửa đã bị khóa chặt.\nBạn cần tìm [Chìa Khóa Cũ] để mở cửa này.\n[Click chuột] hoặc [ENTER] để đóng");
                
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    isConfirmingNhaVo = false; 
                }
            } 
            // TRƯỜNG HỢP 2: CÓ CHÌA KHÓA
            else {
                UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Chìa Khóa Cũ].\nBạn có muốn dùng nó để mở cửa sang Nhà Võ không?");
                
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Có, sang Nhà Võ", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Không, ở lại", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (hoverYes) {
                        isConfirmingNhaVo = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                        Transition_StartToMap(MAP_NHA_VO, (Vector2){ 390, 333 }); 
                    } else if (hoverNo) {
                        isConfirmingNhaVo = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }
        // ==========================================
        // [MỚI] VẼ VÒNG MA PHÁP DỊCH CHUYỂN
        // ==========================================
        if (isGateOpened) {
            // Load ảnh Aura dùng chung từ Menu
            if (texTeleAura.id == 0) texTeleAura = LoadTexture("resources/menu/aura.png");
            
            BeginMode2D(gameCamera); // Ép camera để vẽ đè lên mặt sàn Map
            
            // 1. Vẽ 2 vòng tròn xoay mờ mờ dưới đất làm mốc
            DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, 
                (Rectangle){gatePos.x, gatePos.y, 60, 60}, (Vector2){30, 30}, GetTime() * 100.0f, Fade(BLUE, 0.6f));
            DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, 
                (Rectangle){elevatorDest.x, elevatorDest.y, 60, 60}, (Vector2){30, 30}, GetTime() * 100.0f, Fade(BLUE, 0.6f));

            // 2. Kỹ xảo nuốt chửng khi Teleport
            if (isTeleporting) {
                float p = teleTimer; // Chạy từ 0.0 đến 2.0
                Vector2 currentAuraPos = (p < 1.0f) ? teleSource : teleDest;
                
                // Nửa đầu (0->1s): Phóng to vòng tròn để nuốt nhân vật
                // Nửa sau (1->2s): Thu nhỏ vòng tròn nhả nhân vật ra
                float scale = (p < 1.0f) ? (p * 2.5f) : ((2.0f - p) * 2.5f);
                float rotSpeed = 800.0f * p; // Càng quay càng nhanh
                
                float drawSize = 80.0f * scale;
                // +25 để căn giữa tâm vòng phép vào bụng nhân vật
                Rectangle destAura = { currentAuraPos.x + 25, currentAuraPos.y + 25, drawSize, drawSize }; 
                Vector2 origin = { drawSize/2.0f, drawSize/2.0f };
                
                BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, destAura, origin, rotSpeed, Fade(SKYBLUE, 0.9f));
                DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, destAura, origin, -rotSpeed*1.5f, Fade(WHITE, 0.8f));
                
                // Vẽ cái lõi trắng chói lòa (Quả cầu ánh sáng che kín nhân vật)
                DrawCircleV((Vector2){currentAuraPos.x + 25, currentAuraPos.y + 25}, 25.0f * scale, Fade(RAYWHITE, scale/2.0f));
                EndBlendMode();
            }
            EndMode2D();
        }
        // ==========================================
        // 2. VẼ UI CHỐT CHẶN THẦY TUẤN
        // ==========================================
        if (!isGateOpened) {
            float dist = Vector2Distance(playerHitCenter, gatePos);
            // Hiện dấu chấm than khi ở xa cửa kính
            if (dist >= 60.0f) {
                Vector2 screenPosGate = GetWorldToScreen2D(gatePos, gameCamera);
                float floatOffset = sin(GetTime() * 6.0f) * 5.0f;
                DrawTextEx(globalFont, "!", (Vector2){screenPosGate.x, screenPosGate.y - 40 + floatOffset}, 40, 1, RED);
            }
            if (dist < 60.0f && !isInspectingGate) {
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){gatePos.x - 30, gatePos.y - 30}, 20, 1, YELLOW);
            }

            if (isInspectingGate) {
                if (!Inventory_HasItem(ITEM_2)) {
                    UI_DrawDialog("Hệ Thống", "Khu vực này đã bị khóa.\nBạn cần có [Tấm Thẻ] của Cô Lễ Tân để đi tiếp.\n[Click chuột] để đóng");
                } else {
                    UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Tấm Thẻ].\nBạn có muốn dùng nó để mở cửa không?");
                    
                    Vector2 mousePos = GetVirtualMousePos();
                    Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                    Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                    bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                    bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                    DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                    DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                    DrawTextEx(globalFont, "Có, dùng thẻ mở cửa", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                    DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                    DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                    DrawTextEx(globalFont, "Không, để sau", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                    if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                    else SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                }
            }
        }
        // ==========================================
        // 3. VẼ UI BẢNG THÀNH TÍCH
        // ==========================================
        Vector2 bangThanhTichPos = { 420.0f, 230.0f }; 
        float distToBoard = Vector2Distance(playerHitCenter, bangThanhTichPos);
        
        // [SỬA LỖI]: Ép tọa độ UI dính chặt vào tọa độ Bản đồ dù có Zoom Camera
        Vector2 screenPos = GetWorldToScreen2D(bangThanhTichPos, gameCamera);
        
        if (distToBoard >= 60.0f) {
            float floatOffset = sin(GetTime() * 6.0f) * 5.0f; 
            // Vẽ dựa theo screenPos thay vì bangThanhTichPos
            DrawTextEx(globalFont, "!", (Vector2){screenPos.x + 10, screenPos.y - 40 + floatOffset}, 40, 1, RED);
        } 
        else {
            DrawTextEx(globalFont, "[E] Xem Bang Thanh Tich", (Vector2){screenPos.x - 70, screenPos.y - 40}, 20, 1, YELLOW);
            
            if (IsKeyDown(KEY_E)) {
                DrawRectangle(screenPos.x - 100, screenPos.y - 100, 310, 80, Fade(BLACK, 0.8f));
                DrawTextEx(globalFont, "DANH SÁCH SINH VIÊN XUẤT SẮC:", (Vector2){screenPos.x - 90, screenPos.y - 90}, 20, 1, WHITE);
                DrawTextEx(globalFont, "Hoàng Đạt (HE203667)", (Vector2){screenPos.x - 90, screenPos.y - 60}, 20, 1, RED);

                DrawLine(screenPos.x - 90, screenPos.y - 50, screenPos.x + 90, screenPos.y - 50, RED);
            }
        }
        // ==========================================
        // 4. VẼ MÀN HÌNH ĐEN FAKE COMBAT CHAP 1
        // ==========================================
        for (int i = 0; i < npcCount; i++) {
            // Chỉ bật màn đen khi NPC ở Toà Alpha chuyển sang key "COMBAT_IN"
            if (npcList[i].mapID == MAP_TOA_ALPHA && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                
                float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f; // Nhấp nháy chữ
                DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                break; 
            }
        }
    }
    else if (map->currentMapID == MAP_NHA_VO) {
        // ==========================================
        // 2. VẼ UI CỬA CĂNG TIN
        // ==========================================
        float distToCangTin = Vector2Distance(playerHitCenter, exitToCangtin);
        if (distToCangTin >= INTERACT_DISTANCE) {
            Vector2 screenPosCangTin = GetWorldToScreen2D(exitToCangtin, gameCamera);
            float offsetCT = sin(GetTime() * 6.0f) * 5.0f;
            DrawTextEx(globalFont, "!", (Vector2){screenPosCangTin.x, screenPosCangTin.y - 40 + offsetCT}, 40, 1, RED);
        } else {
            if (!isConfirmingCangTin) {
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){exitToCangtin.x - 40, exitToCangtin.y - 40}, 24, 1, YELLOW);
            }
        }

        // BẬT BẢNG THOẠI HỆ THỐNG CỬA CĂNG TIN
        if (isConfirmingCangTin) {
            if (!Inventory_HasItem(ITEM_KEY_BETA)) {
                UI_DrawDialog("Hệ Thống", "Cửa Căng Tin đã bị khóa chặt.\nBạn cần lấy được [Chìa Khóa Căng Tin] từ Thầy Hùng.\n[Click chuột] hoặc [ENTER] để đóng");
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isConfirmingCangTin = false; 
            } else {
                UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Chìa Khóa Căng Tin].\nBạn có muốn mở cửa sang Căng Tin không?");
                
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Có, sang Căng Tin", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Không, ở lại", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (hoverYes) {
                        isConfirmingCangTin = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                        Transition_StartToMap(MAP_NHA_AN, (Vector2){ 180, 380 }); 
                    } else if (hoverNo) {
                        isConfirmingCangTin = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }

        // ==========================================
        // 3. VẼ MÀN HÌNH ĐEN FAKE COMBAT CHỜ BẤM C
        // ==========================================
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == MAP_NHA_VO && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                // Phủ nền đen mờ 90%
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                
                // Chữ nhấp nháy
                float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f; // Dao động 0.0 -> 1.0
                DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                break; // Vẽ 1 lần là đủ
            }
        }
    }
    else if (map->currentMapID == MAP_NHA_AN) {
        
        // ==========================================
        // 1. VẼ HÀNG RÀO CHẶN (BIỂN BÁO TAM GIÁC VÀNG)
        // ==========================================
        if (!isFenceOpened) {
            float distFence = Vector2Distance(playerHitCenter, fencePos);
            Vector2 screenPosFence = GetWorldToScreen2D(fencePos, gameCamera);
            
            // Vẽ biển báo Tam Giác Vàng siêu xịn
            DrawTriangle(
                (Vector2){ screenPosFence.x, screenPosFence.y - 20 },
                (Vector2){ screenPosFence.x - 20, screenPosFence.y + 15 },
                (Vector2){ screenPosFence.x + 20, screenPosFence.y + 15 },
                YELLOW
            );
            DrawTriangleLines(
                (Vector2){ screenPosFence.x, screenPosFence.y - 20 },
                (Vector2){ screenPosFence.x - 20, screenPosFence.y + 15 },
                (Vector2){ screenPosFence.x + 20, screenPosFence.y + 15 },
                RED
            );
            DrawTextEx(globalFont, "!", (Vector2){ screenPosFence.x - 5, screenPosFence.y - 10 }, 24, 1, RED);

            if (distFence < 60.0f && !isInspectingFence) {
                DrawTextEx(globalFont, "[E] Kiem tra", (Vector2){ fencePos.x - 30, fencePos.y - 30 }, 20, 1, YELLOW);
            }

            if (isInspectingFence) {
                UI_DrawDialog("Hệ Thống", "Khu vực bếp chính đang tạm phong tỏa.\nBạn phải giải quyết xong việc với [Cô Đầu Bếp] mới được qua.\n[Click chuột] để đóng");
            }
        }

        // ==========================================
        // 2. VẼ UI CỬA THƯ VIỆN (YÊU CẦU CHÌA DELTA)
        // ==========================================
        float distToThuVien = Vector2Distance(playerHitCenter, exitToThuVien);
        if (distToThuVien >= INTERACT_DISTANCE) {
            Vector2 screenPosTV = GetWorldToScreen2D(exitToThuVien, gameCamera);
            float offsetTV = sin(GetTime() * 6.0f) * 5.0f;
            DrawTextEx(globalFont, "!", (Vector2){screenPosTV.x, screenPosTV.y - 40 + offsetTV}, 40, 1, RED);
        } else {
            if (!isConfirmingThuVien) {
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){exitToThuVien.x - 40, exitToThuVien.y - 40}, 24, 1, YELLOW);
            }
        }

        if (isConfirmingThuVien) {
            if (!Inventory_HasItem(ITEM_KEY_DELTA)) {
                UI_DrawDialog("Hệ Thống", "Cửa Thư Viện đã bị khóa.\nBạn cần lấy được [Chìa Khóa Delta] từ Chú Đầu Bếp.\n[Click chuột] hoặc [ENTER] để đóng");
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isConfirmingThuVien = false; 
            } else {
                UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Chìa Khóa Delta].\nBạn có muốn mở cửa sang Thư Viện không?");
                
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Có, sang Thư Viện", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Không, ở lại", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (hoverYes) {
                        isConfirmingThuVien = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                        Transition_StartToMap(MAP_THU_VIEN, (Vector2){ 335, 200 }); 
                    } else if (hoverNo) {
                        isConfirmingThuVien = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }

        // ==========================================
        // 3. VẼ MÀN HÌNH ĐEN FAKE COMBAT CHAP 3
        // ==========================================
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == MAP_NHA_AN && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
                DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                break; 
            }
        }
    }
   // ==========================================
    // UI CHO MAP BETA (CHAPTER 5)
    // ==========================================
    else if (map->currentMapID == MAP_BETA) {

        // 1. VÒNG MA PHÁP DỊCH CHUYỂN BETA & CHỐT CHẶN CỬA LỚP
        if (isBetaGateOpened) {
            if (texTeleAura.id == 0) texTeleAura = LoadTexture("resources/menu/aura.png");
            BeginMode2D(gameCamera);
            DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, 
                (Rectangle){betaGatePos.x, betaGatePos.y, 60, 60}, (Vector2){30, 30}, GetTime() * 100.0f, Fade(PURPLE, 0.6f));
            DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, 
                (Rectangle){betaElevatorDest.x, betaElevatorDest.y, 60, 60}, (Vector2){30, 30}, GetTime() * 100.0f, Fade(PURPLE, 0.6f));

            if (isTeleportingBeta) {
                float p = teleTimerBeta;
                Vector2 currentAuraPos = (p < 1.0f) ? teleSourceBeta : teleDestBeta;
                float scale = (p < 1.0f) ? (p * 2.5f) : ((2.0f - p) * 2.5f);
                float rotSpeed = 800.0f * p;
                float drawSize = 80.0f * scale;
                Rectangle destAura = { currentAuraPos.x + 25, currentAuraPos.y + 25, drawSize, drawSize }; 
                Vector2 origin = { drawSize/2.0f, drawSize/2.0f };
                BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, destAura, origin, rotSpeed, Fade(MAGENTA, 0.9f));
                DrawTexturePro(texTeleAura, (Rectangle){0,0,texTeleAura.width,texTeleAura.height}, destAura, origin, -rotSpeed*1.5f, Fade(WHITE, 0.8f));
                DrawCircleV((Vector2){currentAuraPos.x + 25, currentAuraPos.y + 25}, 25.0f * scale, Fade(RAYWHITE, scale/2.0f));
                EndBlendMode();
            }
            EndMode2D();
        } else {
            float distGate = Vector2Distance(playerHitCenter, betaGatePos);
            Vector2 screenPosGate = GetWorldToScreen2D(betaGatePos, gameCamera);
            
            if (distGate >= 60.0f) {
                float floatOffset = sin(GetTime() * 6.0f) * 5.0f;
                DrawTextEx(globalFont, "!", (Vector2){screenPosGate.x, screenPosGate.y - 40 + floatOffset}, 40, 1, RED);
            } else {
                // Chỉ hiện chữ bấm E nếu chưa mở bảng thoại
                if (!isInspectingBetaGate) {
                    DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){screenPosGate.x - 40, screenPosGate.y - 40}, 20, 1, YELLOW);
                }
            }

            // VẼ BẢNG THÔNG BÁO CHẶN CỬA
            if (isInspectingBetaGate) {
                UI_DrawDialog("Hệ Thống", "Cửa lớp học đang bị chặn.\nBạn phải giải quyết với [Chú Lao Công] bên ngoài mới được vào.\n[Click chuột] hoặc [ENTER] để đóng");
            }
        }

        // 2. UI CỬA SANG PHÒNG LAB
        float distToLab = Vector2Distance(playerHitCenter, exitToPhongLab);
        Vector2 screenPosLab = GetWorldToScreen2D(exitToPhongLab, gameCamera);

        if (distToLab >= INTERACT_DISTANCE) {
            float floatOffset = sin(GetTime() * 6.0f) * 5.0f;
            DrawTextEx(globalFont, "!", (Vector2){screenPosLab.x, screenPosLab.y - 40 + floatOffset}, 40, 1, RED);
        } else {
            if (!isConfirmingLab) {
                DrawTextEx(globalFont, "[E] Kiem tra cua", (Vector2){screenPosLab.x - 40, screenPosLab.y - 40}, 24, 1, YELLOW);
            }
        }

        if (isConfirmingLab) {
            if (!Inventory_HasItem(ITEM_1)) {
                UI_DrawDialog("Hệ Thống", "Cửa dẫn xuống Phòng Lab đã bị khóa.\nBạn cần có [Bản Đồ] để tìm đường đi.\n[Click chuột] hoặc [ENTER] để đóng");
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isConfirmingLab = false; 
            } else {
                UI_DrawDialog("Hệ Thống", "Hệ thống nhận diện bạn có [Bản Đồ].\nBạn có muốn tiến xuống Phòng Lab không?");
                
                Vector2 mousePos = GetVirtualMousePos();
                Rectangle btnYes = { (SCREEN_WIDTH - 400) / 2, 80, 400, 45 };
                Rectangle btnNo = { (SCREEN_WIDTH - 400) / 2, 135, 400, 45 };

                bool hoverYes = CheckCollisionPointRec(mousePos, btnYes);
                bool hoverNo = CheckCollisionPointRec(mousePos, btnNo);

                DrawRectangleRec(btnYes, hoverYes ? Fade(BLUE, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnYes, 2, hoverYes ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Có, xuống Phòng Lab", (Vector2){ btnYes.x + 20, btnYes.y + 12 }, 24, 1, hoverYes ? YELLOW : WHITE);

                DrawRectangleRec(btnNo, hoverNo ? Fade(RED, 0.8f) : Fade(BLACK, 0.9f));
                DrawRectangleLinesEx(btnNo, 2, hoverNo ? YELLOW : GRAY);
                DrawTextEx(globalFont, "Không, ở lại", (Vector2){ btnNo.x + 20, btnNo.y + 12 }, 24, 1, hoverNo ? YELLOW : WHITE);

                if (hoverYes || hoverNo) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (hoverYes) {
                        isConfirmingLab = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                        Audio_PlaySoundEffect(SFX_UI_CLICK);
                        
                        // [CHÚ Ý] Đo lại tọa độ 400, 300 này lúc qua Lab nhé
                        Transition_StartToMap(MAP_LAB, (Vector2){ 740, 340 }); 
                    } else if (hoverNo) {
                        isConfirmingLab = false;
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }

        // 3. [ĐÃ FIX] MÀN HÌNH ĐEN FAKE COMBAT BETA
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == MAP_BETA && strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f; // Nhấp nháy chữ
                DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                break; 
            }
        }
    }
    // ==========================================
    // UI CHO MAP LAB (CHAPTER 6)
    // ==========================================
    else if (map->currentMapID == MAP_LAB) {
        
        for (int i = 0; i < npcCount; i++) {
            if (npcList[i].mapID == MAP_LAB) {
                
                // 1. MÀN HÌNH ĐEN COMBAT
                if (strcmp(npcList[i].dialogKey, "COMBAT_IN") == 0) {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.9f));
                    float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
                    DrawTextEx(globalFont, "- DANG TRONG COMBAT -", (Vector2){ SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 60 }, 36, 1, RED);
                    DrawTextEx(globalFont, "BAM [C] DE AUTO-WIN", (Vector2){ SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10 }, 30, 1, Fade(YELLOW, alpha));
                    break;
                }
                
                // 2. MÀN HÌNH HACKING "MATRIX"
                else if (strcmp(npcList[i].dialogKey, "HACK_START") == 0) {
                    // Phủ đen xì toàn màn hình
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.95f));
                    
                    // --- KỸ XẢO: MƯA MÃ CODE MA TRẬN CUỘN TỪ TRÊN XUỐNG ---
                    BeginBlendMode(BLEND_ADDITIVE);
                    for (int k = 0; k < 25; k++) {
                        float speed = 150.0f + (k % 5) * 60.0f; // Tốc độ rơi khác nhau
                        float dropY = fmod(GetTime() * speed + k * 87.0f, SCREEN_HEIGHT + 100) - 50;
                        DrawTextEx(globalFont, "0 1 0 1 1 0 1", (Vector2){ (float)(k * 35), dropY }, 20, 1, Fade(GREEN, 0.3f));
                        DrawTextEx(globalFont, "1 0 1 0 0 1 0", (Vector2){ (float)(k * 35), dropY - 30 }, 20, 1, Fade(LIME, 0.4f));
                    }
                    EndBlendMode();

                    // --- KHUNG TERMINAL HACKING ---
                    DrawRectangleLinesEx((Rectangle){ 50, 50, SCREEN_WIDTH - 100, SCREEN_HEIGHT - 100 }, 4, GREEN);
                    DrawTextEx(globalFont, "SYSTEM OVERRIDE TERMINAL", (Vector2){ SCREEN_WIDTH/2 - 150, 80 }, 24, 1, LIME);
                    DrawTextEx(globalFont, "NHẬP MÃ TRUY CẬP (6 SỐ):", (Vector2){ SCREEN_WIDTH/2 - 140, 150 }, 24, 1, LIME);

                    // Liên kết với biến bên story_manager.c để lấy số người chơi đang gõ
                    extern char hackInput[7];
                    extern int hackInputLen;

                    // Vẽ 6 ô vuông nhập mã
                    int boxW = 50;
                    int boxH = 60;
                    int startX = SCREEN_WIDTH/2 - (6 * boxW + 5 * 10) / 2;
                    int startY = 220;

                    for (int b = 0; b < 6; b++) {
                        Rectangle boxRec = { startX + b * (boxW + 10), startY, boxW, boxH };
                        
                        // Ô nào đang chờ gõ thì nhấp nháy
                        if (b == hackInputLen) {
                            float blink = (sin(GetTime() * 10.0f) + 1.0f) / 2.0f;
                            DrawRectangleLinesEx(boxRec, 2, Fade(GREEN, blink));
                        } else {
                            DrawRectangleLinesEx(boxRec, 2, DARKGREEN);
                        }
                        
                        // Nếu đã gõ thì in số màu LIME cực sáng
                        if (b < hackInputLen) {
                            char numStr[2] = { hackInput[b], '\0' };
                            DrawTextEx(globalFont, numStr, (Vector2){ boxRec.x + 16, boxRec.y + 15 }, 30, 1, LIME);
                        }
                    }

                    // Hướng dẫn thao tác
                    float alpha = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
                    DrawTextEx(globalFont, u8"SỬ DỤNG BÀN PHÍM ĐỂ NHẬP - BẤM [BACKSPACE] ĐỂ XÓA", (Vector2){ SCREEN_WIDTH/2 - 260, 340 }, 20, 1, Fade(GREEN, alpha));
                    
                    break;
                }
            }
        }
    }
}

//debug only
void Interact_DrawDebugExits(GameMap *map) {
    if (map->currentMapID == MAP_THU_VIEN) {
        // Vẽ cửa sang Tòa Beta
        DrawCircleV(exitToBeta, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToBeta.x, (int)exitToBeta.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > BETA", (int)exitToBeta.x - 35, (int)exitToBeta.y - 10, 10, WHITE);
        DrawCircle((int)exitToBeta.x, (int)exitToBeta.y, 3.0f, RED);
        // Vẽ cửa sang Đen
        DrawCircleV(exitToBlack, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToBlack.x, (int)exitToBlack.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > DEN", (int)exitToBlack.x - 20, (int)exitToBlack.y - 10, 10, WHITE);
        DrawCircle((int)exitToBlack.x, (int)exitToBlack.y, 3.0f, RED);
        // Vẽ cửa sang Trắng
        DrawCircleV(exitToWhite, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToWhite.x, (int)exitToWhite.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > TRANG", (int)exitToWhite.x - 20, (int)exitToWhite.y - 10, 10, WHITE);
        DrawCircle((int)exitToWhite.x, (int)exitToWhite.y, 3.0f, RED);
    }
    else if (map->currentMapID == MAP_TOA_ALPHA) {
        // Vẽ cửa sang nhavo chapter 2
       DrawCircleV(exitToNhaVo, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToNhaVo.x, (int)exitToNhaVo.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > NHA VO", (int)exitToNhaVo.x - 35, (int)exitToNhaVo.y - 10, 10, WHITE);
        DrawCircle((int)exitToNhaVo.x, (int)exitToNhaVo.y, 3.0f, RED);
    }
    else if (map->currentMapID == MAP_NHA_VO) {
        // Cửa sang Căng Tin
        DrawCircleV(exitToCangtin, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToCangtin.x, (int)exitToCangtin.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > CANGTIN", (int)exitToCangtin.x - 30, (int)exitToCangtin.y - 10, 10, WHITE);
    }
    else if (map->currentMapID == MAP_NHA_AN) {
        DrawCircleV(exitToThuVien, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToThuVien.x, (int)exitToThuVien.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > THU VIEN", (int)exitToThuVien.x - 35, (int)exitToThuVien.y - 10, 10, WHITE);
    }
    else if (map->currentMapID == MAP_BETA) {
        DrawCircleV(exitToPhongLab, INTERACT_DISTANCE, Fade(MAGENTA, 0.3f));
        DrawCircleLines((int)exitToPhongLab.x, (int)exitToPhongLab.y, INTERACT_DISTANCE, MAGENTA);
        DrawText("EXIT > LAB", (int)exitToPhongLab.x - 30, (int)exitToPhongLab.y - 10, 10, WHITE);
    }
} 