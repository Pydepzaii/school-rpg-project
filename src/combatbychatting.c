#include "combatbychatting.h"
#include "raylib.h"
#include "ui_style.h" // Để dùng font globalFont
#include "settings.h" // Để dùng GetVirtualMousePos
#include <stdio.h>
#include <string.h>

#define MAX_SETS 5
#define MAX_QS_PER_SET 10

// Cấu trúc câu hỏi
typedef struct {
    char question[256];
    char answers[4][128];
    int correctAnswer;
} Question;

// Cấu trúc bộ câu hỏi
typedef struct {
    Question qList[MAX_QS_PER_SET];
    int count;
    float passRate;
} QuizSet;

static QuizSet quizBank[MAX_SETS];
static bool cbcActive = false;
static CBC_State cbcState;
static Player *pPlayer = NULL;
static Npc *pEnemy = NULL;

static int currentSetID = 0;
static int currentQuestion = 0;
static int correctCount = 0;
static int selectedAnswer = -1;
static float stateTimer = 0.0f;
static bool isPass = false;

static bool hiddenAnswers[4] = {false, false, false, false}; 
static bool showHint = false;

// Tọa độ vùng bấm cho 4 đáp án
static Rectangle answerRecs[4] = {
    { 110, 190, 580, 40 }, // Đáp án A
    { 110, 240, 580, 40 }, // Đáp án B
    { 110, 290, 580, 40 }, // Đáp án C
    { 110, 340, 580, 40 }  // Đáp án D
};
static Rectangle skillBtnRec = { 550, 95, 140, 35 }; // Nút Kỹ năng nhỏ gọn bên phải

void CBC_Init() {
    // --- BỘ 0: NGƯỜI RƠM (ID 99) - 3 CÂU ---
    quizBank[0].count = 3;
    quizBank[0].passRate = 0.6f; // Chỉ cần đúng 2/3 (60%)
    
    strcpy(quizBank[0].qList[0].question, u8"Câu 1: Dấu '=' trong C dùng để làm gì?");
    strcpy(quizBank[0].qList[0].answers[0], u8"So sánh");
    strcpy(quizBank[0].qList[0].answers[1], u8"Gán giá trị");
    strcpy(quizBank[0].qList[0].answers[2], u8"Kết thúc lệnh");
    strcpy(quizBank[0].qList[0].answers[3], u8"Không làm gì");
    quizBank[0].qList[0].correctAnswer = 1;

    strcpy(quizBank[0].qList[1].question, u8"Câu 2: Kiểu 'int' dùng để lưu số gì?");
    strcpy(quizBank[0].qList[1].answers[0], u8"Số thực");
    strcpy(quizBank[0].qList[1].answers[1], u8"Ký tự");
    strcpy(quizBank[0].qList[1].answers[2], u8"Số nguyên");
    strcpy(quizBank[0].qList[1].answers[3], u8"Hình ảnh");
    quizBank[0].qList[1].correctAnswer = 2;

    strcpy(quizBank[0].qList[2].question, u8"Câu 3: Dấu '&&' là phép toán gì?");
    strcpy(quizBank[0].qList[2].answers[0], u8"Phép AND logic");
    strcpy(quizBank[0].qList[2].answers[1], u8"Phép OR logic");
    strcpy(quizBank[0].qList[2].answers[2], u8"Phép Cộng");
    strcpy(quizBank[0].qList[2].answers[3], u8"Phép Chia");
    quizBank[0].qList[2].correctAnswer = 0;

    // --- BỘ 1: CÔ ĐẦU BẾP (Giả sử ID 5) - 2 CÂU ---
    quizBank[1].count = 2;
    quizBank[1].passRate = 0.5f;
    strcpy(quizBank[1].qList[0].question, u8"Câu 1: Raylib dùng để làm gì?");
    strcpy(quizBank[1].qList[0].answers[0], u8"Làm game");
    strcpy(quizBank[1].qList[0].answers[1], u8"Làm web");
    strcpy(quizBank[1].qList[0].answers[2], u8"Hack NASA");
    strcpy(quizBank[1].qList[0].answers[3], u8"Nấu ăn");
    quizBank[1].qList[0].correctAnswer = 0;
    
    strcpy(quizBank[1].qList[1].question, u8"Câu 2: Phím tắt để lưu file là gì?");
    strcpy(quizBank[1].qList[1].answers[0], u8"Ctrl + C");
    strcpy(quizBank[1].qList[1].answers[1], u8"Ctrl + V");
    strcpy(quizBank[1].qList[1].answers[2], u8"Ctrl + S");
    strcpy(quizBank[1].qList[1].answers[3], u8"Alt + F4");
    quizBank[1].qList[1].correctAnswer = 2;
}

void CBC_Start(Player *playerPtr, Npc *enemyPtr) {
    pPlayer = playerPtr;
    pEnemy = enemyPtr;
    cbcActive = true;
    cbcState = CBC_STATE_INTRO;
    currentQuestion = 0;
    correctCount = 0;
    selectedAnswer = -1;
    stateTimer = 0.0f;

    // Reset giao diện skill
    for(int i=0; i<4; i++) hiddenAnswers[i] = false;
    showHint = false;

    if (pPlayer != NULL) {
        if (pPlayer->cbcStats.maxHp <= 0) pPlayer->cbcStats.maxHp = 5; // Chốt an toàn
        
        // [QUAN TRỌNG]: LUẬT HỒI MÁU MỚI
        if (enemyPtr->id == 99) {
            // Đánh với Người Rơm (Tập luyện) -> Hồi đầy máu
            pPlayer->cbcStats.hp = pPlayer->cbcStats.maxHp;
        } else {
            // Đánh với NPC khác -> Giữ nguyên máu hiện tại!
            // (Vớt vát: Nếu lỡ còn 0 máu thì cho 1 máu để còn có cơ hội lật kèo)
            if (pPlayer->cbcStats.hp <= 0) pPlayer->cbcStats.hp = 1;
        }

        // Reset các chỉ số trong trận
        pPlayer->cbcStats.comboCorrect = 0;
        pPlayer->cbcStats.comboWrong = 0;
        pPlayer->cbcStats.skipNext = false;
        pPlayer->cbcStats.canRetry = true; 
        
        // Nạp Skill theo Class
        int pClass = pPlayer->pClass;
        if (pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU || pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) {
            pPlayer->cbcStats.skillUses = 3;
        } else {
            pPlayer->cbcStats.skillUses = 2; 
        }
    }

    if (enemyPtr->id == 99) currentSetID = 0;      
    else if (enemyPtr->id == 5) currentSetID = 1;  
    else currentSetID = 1;                         
}

void CBC_Update() {
    if (!cbcActive) return;
    float dt = GetFrameTime();
    stateTimer += dt;

    switch (cbcState) {
        case CBC_STATE_INTRO:
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                cbcState = CBC_STATE_QUESTION;
                stateTimer = 0.0f;
            }
            break;

        case CBC_STATE_QUESTION: {
            Vector2 mPos = GetVirtualMousePos();
            int pClass = pPlayer->pClass;
            int correctIdx = quizBank[currentSetID].qList[currentQuestion].correctAnswer;

            // --- 1. XỬ LÝ NÚT BẤM SKILL ---
            if (pPlayer->cbcStats.skillUses > 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mPos, skillBtnRec)) {
                pPlayer->cbcStats.skillUses--; // Trừ 1 lượt Skill
                
                if (pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU) { 
                    // SKILL ĐẦU GẤU: Xóa 1 đáp án sai
                    for (int i = 0; i < 4; i++) {
                        if (i != correctIdx && !hiddenAnswers[i]) {
                            hiddenAnswers[i] = true;
                            break; 
                        }
                    }
                }
                else if (pClass == CLASS_WARRIOR || pClass == CLASS_HOC_BA) { 
                    // SKILL HỌC BÁ: Tiết lộ đáp án đúng (Bật cờ showHint cho giao diện)
                    showHint = true;
                }
                else if (pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) { 
                    // SKILL SOÁI CA: Chuyển sang câu hỏi khác (Cộng điểm luôn)
                    correctCount++;
                    pPlayer->cbcStats.comboCorrect++;
                    pPlayer->cbcStats.comboWrong = 0;
                    cbcState = CBC_STATE_RESULT;
                    stateTimer = 0.0f;
                    break; 
                }
                else if (pClass == CLASS_ARCHER || pClass == CLASS_PHU_NHI_DAI) { 
                    // SKILL PHÚ NHỊ ĐẠI: Loại bỏ 2 đáp án sai ngẫu nhiên
                    int removed = 0;
                    for (int i = 0; i < 4; i++) {
                        if (i != correctIdx && !hiddenAnswers[i]) {
                            hiddenAnswers[i] = true;
                            removed++;
                            if (removed >= 2) break;
                        }
                    }
                }
            }

            // --- 2. XỬ LÝ CHỌN ĐÁP ÁN ---
            for (int i = 0; i < 4; i++) {
                // Chỉ cho phép click nếu đáp án chưa bị ẩn bởi Skill
                if (!hiddenAnswers[i] && CheckCollisionPointRec(mPos, answerRecs[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedAnswer = i;
                }
            }
            // Hỗ trợ phím cứng (1,2,3,4)
            if (IsKeyPressed(KEY_ONE) && !hiddenAnswers[0])   selectedAnswer = 0;
            if (IsKeyPressed(KEY_TWO) && !hiddenAnswers[1])   selectedAnswer = 1;
            if (IsKeyPressed(KEY_THREE) && !hiddenAnswers[2]) selectedAnswer = 2;
            if (IsKeyPressed(KEY_FOUR) && !hiddenAnswers[3])  selectedAnswer = 3;

            // --- 3. XỬ LÝ ĐÚNG/SAI VÀ NỘI TẠI (PASSIVE) ---
            if (selectedAnswer != -1) {
                bool isCorrect = (selectedAnswer == correctIdx);

                // NỘI TẠI HỌC BÁ: Làm sai 1 câu được chọn lại
                if (!isCorrect && (pClass == CLASS_WARRIOR || pClass == CLASS_HOC_BA) && pPlayer->cbcStats.canRetry) {
                    pPlayer->cbcStats.canRetry = false;   // Mất quyền chọn lại ở câu này
                    hiddenAnswers[selectedAnswer] = true; // Gạch bỏ đáp án vừa đoán sai
                    selectedAnswer = -1;                  // Reset lựa chọn để đoán tiếp
                } 
                else {
                    if (isCorrect) {
                        correctCount++;
                        pPlayer->cbcStats.comboCorrect++;
                        pPlayer->cbcStats.comboWrong = 0;

                        // NỘI TẠI ĐẦU GẤU: Đúng 3 câu liên tiếp hồi 1 máu
                        if ((pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU) && pPlayer->cbcStats.comboCorrect == 3) {
                            if (pPlayer->cbcStats.hp < pPlayer->cbcStats.maxHp) pPlayer->cbcStats.hp++;
                            pPlayer->cbcStats.comboCorrect = 0; // Reset lại bộ đếm combo
                        }
                        // NỘI TẠI SOÁI CA: Đúng 3 câu liên tiếp skip 1 câu sau
                        if ((pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) && pPlayer->cbcStats.comboCorrect == 3) {
                            pPlayer->cbcStats.skipNext = true;
                            pPlayer->cbcStats.comboCorrect = 0;
                        }
                    } else {
                        pPlayer->cbcStats.hp--; // Trừ 1 máu sinh tồn
                        pPlayer->cbcStats.comboWrong++;
                        pPlayer->cbcStats.comboCorrect = 0;

                        // NỘI TẠI PHÚ NHỊ ĐẠI: Sai 2 câu liên tục skip 1 câu sau
                        if ((pClass == CLASS_ARCHER || pClass == CLASS_PHU_NHI_DAI) && pPlayer->cbcStats.comboWrong == 2) {
                            pPlayer->cbcStats.skipNext = true;
                            pPlayer->cbcStats.comboWrong = 0;
                        }
                    }
                    cbcState = CBC_STATE_RESULT;
                    stateTimer = 0.0f;
                }
            }
            break;
        }
        
        case CBC_STATE_RESULT:
            if (stateTimer > 1.2f || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                
                if (pPlayer->cbcStats.hp <= 0) {
                    isPass = false; // Hết máu -> Tèo
                    cbcState = CBC_STATE_FINAL;
                } else {
                    currentQuestion++;
                    selectedAnswer = -1;
                    pPlayer->cbcStats.canRetry = true; // Hồi quyền chọn lại cho Học Bá
                    
                    // Reset lại giao diện hiệu ứng Skill
                    for(int i=0; i<4; i++) hiddenAnswers[i] = false;
                    showHint = false;

                    // KIỂM TRA NỘI TẠI SKIP CÂU HỎI
                    if (pPlayer->cbcStats.skipNext) {
                        pPlayer->cbcStats.skipNext = false;
                        currentQuestion++; // Nhảy thêm 1 câu
                        correctCount++;    // Free điểm câu bị bỏ qua
                    }

                    if (currentQuestion >= quizBank[currentSetID].count) {
                        isPass = true; // Sống sót đến cuối -> Thắng
                        cbcState = CBC_STATE_FINAL;
                    } else {
                        cbcState = CBC_STATE_QUESTION;
                    }
                }
                stateTimer = 0.0f;
            }
            break;

        case CBC_STATE_FINAL:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                cbcActive = false;
                if (isPass && pEnemy) pEnemy->isDead = true; // Thắng thì kẻ địch "chết"
            }
            break;
    }
}

void CBC_Draw() {
    // --- BẢNG MÀU STARDEW VALLEY ---
    Color bgParchment = (Color){ 238, 195, 134, 255 }; // Màu giấy da / gỗ lót
    Color woodBorder  = (Color){ 186, 104, 34, 255 };  // Màu gỗ cam sáng (viền khung)
    Color darkBorder  = (Color){ 105, 50, 15, 255 };   // Màu nâu tối (viền đổ bóng)
    Color btnNormal   = (Color){ 220, 145, 40, 255 };  // Nút bấm bình thường
    Color btnHover    = (Color){ 245, 175, 70, 255 };  // Nút bấm khi di chuột qua
    Color textColor   = (Color){ 60, 30, 10, 255 };    // Chữ màu nâu đen

    // Nền tối mờ 50%
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    // --- VẼ KHUNG CHÍNH (MAIN DIALOG BOX) ---
    Rectangle mainRec = { 80, 70, 640, 340 };
    // 1. Viền nâu tối ngoài cùng
    DrawRectangleRec((Rectangle){ mainRec.x - 4, mainRec.y - 4, mainRec.width + 8, mainRec.height + 8 }, darkBorder);
    // 2. Viền gỗ cam
    DrawRectangleRec((Rectangle){ mainRec.x - 2, mainRec.y - 2, mainRec.width + 4, mainRec.height + 4 }, woodBorder);
    // 3. Nền giấy da bên trong
    DrawRectangleRec(mainRec, bgParchment);
    // 4. Đường chìm mỏng bên trong cùng
    DrawRectangleLinesEx((Rectangle){ mainRec.x + 4, mainRec.y + 4, mainRec.width - 8, mainRec.height - 8 }, 2, woodBorder);

    if (cbcState == CBC_STATE_INTRO) {
        DrawTextEx(globalFont, u8"- SỰ KIỆN VẤN ĐÁP -", (Vector2){ 260, 110 }, 28, 1, textColor);
        
        char targetText[150];
        sprintf(targetText, u8"Hệ thống yêu cầu bạn trả lời %d câu hỏi.\nSinh lực hiện tại: %d máu.\n\nBạn đã sẵn sàng chưa?", quizBank[currentSetID].count, pPlayer->cbcStats.hp);
        DrawTextEx(globalFont, targetText, (Vector2){ 120, 170 }, 24, 1, textColor);
        
        // Vẽ nút "Có" (Bắt đầu)
        Rectangle startBtn = { 250, 310, 300, 45 };
        DrawRectangleRec(startBtn, btnNormal);
        DrawRectangleLinesEx(startBtn, 3, darkBorder);
        DrawTextEx(globalFont, u8"Bắt đầu (Click / Enter)", (Vector2){ 280, 320 }, 22, 1, textColor);
    }
    else if (cbcState == CBC_STATE_QUESTION) {
        
        // --- TAB CHỈ SỐ (Như cái đồng hồ góc phải của Stardew) ---
        Rectangle tabRec = { 460, 20, 260, 45 };
        DrawRectangleRec((Rectangle){ tabRec.x - 4, tabRec.y - 4, tabRec.width + 8, tabRec.height + 8 }, darkBorder);
        DrawRectangleRec((Rectangle){ tabRec.x - 2, tabRec.y - 2, tabRec.width + 4, tabRec.height + 4 }, woodBorder);
        DrawRectangleRec(tabRec, bgParchment);
        
        char statsText[50];
        sprintf(statsText, u8"Máu: %d/%d | Skill: %d", pPlayer->cbcStats.hp, pPlayer->cbcStats.maxHp, pPlayer->cbcStats.skillUses);
        DrawTextEx(globalFont, statsText, (Vector2){ tabRec.x + 20, tabRec.y + 12 }, 22, 1, textColor);

        // --- CÂU HỎI ---
        char qTitle[50];
        sprintf(qTitle, u8"- CÂU %d/%d -", currentQuestion + 1, quizBank[currentSetID].count);
        DrawTextEx(globalFont, qTitle, (Vector2){ 100, 90 }, 22, 1, textColor);
        DrawTextEx(globalFont, quizBank[currentSetID].qList[currentQuestion].question, (Vector2){ 100, 130 }, 24, 1, textColor);

        // --- CÁC NÚT ĐÁP ÁN ---
        Vector2 mPos = GetVirtualMousePos();
        const char* labels[] = {"A.", "B.", "C.", "D."};

        for (int i = 0; i < 4; i++) {
            if (hiddenAnswers[i]) continue; // Ẩn nếu dùng Skill

            bool hover = CheckCollisionPointRec(mPos, answerRecs[i]);
            Color cardColor = hover ? btnHover : btnNormal;
            
            // Skill Học Bá -> Nổi bật màu Xanh lá
            if (showHint && i == quizBank[currentSetID].qList[currentQuestion].correctAnswer) cardColor = (Color){ 160, 220, 100, 255 };

            DrawRectangleRec(answerRecs[i], cardColor);
            DrawRectangleLinesEx(answerRecs[i], 3, darkBorder); 
            
            char ansFull[150];
            sprintf(ansFull, "%s  %s", labels[i], quizBank[currentSetID].qList[currentQuestion].answers[i]);
            DrawTextEx(globalFont, ansFull, (Vector2){ answerRecs[i].x + 15, answerRecs[i].y + 10 }, 22, 1, textColor);
        }

        // --- NÚT KỸ NĂNG ---
        if (pPlayer->cbcStats.skillUses > 0) {
            bool hoverBtn = CheckCollisionPointRec(mPos, skillBtnRec);
            DrawRectangleRec(skillBtnRec, hoverBtn ? btnHover : btnNormal);
            DrawRectangleLinesEx(skillBtnRec, 2, darkBorder);
            DrawTextEx(globalFont, u8"Kỹ Năng", (Vector2){ skillBtnRec.x + 35, skillBtnRec.y + 8 }, 20, 1, textColor);
        }
    }
    else if (cbcState == CBC_STATE_RESULT) {
        bool correct = (selectedAnswer == quizBank[currentSetID].qList[currentQuestion].correctAnswer);
        
        DrawTextEx(globalFont, correct ? u8"- CHÍNH XÁC -" : u8"- SAI RỒI -", (Vector2){ 310, 170 }, 30, 1, correct ? (Color){ 20, 100, 20, 255 } : RED);
        
        char rightAns[100];
        sprintf(rightAns, u8"Đáp án đúng là: %d", quizBank[currentSetID].qList[currentQuestion].correctAnswer + 1);
        DrawTextEx(globalFont, rightAns, (Vector2){ 290, 240 }, 24, 1, textColor);
        
        DrawTextEx(globalFont, u8"(Bấm để tiếp tục)", (Vector2){ 320, 320 }, 20, 1, darkBorder);
    }
    else if (cbcState == CBC_STATE_FINAL) {
        DrawTextEx(globalFont, isPass ? u8"- VƯỢT ẢI THÀNH CÔNG -" : u8"- THẤT BẠI -", (Vector2){ 250, 160 }, 30, 1, isPass ? (Color){ 20, 100, 20, 255 } : RED);
        
        char scoreText[50];
        sprintf(scoreText, u8"Bạn trả lời đúng: %d / %d", correctCount, quizBank[currentSetID].count);
        DrawTextEx(globalFont, scoreText, (Vector2){ 270, 240 }, 24, 1, textColor);
        
        DrawTextEx(globalFont, u8"(Bấm để thoát)", (Vector2){ 330, 330 }, 20, 1, darkBorder);
    }
}

bool CBC_IsActive() { return cbcActive; }
void CBC_Shutdown() { }