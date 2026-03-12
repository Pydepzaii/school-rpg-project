#include "combatbychatting.h"
#include "raylib.h"
#include "ui_style.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include "npc.h"

#define MAX_SETS 13
#define MAX_QS_PER_SET 20

typedef struct {
    char question[256];
    char answers[4][128];
    int correctAnswer;
} Question;

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
static bool cbcJustLost = false;

static bool hiddenAnswers[4] = {false, false, false, false}; 
static bool showHint = false;
static float fullHpMsgTimer = 0.0f;
static bool skillUsedThisQuestion = false;
static float skillBlockMsgTimer = 0.0f;

static Rectangle answerRecs[4] = {
    { 110, 190, 580, 40 },
    { 110, 240, 580, 40 },
    { 110, 290, 580, 40 },
    { 110, 340, 580, 40 }
};
static Rectangle skillBtnRec = { 550, 95, 140, 35 };

void CBC_Init() {
    // --- BỘ 0: NGƯỜI RƠM (ID 99) ---
    quizBank[0].count = 3;
    quizBank[0].passRate = 0.6f; 
    
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

    // --- BỘ 1: CÔ LỄ TÂN (Chapter 1 - 3 câu) ---
    quizBank[1].count = 3;
    quizBank[1].passRate = 0.6f;

    strcpy(quizBank[1].qList[0].question, u8"Thủ đô của Việt Nam là thành phố nào?");
    strcpy(quizBank[1].qList[0].answers[0], u8"Hồ Chí Minh");
    strcpy(quizBank[1].qList[0].answers[1], u8"Hà Nội");
    strcpy(quizBank[1].qList[0].answers[2], u8"Đà Nẵng");
    strcpy(quizBank[1].qList[0].answers[3], u8"Huế");
    quizBank[1].qList[0].correctAnswer = 1;

    strcpy(quizBank[1].qList[1].question, u8"Sông nào dài nhất Việt Nam?");
    strcpy(quizBank[1].qList[1].answers[0], u8"Sông Hồng");
    strcpy(quizBank[1].qList[1].answers[1], u8"Sông Đà");
    strcpy(quizBank[1].qList[1].answers[2], u8"Sông Mekong");
    strcpy(quizBank[1].qList[1].answers[3], u8"Sông Đồng Nai");
    quizBank[1].qList[1].correctAnswer = 2;

    strcpy(quizBank[1].qList[2].question, u8"Núi nào cao nhất Việt Nam?");
    strcpy(quizBank[1].qList[2].answers[0], u8"Núi Bà Đen");
    strcpy(quizBank[1].qList[2].answers[1], u8"Núi Ngọc Linh");
    strcpy(quizBank[1].qList[2].answers[2], u8"Núi Fansipan");
    strcpy(quizBank[1].qList[2].answers[3], u8"Núi Chư Yang Sin");
    quizBank[1].qList[2].correctAnswer = 2;

    // --- BỘ 2: THẦY TUẤN VM (Chapter 1 - 5 câu) ---
    quizBank[2].count = 5;
    quizBank[2].passRate = 0.6f;

    strcpy(quizBank[2].qList[0].question, u8"Ai là người đầu tiên đặt chân lên Mặt Trăng?");
    strcpy(quizBank[2].qList[0].answers[0], u8"Buzz Aldrin");
    strcpy(quizBank[2].qList[0].answers[1], u8"Yuri Gagarin");
    strcpy(quizBank[2].qList[0].answers[2], u8"Neil Armstrong");
    strcpy(quizBank[2].qList[0].answers[3], u8"John Glenn");
    quizBank[2].qList[0].correctAnswer = 2;

    strcpy(quizBank[2].qList[1].question, u8"Năm nào Việt Nam thống nhất đất nước?");
    strcpy(quizBank[2].qList[1].answers[0], u8"1973");
    strcpy(quizBank[2].qList[1].answers[1], u8"1954");
    strcpy(quizBank[2].qList[1].answers[2], u8"1976");
    strcpy(quizBank[2].qList[1].answers[3], u8"1975");
    quizBank[2].qList[1].correctAnswer = 2;

    strcpy(quizBank[2].qList[2].question, u8"Nguyên tố nào có ký hiệu hóa học là 'Fe'?");
    strcpy(quizBank[2].qList[2].answers[0], u8"Đồng");
    strcpy(quizBank[2].qList[2].answers[1], u8"Sắt");
    strcpy(quizBank[2].qList[2].answers[2], u8"Vàng");
    strcpy(quizBank[2].qList[2].answers[3], u8"Bạc");
    quizBank[2].qList[2].correctAnswer = 1;

    strcpy(quizBank[2].qList[3].question, u8"Tác phẩm 'Truyện Kiều' do ai sáng tác?");
    strcpy(quizBank[2].qList[3].answers[0], u8"Hồ Xuân Hương");
    strcpy(quizBank[2].qList[3].answers[1], u8"Nguyễn Du");
    strcpy(quizBank[2].qList[3].answers[2], u8"Nguyễn Trãi");
    strcpy(quizBank[2].qList[3].answers[3], u8"Phan Bội Châu");
    quizBank[2].qList[3].correctAnswer = 1;

    strcpy(quizBank[2].qList[4].question, u8"Tốc độ ánh sáng trong chân không xấp xỉ bao nhiêu km/s?");
    strcpy(quizBank[2].qList[4].answers[0], u8"150.000 km/s");
    strcpy(quizBank[2].qList[4].answers[1], u8"300.000 km/s");
    strcpy(quizBank[2].qList[4].answers[2], u8"500.000 km/s");
    strcpy(quizBank[2].qList[4].answers[3], u8"1.000.000 km/s");
    quizBank[2].qList[4].correctAnswer = 1;

    // --- BỘ 3: THẦY CHÍNH (Chapter 2 - 5 câu Vovinam) ---
    quizBank[3].count = 5;
    quizBank[3].passRate = 0.6f;

    strcpy(quizBank[3].qList[0].question, u8"Vovinam được sáng lập bởi ai?");
    strcpy(quizBank[3].qList[0].answers[0], u8"Trần Hưng Đạo");
    strcpy(quizBank[3].qList[0].answers[1], u8"Nguyễn Lộc");
    strcpy(quizBank[3].qList[0].answers[2], u8"Lê Văn Tâm");
    strcpy(quizBank[3].qList[0].answers[3], u8"Phạm Xuân Tòng");
    quizBank[3].qList[0].correctAnswer = 1;

    strcpy(quizBank[3].qList[1].question, u8"Vovinam được sáng lập năm nào?");
    strcpy(quizBank[3].qList[1].answers[0], u8"1930");
    strcpy(quizBank[3].qList[1].answers[1], u8"1938");
    strcpy(quizBank[3].qList[1].answers[2], u8"1945");
    strcpy(quizBank[3].qList[1].answers[3], u8"1950");
    quizBank[3].qList[1].correctAnswer = 1;

    strcpy(quizBank[3].qList[2].question, u8"Màu võ phục truyền thống của Vovinam là gì?");
    strcpy(quizBank[3].qList[2].answers[0], u8"Trắng");
    strcpy(quizBank[3].qList[2].answers[1], u8"Đen");
    strcpy(quizBank[3].qList[2].answers[2], u8"Xanh lam");
    strcpy(quizBank[3].qList[2].answers[3], u8"Đỏ");
    quizBank[3].qList[2].correctAnswer = 2;

    strcpy(quizBank[3].qList[3].question, u8"Vovinam - Việt Võ Đạo có nguồn gốc từ nước nào?");
    strcpy(quizBank[3].qList[3].answers[0], u8"Trung Quốc");
    strcpy(quizBank[3].qList[3].answers[1], u8"Nhật Bản");
    strcpy(quizBank[3].qList[3].answers[2], u8"Việt Nam");
    strcpy(quizBank[3].qList[3].answers[3], u8"Hàn Quốc");
    quizBank[3].qList[3].correctAnswer = 2;

    strcpy(quizBank[3].qList[4].question, u8"Đòn đặc trưng nổi tiếng nhất của Vovinam là gì?");
    strcpy(quizBank[3].qList[4].answers[0], u8"Đấm thẳng");
    strcpy(quizBank[3].qList[4].answers[1], u8"Cắt cổ bằng chân");
    strcpy(quizBank[3].qList[4].answers[2], u8"Đá vòng cầu");
    strcpy(quizBank[3].qList[4].answers[3], u8"Quét chân");
    quizBank[3].qList[4].correctAnswer = 1;

    // --- BỘ 4: THẦY HÙNG (Chapter 2 - 7 câu Vovinam) ---
    quizBank[4].count = 7;
    quizBank[4].passRate = 0.6f;

    strcpy(quizBank[4].qList[0].question, u8"Cấp đai cao nhất trong Vovinam là gì?");
    strcpy(quizBank[4].qList[0].answers[0], u8"Đai đen");
    strcpy(quizBank[4].qList[0].answers[1], u8"Đai đỏ");
    strcpy(quizBank[4].qList[0].answers[2], u8"Đai vàng");
    strcpy(quizBank[4].qList[0].answers[3], u8"Đai trắng");
    quizBank[4].qList[0].correctAnswer = 1;

    strcpy(quizBank[4].qList[1].question, u8"Khẩu hiệu của Vovinam là gì?");
    strcpy(quizBank[4].qList[1].answers[0], u8"Thân - Tâm - Trí");
    strcpy(quizBank[4].qList[1].answers[1], u8"Sức mạnh - Tốc độ - Kỹ thuật");
    strcpy(quizBank[4].qList[1].answers[2], u8"Cách mạng tâm thân");
    strcpy(quizBank[4].qList[1].answers[3], u8"Võ đức - Võ công - Võ thể");
    quizBank[4].qList[1].correctAnswer = 2;

    strcpy(quizBank[4].qList[2].question, u8"Vovinam lần đầu được dạy công khai ở đâu?");
    strcpy(quizBank[4].qList[2].answers[0], u8"Hà Nội");
    strcpy(quizBank[4].qList[2].answers[1], u8"Huế");
    strcpy(quizBank[4].qList[2].answers[2], u8"Sài Gòn");
    strcpy(quizBank[4].qList[2].answers[3], u8"Đà Nẵng");
    quizBank[4].qList[2].correctAnswer = 0;

    strcpy(quizBank[4].qList[3].question, u8"Vovinam hiện được phổ biến ở bao nhiêu quốc gia?");
    strcpy(quizBank[4].qList[3].answers[0], u8"Hơn 20 quốc gia");
    strcpy(quizBank[4].qList[3].answers[1], u8"Hơn 40 quốc gia");
    strcpy(quizBank[4].qList[3].answers[2], u8"Hơn 60 quốc gia");
    strcpy(quizBank[4].qList[3].answers[3], u8"Hơn 80 quốc gia");
    quizBank[4].qList[3].correctAnswer = 2;

    strcpy(quizBank[4].qList[4].question, u8"Trong Vovinam, 'đòn chân bay' thường nhắm vào đâu?");
    strcpy(quizBank[4].qList[4].answers[0], u8"Bụng đối thủ");
    strcpy(quizBank[4].qList[4].answers[1], u8"Cổ đối thủ");
    strcpy(quizBank[4].qList[4].answers[2], u8"Chân đối thủ");
    strcpy(quizBank[4].qList[4].answers[3], u8"Lưng đối thủ");
    quizBank[4].qList[4].correctAnswer = 1;

    strcpy(quizBank[4].qList[5].question, u8"Chưởng môn đệ nhị của Vovinam là ai?");
    strcpy(quizBank[4].qList[5].answers[0], u8"Trần Huy Phong");
    strcpy(quizBank[4].qList[5].answers[1], u8"Lê Sáng");
    strcpy(quizBank[4].qList[5].answers[2], u8"Nguyễn Văn Chiếu");
    strcpy(quizBank[4].qList[5].answers[3], u8"Phạm Xuân Tòng");
    quizBank[4].qList[5].correctAnswer = 1;

    strcpy(quizBank[4].qList[6].question, u8"Vovinam được đưa vào thi đấu SEA Games lần đầu năm nào?");
    strcpy(quizBank[4].qList[6].answers[0], u8"2005");
    strcpy(quizBank[4].qList[6].answers[1], u8"2009");
    strcpy(quizBank[4].qList[6].answers[2], u8"2011");
    strcpy(quizBank[4].qList[6].answers[3], u8"2013");
    quizBank[4].qList[6].correctAnswer = 1;

    // --- BỘ 5: CÔ ĐẦU BẾP (Chapter 3 - 5 câu) ---
    quizBank[5].count = 5;
    quizBank[5].passRate = 0.6f;

    strcpy(quizBank[5].qList[0].question, u8"Vitamin C có nhiều nhất trong loại quả nào?");
    strcpy(quizBank[5].qList[0].answers[0], u8"Cam");
    strcpy(quizBank[5].qList[0].answers[1], u8"Ổi");
    strcpy(quizBank[5].qList[0].answers[2], u8"Chanh");
    strcpy(quizBank[5].qList[0].answers[3], u8"Xoài");
    quizBank[5].qList[0].correctAnswer = 1;

    strcpy(quizBank[5].qList[1].question, u8"Nhiệt độ an toàn để bảo quản thực phẩm trong tủ lạnh là\nbao nhiêu?");
    strcpy(quizBank[5].qList[1].answers[0], u8"Dưới 10 độ C");
    strcpy(quizBank[5].qList[1].answers[1], u8"Dưới 5 độ C");
    strcpy(quizBank[5].qList[1].answers[2], u8"Dưới 15 độ C");
    strcpy(quizBank[5].qList[1].answers[3], u8"Dưới 20 độ C");
    quizBank[5].qList[1].correctAnswer = 1;

    strcpy(quizBank[5].qList[2].question, u8"Nước sôi ở nhiệt độ bao nhiêu độ C ở điều kiện thường?");
    strcpy(quizBank[5].qList[2].answers[0], u8"90 độ C");
    strcpy(quizBank[5].qList[2].answers[1], u8"95 độ C");
    strcpy(quizBank[5].qList[2].answers[2], u8"100 độ C");
    strcpy(quizBank[5].qList[2].answers[3], u8"105 độ C");
    quizBank[5].qList[2].correctAnswer = 2;

    strcpy(quizBank[5].qList[3].question, u8"Chất nào giúp cơm không bị dính nồi khi nấu?");
    strcpy(quizBank[5].qList[3].answers[0], u8"Muối");
    strcpy(quizBank[5].qList[3].answers[1], u8"Đường");
    strcpy(quizBank[5].qList[3].answers[2], u8"Dầu ăn");
    strcpy(quizBank[5].qList[3].answers[3], u8"Giấm");
    quizBank[5].qList[3].correctAnswer = 2;

    strcpy(quizBank[5].qList[4].question, u8"Thực phẩm nào chứa nhiều protein nhất?");
    strcpy(quizBank[5].qList[4].answers[0], u8"Gạo trắng");
    strcpy(quizBank[5].qList[4].answers[1], u8"Ức gà");
    strcpy(quizBank[5].qList[4].answers[2], u8"Khoai lang");
    strcpy(quizBank[5].qList[4].answers[3], u8"Bắp cải");
    quizBank[5].qList[4].correctAnswer = 1;

    // --- BỘ 6: CHÚ ĐẦU BẾP (Chapter 3 - 5 câu) ---
    quizBank[6].count = 5;
    quizBank[6].passRate = 0.6f;

    strcpy(quizBank[6].qList[0].question, u8"Phản ứng Maillard xảy ra khi nào trong nấu ăn?");
    strcpy(quizBank[6].qList[0].answers[0], u8"Khi luộc thực phẩm");
    strcpy(quizBank[6].qList[0].answers[1], u8"Khi thực phẩm bị đông lạnh");
    strcpy(quizBank[6].qList[0].answers[2], u8"Khi protein và đường bị làm nóng tạo màu vàng nâu");
    strcpy(quizBank[6].qList[0].answers[3], u8"Khi thêm muối vào thực phẩm");
    quizBank[6].qList[0].correctAnswer = 2;

    strcpy(quizBank[6].qList[1].question, u8"Chỉ số GI (Glycemic Index) đo lường điều gì?");
    strcpy(quizBank[6].qList[1].answers[0], u8"Lượng chất béo trong thực phẩm");
    strcpy(quizBank[6].qList[1].answers[1], u8"Tốc độ thực phẩm làm tăng đường huyết");
    strcpy(quizBank[6].qList[1].answers[2], u8"Lượng calo trong thực phẩm");
    strcpy(quizBank[6].qList[1].answers[3], u8"Hàm lượng vitamin trong thực phẩm");
    quizBank[6].qList[1].correctAnswer = 1;

    strcpy(quizBank[6].qList[2].question, u8"Kỹ thuật 'Sous Vide' trong nấu ăn là gì?");
    strcpy(quizBank[6].qList[2].answers[0], u8"Nướng ở nhiệt độ rất cao");
    strcpy(quizBank[6].qList[2].answers[1], u8"Nấu thực phẩm trong túi hút chân không dưới nước");
    strcpy(quizBank[6].qList[2].answers[2], u8"Chiên ngập dầu ở nhiệt độ thấp");
    strcpy(quizBank[6].qList[2].answers[3], u8"Hấp cách thủy truyền thống");
    quizBank[6].qList[2].correctAnswer = 1;

    strcpy(quizBank[6].qList[3].question, u8"Tại sao thêm muối vào nước trước khi luộc mì?");
    strcpy(quizBank[6].qList[3].answers[0], u8"Làm nước sôi nhanh hơn");
    strcpy(quizBank[6].qList[3].answers[1], u8"Làm mì không bị dính");
    strcpy(quizBank[6].qList[3].answers[2], u8"Tăng nhiệt độ sôi và tạo vị cho mì");
    strcpy(quizBank[6].qList[3].answers[3], u8"Giúp mì chín đều hơn");
    quizBank[6].qList[3].correctAnswer = 2;

    strcpy(quizBank[6].qList[4].question, u8"Umami là vị thứ mấy trong các vị cơ bản của con người?");
    strcpy(quizBank[6].qList[4].answers[0], u8"Vị thứ 4");
    strcpy(quizBank[6].qList[4].answers[1], u8"Vị thứ 5");
    strcpy(quizBank[6].qList[4].answers[2], u8"Vị thứ 6");
    strcpy(quizBank[6].qList[4].answers[3], u8"Vị thứ 7");
    quizBank[6].qList[4].correctAnswer = 1;

    // --- BỘ 7: CHÚ LAO CÔNG (Chapter 4 - 5 câu Chính trị) ---
    quizBank[7].count = 5;
    quizBank[7].passRate = 0.6f;

    strcpy(quizBank[7].qList[0].question, u8"Quốc hội Việt Nam họp mấy kỳ mỗi năm?");
    strcpy(quizBank[7].qList[0].answers[0], u8"1 kỳ");
    strcpy(quizBank[7].qList[0].answers[1], u8"2 kỳ");
    strcpy(quizBank[7].qList[0].answers[2], u8"3 kỳ");
    strcpy(quizBank[7].qList[0].answers[3], u8"4 kỳ");
    quizBank[7].qList[0].correctAnswer = 1;

    strcpy(quizBank[7].qList[1].question, u8"Nhà nước Việt Nam theo thể chế chính trị nào?");
    strcpy(quizBank[7].qList[1].answers[0], u8"Quân chủ lập hiến");
    strcpy(quizBank[7].qList[1].answers[1], u8"Cộng hòa đại nghị");
    strcpy(quizBank[7].qList[1].answers[2], u8"Cộng hòa xã hội chủ nghĩa");
    strcpy(quizBank[7].qList[1].answers[3], u8"Liên bang");
    quizBank[7].qList[1].correctAnswer = 2;

    strcpy(quizBank[7].qList[2].question, u8"Hiến pháp hiện hành của Việt Nam được ban hành năm nào?");
    strcpy(quizBank[7].qList[2].answers[0], u8"1992");
    strcpy(quizBank[7].qList[2].answers[1], u8"2001");
    strcpy(quizBank[7].qList[2].answers[2], u8"2013");
    strcpy(quizBank[7].qList[2].answers[3], u8"2015");
    quizBank[7].qList[2].correctAnswer = 2;

    strcpy(quizBank[7].qList[3].question, u8"Cơ quan quyền lực nhà nước cao nhất của Việt Nam là gì?");
    strcpy(quizBank[7].qList[3].answers[0], u8"Chính phủ");
    strcpy(quizBank[7].qList[3].answers[1], u8"Quốc hội");
    strcpy(quizBank[7].qList[3].answers[2], u8"Tòa án nhân dân tối cao");
    strcpy(quizBank[7].qList[3].answers[3], u8"Hội đồng nhân dân");
    quizBank[7].qList[3].correctAnswer = 1;

    strcpy(quizBank[7].qList[4].question, u8"Việt Nam gia nhập ASEAN năm nào?");
    strcpy(quizBank[7].qList[4].answers[0], u8"1990");
    strcpy(quizBank[7].qList[4].answers[1], u8"1993");
    strcpy(quizBank[7].qList[4].answers[2], u8"1995");
    strcpy(quizBank[7].qList[4].answers[3], u8"1997");
    quizBank[7].qList[4].correctAnswer = 2;

    // --- BỘ 8: CÔ THỦ THƯ (Chapter 4 - 10 câu Văn học) ---
    quizBank[8].count = 10;
    quizBank[8].passRate = 0.6f;

    strcpy(quizBank[8].qList[0].question, u8"Truyện Kiều có bao nhiêu câu thơ?");
    strcpy(quizBank[8].qList[0].answers[0], u8"3254 câu");
    strcpy(quizBank[8].qList[0].answers[1], u8"3524 câu");
    strcpy(quizBank[8].qList[0].answers[2], u8"3245 câu");
    strcpy(quizBank[8].qList[0].answers[3], u8"3542 câu");
    quizBank[8].qList[0].correctAnswer = 0;

    strcpy(quizBank[8].qList[1].question, u8"Tác phẩm 'Tắt đèn' do ai sáng tác?");
    strcpy(quizBank[8].qList[1].answers[0], u8"Nam Cao");
    strcpy(quizBank[8].qList[1].answers[1], u8"Ngô Tất Tố");
    strcpy(quizBank[8].qList[1].answers[2], u8"Vũ Trọng Phụng");
    strcpy(quizBank[8].qList[1].answers[3], u8"Tô Hoài");
    quizBank[8].qList[1].correctAnswer = 1;

    strcpy(quizBank[8].qList[2].question, u8"Nhân vật Chí Phèo xuất hiện trong tác phẩm của ai?");
    strcpy(quizBank[8].qList[2].answers[0], u8"Nguyễn Công Hoan");
    strcpy(quizBank[8].qList[2].answers[1], u8"Vũ Trọng Phụng");
    strcpy(quizBank[8].qList[2].answers[2], u8"Nam Cao");
    strcpy(quizBank[8].qList[2].answers[3], u8"Ngô Tất Tố");
    quizBank[8].qList[2].correctAnswer = 2;

    strcpy(quizBank[8].qList[3].question, u8"'Bình Ngô Đại Cáo' được viết bằng chữ gì?");
    strcpy(quizBank[8].qList[3].answers[0], u8"Chữ Quốc ngữ");
    strcpy(quizBank[8].qList[3].answers[1], u8"Chữ Nôm");
    strcpy(quizBank[8].qList[3].answers[2], u8"Chữ Hán");
    strcpy(quizBank[8].qList[3].answers[3], u8"Chữ Phạn");
    quizBank[8].qList[3].correctAnswer = 2;

    strcpy(quizBank[8].qList[4].question, u8"Tác phẩm 'Dế Mèn phiêu lưu ký' do ai viết?");
    strcpy(quizBank[8].qList[4].answers[0], u8"Tô Hoài");
    strcpy(quizBank[8].qList[4].answers[1], u8"Nguyên Hồng");
    strcpy(quizBank[8].qList[4].answers[2], u8"Xuân Diệu");
    strcpy(quizBank[8].qList[4].answers[3], u8"Huy Cận");
    quizBank[8].qList[4].correctAnswer = 0;

    strcpy(quizBank[8].qList[5].question, u8"Phong trào Thơ Mới xuất hiện vào thập niên nào?");
    strcpy(quizBank[8].qList[5].answers[0], u8"1920");
    strcpy(quizBank[8].qList[5].answers[1], u8"1930");
    strcpy(quizBank[8].qList[5].answers[2], u8"1940");
    strcpy(quizBank[8].qList[5].answers[3], u8"1950");
    quizBank[8].qList[5].correctAnswer = 1;

    strcpy(quizBank[8].qList[6].question, u8"'Truyện An Dương Vương và Mị Châu - Trọng Thủy' thuộc thể loại \nnào?");
    strcpy(quizBank[8].qList[6].answers[0], u8"Truyền thuyết");
    strcpy(quizBank[8].qList[6].answers[1], u8"Cổ tích");
    strcpy(quizBank[8].qList[6].answers[2], u8"Thần thoại");
    strcpy(quizBank[8].qList[6].answers[3], u8"Ngụ ngôn");
    quizBank[8].qList[6].correctAnswer = 0;

    strcpy(quizBank[8].qList[7].question, u8"Ai là tác giả của bài thơ 'Đây thôn Vĩ Dạ'?");
    strcpy(quizBank[8].qList[7].answers[0], u8"Xuân Diệu");
    strcpy(quizBank[8].qList[7].answers[1], u8"Hàn Mặc Tử");
    strcpy(quizBank[8].qList[7].answers[2], u8"Huy Cận");
    strcpy(quizBank[8].qList[7].answers[3], u8"Chế Lan Viên");
    quizBank[8].qList[7].correctAnswer = 1;

    strcpy(quizBank[8].qList[8].question, u8"Tác phẩm 'Số đỏ' của Vũ Trọng Phụng thuộc thể loại nào?");
    strcpy(quizBank[8].qList[8].answers[0], u8"Truyện ngắn");
    strcpy(quizBank[8].qList[8].answers[1], u8"Thơ");
    strcpy(quizBank[8].qList[8].answers[2], u8"Tiểu thuyết");
    strcpy(quizBank[8].qList[8].answers[3], u8"Kịch");
    quizBank[8].qList[8].correctAnswer = 2;

    strcpy(quizBank[8].qList[9].question, u8"Nhân vật nào là nhân vật chính trong 'Lão Hạc' của Nam Cao?");
    strcpy(quizBank[8].qList[9].answers[0], u8"Chí Phèo");
    strcpy(quizBank[8].qList[9].answers[1], u8"Lão Hạc");
    strcpy(quizBank[8].qList[9].answers[2], u8"Anh Dậu");
    strcpy(quizBank[8].qList[9].answers[3], u8"Binh Tư");
    quizBank[8].qList[9].correctAnswer = 1;

    // --- BỘ 9: CHÚ LAO CÔNG 2 (Chapter 5 - 5 câu An toàn trường học) ---
    quizBank[9].count = 5;
    quizBank[9].passRate = 0.6f;

    strcpy(quizBank[9].qList[0].question, u8"Khi phát hiện hỏa hoạn trong trường, việc đầu tiên cần làm là gì?");
    strcpy(quizBank[9].qList[0].answers[0], u8"Tự dập lửa ngay lập tức");
    strcpy(quizBank[9].qList[0].answers[1], u8"Báo động và sơ tán theo lối thoát hiểm");
    strcpy(quizBank[9].qList[0].answers[2], u8"Thu dọn đồ đạc cá nhân");
    strcpy(quizBank[9].qList[0].answers[3], u8"Gọi điện cho bạn bè");
    quizBank[9].qList[0].correctAnswer = 1;

    strcpy(quizBank[9].qList[1].question, u8"Biển báo màu xanh hình người chạy trong tòa nhà có ý nghĩa gì?");
    strcpy(quizBank[9].qList[1].answers[0], u8"Khu vực tập thể dục");
    strcpy(quizBank[9].qList[1].answers[1], u8"Lối thoát hiểm khẩn cấp");
    strcpy(quizBank[9].qList[1].answers[2], u8"Khu vực cấm chạy");
    strcpy(quizBank[9].qList[1].answers[3], u8"Khu vực dành cho trẻ em");
    quizBank[9].qList[1].correctAnswer = 1;

    strcpy(quizBank[9].qList[2].question, u8"Bình chữa cháy CO2 thường có màu gì?");
    strcpy(quizBank[9].qList[2].answers[0], u8"Xanh lá");
    strcpy(quizBank[9].qList[2].answers[1], u8"Vàng");
    strcpy(quizBank[9].qList[2].answers[2], u8"Đỏ");
    strcpy(quizBank[9].qList[2].answers[3], u8"Trắng");
    quizBank[9].qList[2].correctAnswer = 2;

    strcpy(quizBank[9].qList[3].question, u8"Khi gặp sự cố điện giật, không nên làm gì?");
    strcpy(quizBank[9].qList[3].answers[0], u8"Ngắt cầu dao điện");
    strcpy(quizBank[9].qList[3].answers[1], u8"Dùng tay kéo nạn nhân ra khỏi nguồn điện");
    strcpy(quizBank[9].qList[3].answers[2], u8"Dùng vật cách điện để tách nạn nhân");
    strcpy(quizBank[9].qList[3].answers[3], u8"Gọi cấp cứu ngay");
    quizBank[9].qList[3].correctAnswer = 1;

    strcpy(quizBank[9].qList[4].question, u8"Số điện thoại cứu hỏa tại Việt Nam là số nào?");
    strcpy(quizBank[9].qList[4].answers[0], u8"113");
    strcpy(quizBank[9].qList[4].answers[1], u8"114");
    strcpy(quizBank[9].qList[4].answers[2], u8"115");
    strcpy(quizBank[9].qList[4].answers[3], u8"116");
    quizBank[9].qList[4].correctAnswer = 1;

    // --- BỘ 10: THẦY ANH (Chapter 5 - 7 câu Tiếng Anh) ---
    quizBank[10].count = 7;
    quizBank[10].passRate = 0.6f;

    strcpy(quizBank[10].qList[0].question, u8"'Procrastination' trong tiếng Anh có nghĩa là gì?");
    strcpy(quizBank[10].qList[0].answers[0], u8"Sự chăm chỉ");
    strcpy(quizBank[10].qList[0].answers[1], u8"Sự trì hoãn");
    strcpy(quizBank[10].qList[0].answers[2], u8"Sự cẩn thận");
    strcpy(quizBank[10].qList[0].answers[3], u8"Sự tập trung");
    quizBank[10].qList[0].correctAnswer = 1;

    strcpy(quizBank[10].qList[1].question, u8"Câu nào dưới đây dùng đúng thì Present Perfect?");
    strcpy(quizBank[10].qList[1].answers[0], u8"I have went to school yesterday.");
    strcpy(quizBank[10].qList[1].answers[1], u8"I went to school yesterday.");
    strcpy(quizBank[10].qList[1].answers[2], u8"I have been to Hanoi three times.");
    strcpy(quizBank[10].qList[1].answers[3], u8"I have go to the store.");
    quizBank[10].qList[1].correctAnswer = 2;

    strcpy(quizBank[10].qList[2].question, u8"Từ nào là antonym (trái nghĩa) của 'enormous'?");
    strcpy(quizBank[10].qList[2].answers[0], u8"Huge");
    strcpy(quizBank[10].qList[2].answers[1], u8"Gigantic");
    strcpy(quizBank[10].qList[2].answers[2], u8"Tiny");
    strcpy(quizBank[10].qList[2].answers[3], u8"Vast");
    quizBank[10].qList[2].correctAnswer = 2;

    strcpy(quizBank[10].qList[3].question, u8"Thành ngữ 'It's raining cats and dogs' có nghĩa là gì?");
    strcpy(quizBank[10].qList[3].answers[0], u8"Trời nắng to");
    strcpy(quizBank[10].qList[3].answers[1], u8"Thú cưng đang chạy ngoài mưa");
    strcpy(quizBank[10].qList[3].answers[2], u8"Trời mưa to");
    strcpy(quizBank[10].qList[3].answers[3], u8"Thời tiết lạ lùng");
    quizBank[10].qList[3].correctAnswer = 2;

    strcpy(quizBank[10].qList[4].question, u8"Từ 'Ubiquitous' có nghĩa là gì?");
    strcpy(quizBank[10].qList[4].answers[0], u8"Hiếm gặp");
    strcpy(quizBank[10].qList[4].answers[1], u8"Có mặt khắp nơi");
    strcpy(quizBank[10].qList[4].answers[2], u8"Nguy hiểm");
    strcpy(quizBank[10].qList[4].answers[3], u8"Cổ xưa");
    quizBank[10].qList[4].correctAnswer = 1;

    strcpy(quizBank[10].qList[5].question, u8"Câu bị động đúng của 'They built this school in 1990' là?");
    strcpy(quizBank[10].qList[5].answers[0], u8"This school was built in 1990.");
    strcpy(quizBank[10].qList[5].answers[1], u8"This school is built in 1990.");
    strcpy(quizBank[10].qList[5].answers[2], u8"This school were built in 1990.");
    strcpy(quizBank[10].qList[5].answers[3], u8"This school has been built in 1990.");
    quizBank[10].qList[5].correctAnswer = 0;

    strcpy(quizBank[10].qList[6].question, u8"'Serendipity' có nghĩa là gì trong tiếng Anh?");
    strcpy(quizBank[10].qList[6].answers[0], u8"Sự buồn bã sâu sắc");
    strcpy(quizBank[10].qList[6].answers[1], u8"May mắn tình cờ tìm được điều tốt");
    strcpy(quizBank[10].qList[6].answers[2], u8"Sự kiên trì bền bỉ");
    strcpy(quizBank[10].qList[6].answers[3], u8"Khoảnh khắc hoài niệm");
    quizBank[10].qList[6].correctAnswer = 1;

    // --- BỘ 11: THẦY HIỆU TRƯỞNG (Chapter 6 - 10 câu Tổng hợp) ---
    quizBank[11].count = 10;
    quizBank[11].passRate = 0.6f;

    strcpy(quizBank[11].qList[0].question, u8"Trong lập trình C, con trỏ NULL trỏ đến địa chỉ nào?");
    strcpy(quizBank[11].qList[0].answers[0], u8"Địa chỉ của hàm main()");
    strcpy(quizBank[11].qList[0].answers[1], u8"Địa chỉ 0 (không trỏ đến đâu hợp lệ)");
    strcpy(quizBank[11].qList[0].answers[2], u8"Địa chỉ đầu tiên trong bộ nhớ");
    strcpy(quizBank[11].qList[0].answers[3], u8"Địa chỉ của biến toàn cục");
    quizBank[11].qList[0].correctAnswer = 1;

    strcpy(quizBank[11].qList[1].question, u8"Độ phức tạp thời gian của thuật toán Quick Sort (trung bình) là?");
    strcpy(quizBank[11].qList[1].answers[0], u8"O(n²)");
    strcpy(quizBank[11].qList[1].answers[1], u8"O(n)");
    strcpy(quizBank[11].qList[1].answers[2], u8"O(n log n)");
    strcpy(quizBank[11].qList[1].answers[3], u8"O(log n)");
    quizBank[11].qList[1].correctAnswer = 2;

    strcpy(quizBank[11].qList[2].question, u8"Giao thức nào dùng để truyền trang web an toàn (mã hóa)?");
    strcpy(quizBank[11].qList[2].answers[0], u8"HTTP");
    strcpy(quizBank[11].qList[2].answers[1], u8"FTP");
    strcpy(quizBank[11].qList[2].answers[2], u8"HTTPS");
    strcpy(quizBank[11].qList[2].answers[3], u8"SMTP");
    quizBank[11].qList[2].correctAnswer = 2;

    strcpy(quizBank[11].qList[3].question, u8"FPT University được thành lập năm nào?");
    strcpy(quizBank[11].qList[3].answers[0], u8"2004");
    strcpy(quizBank[11].qList[3].answers[1], u8"2006");
    strcpy(quizBank[11].qList[3].answers[2], u8"2008");
    strcpy(quizBank[11].qList[3].answers[3], u8"2010");
    quizBank[11].qList[3].correctAnswer = 1;

    strcpy(quizBank[11].qList[4].question, u8"Trong hệ nhị phân, số 1010 tương đương với số thập phân nào?");
    strcpy(quizBank[11].qList[4].answers[0], u8"8");
    strcpy(quizBank[11].qList[4].answers[1], u8"10");
    strcpy(quizBank[11].qList[4].answers[2], u8"12");
    strcpy(quizBank[11].qList[4].answers[3], u8"14");
    quizBank[11].qList[4].correctAnswer = 1;

    strcpy(quizBank[11].qList[5].question, u8"Định luật Moore phát biểu điều gì?");
    strcpy(quizBank[11].qList[5].answers[0], u8"Tốc độ internet tăng gấp đôi mỗi 18 tháng");
    strcpy(quizBank[11].qList[5].answers[1], u8"Số transistor trên chip tăng gấp đôi mỗi 2 năm");
    strcpy(quizBank[11].qList[5].answers[2], u8"Dung lượng ổ cứng tăng 10 lần mỗi thập kỷ");
    strcpy(quizBank[11].qList[5].answers[3], u8"Giá máy tính giảm một nửa mỗi năm");
    quizBank[11].qList[5].correctAnswer = 1;

    strcpy(quizBank[11].qList[6].question, u8"Trong mô hình OSI, tầng nào chịu trách nhiệm định tuyến gói tin?");
    strcpy(quizBank[11].qList[6].answers[0], u8"Tầng 2 - Data Link");
    strcpy(quizBank[11].qList[6].answers[1], u8"Tầng 3 - Network");
    strcpy(quizBank[11].qList[6].answers[2], u8"Tầng 4 - Transport");
    strcpy(quizBank[11].qList[6].answers[3], u8"Tầng 5 - Session");
    quizBank[11].qList[6].correctAnswer = 1;

    strcpy(quizBank[11].qList[7].question, u8"'Agile' trong phát triển phần mềm là gì?");
    strcpy(quizBank[11].qList[7].answers[0], u8"Ngôn ngữ lập trình tốc độ cao");
    strcpy(quizBank[11].qList[7].answers[1], u8"Phần mềm chống virus");
    strcpy(quizBank[11].qList[7].answers[2], u8"Phương pháp phát triển linh hoạt theo vòng lặp ngắn");
    strcpy(quizBank[11].qList[7].answers[3], u8"Hệ điều hành nhúng");
    quizBank[11].qList[7].correctAnswer = 2;

    strcpy(quizBank[11].qList[8].question, u8"Cấu trúc dữ liệu nào hoạt động theo nguyên tắc LIFO?");
    strcpy(quizBank[11].qList[8].answers[0], u8"Queue (Hàng đợi)");
    strcpy(quizBank[11].qList[8].answers[1], u8"Stack (Ngăn xếp)");
    strcpy(quizBank[11].qList[8].answers[2], u8"Linked List");
    strcpy(quizBank[11].qList[8].answers[3], u8"Binary Tree");
    quizBank[11].qList[8].correctAnswer = 1;

    strcpy(quizBank[11].qList[9].question, u8"Trong SQL, câu lệnh nào dùng để lấy dữ liệu không trùng lặp?");
    strcpy(quizBank[11].qList[9].answers[0], u8"SELECT UNIQUE");
    strcpy(quizBank[11].qList[9].answers[1], u8"SELECT DISTINCT");
    strcpy(quizBank[11].qList[9].answers[2], u8"SELECT DIFFERENT");
    strcpy(quizBank[11].qList[9].answers[3], u8"SELECT FILTER");
    quizBank[11].qList[9].correctAnswer = 1;
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

    for(int i=0; i<4; i++) hiddenAnswers[i] = false;
    showHint = false;
    skillUsedThisQuestion = false;
    fullHpMsgTimer = 0.0f;
    skillBlockMsgTimer = 0.0f;

    if (pPlayer != NULL) {
        if (pPlayer->cbcStats.maxHp <= 0) pPlayer->cbcStats.maxHp = 5;
        
        if (enemyPtr->id == 99) {
            pPlayer->cbcStats.hp = pPlayer->cbcStats.maxHp;
        } else {
            if (pPlayer->cbcStats.hp <= 0) pPlayer->cbcStats.hp = 1;
        }

        pPlayer->cbcStats.comboCorrect = 0;
        pPlayer->cbcStats.comboWrong = 0;
        pPlayer->cbcStats.skipNext = false;
        pPlayer->cbcStats.canRetry = true;

        // [FIX] Set skillUses đúng theo class
        int pClass = pPlayer->pClass;
        if (pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU) {
            pPlayer->cbcStats.skillUses = 3;
        } else if (pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) {
            pPlayer->cbcStats.skillUses = 2;
        } else {
            pPlayer->cbcStats.skillUses = 2;
        }
    }

    if      (enemyPtr->id == 99)                   currentSetID = 0;
    else if (enemyPtr->id == NPC_CO_THU_KY)        currentSetID = 1;
    else if (enemyPtr->id == NPC_THAY_TUAN_VM)     currentSetID = 2;
    else if (enemyPtr->id == NPC_THAY_CHINH)       currentSetID = 3;
    else if (enemyPtr->id == NPC_THAY_HUNG)        currentSetID = 4;
    else if (enemyPtr->id == NPC_CO_BEP_TRUONG)    currentSetID = 5;
    else if (enemyPtr->id == NPC_CHU_PHU_BEP)      currentSetID = 6;
    else if (enemyPtr->id == NPC_LAO_CONG_MAP4)    currentSetID = 7;
    else if (enemyPtr->id == NPC_CO_THU_THU)       currentSetID = 8;
    else if (enemyPtr->id == NPC_LAO_CONG_MAP5)    currentSetID = 9;
    else if (enemyPtr->id == NPC_THAY_CHU_NHIEM)   currentSetID = 10;
    else if (enemyPtr->id == NPC_THAY_HIEU_TRUONG) currentSetID = 11;
    else currentSetID = 0;
}

void CBC_Update() {
    if (!cbcActive) return;
    float dt = GetFrameTime();
    stateTimer += dt;
    if (fullHpMsgTimer > 0.0f)    fullHpMsgTimer -= dt;
    if (skillBlockMsgTimer > 0.0f) skillBlockMsgTimer -= dt;

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
            bool blockSkill = (pClass == CLASS_ARCHER || pClass == CLASS_PHU_NHI_DAI) && skillUsedThisQuestion;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mPos, skillBtnRec)) {
                if (blockSkill) {
                    // [FIX] Phú Nhị Đại đã dùng skill câu này rồi
                    skillBlockMsgTimer = 2.0f;
                } else if (pPlayer->cbcStats.skillUses > 0) {
                    if ((pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) && pPlayer->cbcStats.hp >= pPlayer->cbcStats.maxHp) {
                        // [FIX] Soái Ca đầy máu -> không trừ lượt
                        fullHpMsgTimer = 2.0f;
                    } else {
                        pPlayer->cbcStats.skillUses--;

                        if (pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU) {
                            for (int i = 0; i < 4; i++) {
                                if (i != correctIdx && !hiddenAnswers[i]) {
                                    hiddenAnswers[i] = true;
                                    break;
                                }
                            }
                        }
                        else if (pClass == CLASS_WARRIOR || pClass == CLASS_HOC_BA) {
                            showHint = true;
                        }
                        else if (pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) {
                            pPlayer->cbcStats.hp++;
                        }
                        else if (pClass == CLASS_ARCHER || pClass == CLASS_PHU_NHI_DAI) {
                            skillUsedThisQuestion = true;
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
                }
            }

            // --- 2. XỬ LÝ CHỌN ĐÁP ÁN ---
            for (int i = 0; i < 4; i++) {
                if (!hiddenAnswers[i] && CheckCollisionPointRec(mPos, answerRecs[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedAnswer = i;
                }
            }
            if (IsKeyPressed(KEY_ONE)   && !hiddenAnswers[0]) selectedAnswer = 0;
            if (IsKeyPressed(KEY_TWO)   && !hiddenAnswers[1]) selectedAnswer = 1;
            if (IsKeyPressed(KEY_THREE) && !hiddenAnswers[2]) selectedAnswer = 2;
            if (IsKeyPressed(KEY_FOUR)  && !hiddenAnswers[3]) selectedAnswer = 3;

            // --- 3. XỬ LÝ ĐÚNG/SAI VÀ NỘI TẠI ---
            if (selectedAnswer != -1) {
                bool isCorrect = (selectedAnswer == correctIdx);

                if (!isCorrect && (pClass == CLASS_WARRIOR || pClass == CLASS_HOC_BA) && pPlayer->cbcStats.canRetry) {
                    pPlayer->cbcStats.canRetry = false;
                    hiddenAnswers[selectedAnswer] = true;
                    selectedAnswer = -1;
                }
                else {
                    if (isCorrect) {
                        correctCount++;
                        pPlayer->cbcStats.comboCorrect++;
                        pPlayer->cbcStats.comboWrong = 0;

                        if ((pClass == CLASS_STUDENT || pClass == CLASS_DAU_GAU) && pPlayer->cbcStats.comboCorrect == 3) {
                            if (pPlayer->cbcStats.hp < pPlayer->cbcStats.maxHp) pPlayer->cbcStats.hp++;
                            pPlayer->cbcStats.comboCorrect = 0;
                        }
                        if ((pClass == CLASS_MAGE || pClass == CLASS_SOAI_CA) && pPlayer->cbcStats.comboCorrect == 3) {
                            pPlayer->cbcStats.skipNext = true;
                            pPlayer->cbcStats.comboCorrect = 0;
                        }
                    } else {
                        pPlayer->cbcStats.hp--;
                        pPlayer->cbcStats.comboWrong++;
                        pPlayer->cbcStats.comboCorrect = 0;

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
                    pPlayer->cbcStats.hp = 1;
                    isPass = false;
                    cbcState = CBC_STATE_FINAL;
                } else {
                    currentQuestion++;
                    selectedAnswer = -1;
                    pPlayer->cbcStats.canRetry = true;

                    for(int i=0; i<4; i++) hiddenAnswers[i] = false;
                    showHint = false;
                    skillUsedThisQuestion = false;

                    if (pPlayer->cbcStats.skipNext) {
                    pPlayer->cbcStats.skipNext = false;
                    if (currentQuestion < quizBank[currentSetID].count) {
                            currentQuestion++;
                            correctCount++;
                        }
                    }

                    if (currentQuestion >= quizBank[currentSetID].count) {
                        isPass = true;
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
                if (isPass && pEnemy) pEnemy->isDead = true;
                if (!isPass) cbcJustLost = true;
            }
            break;
    }
}

void CBC_Draw() {
    Color bgParchment = (Color){ 238, 195, 134, 255 };
    Color woodBorder  = (Color){ 186, 104, 34, 255 };
    Color darkBorder  = (Color){ 105, 50, 15, 255 };
    Color btnNormal   = (Color){ 220, 145, 40, 255 };
    Color btnHover    = (Color){ 245, 175, 70, 255 };
    Color textColor   = (Color){ 60, 30, 10, 255 };

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    Rectangle mainRec = { 80, 70, 640, 340 };
    DrawRectangleRec((Rectangle){ mainRec.x - 4, mainRec.y - 4, mainRec.width + 8, mainRec.height + 8 }, darkBorder);
    DrawRectangleRec((Rectangle){ mainRec.x - 2, mainRec.y - 2, mainRec.width + 4, mainRec.height + 4 }, woodBorder);
    DrawRectangleRec(mainRec, bgParchment);
    DrawRectangleLinesEx((Rectangle){ mainRec.x + 4, mainRec.y + 4, mainRec.width - 8, mainRec.height - 8 }, 2, woodBorder);

    if (cbcState == CBC_STATE_INTRO) {
        DrawTextEx(globalFont, u8"- SỰ KIỆN VẤN ĐÁP -", (Vector2){ 260, 110 }, 28, 1, textColor);

        char targetText[150];
        sprintf(targetText, u8"Hệ thống yêu cầu bạn trả lời %d câu hỏi.\nSinh lực hiện tại: %d máu.\n\nBạn đã sẵn sàng chưa?", quizBank[currentSetID].count, pPlayer->cbcStats.hp);
        DrawTextEx(globalFont, targetText, (Vector2){ 120, 170 }, 24, 1, textColor);

        Rectangle startBtn = { 250, 310, 300, 45 };
        DrawRectangleRec(startBtn, btnNormal);
        DrawRectangleLinesEx(startBtn, 3, darkBorder);
        DrawTextEx(globalFont, u8"Bắt đầu (Click / Enter)", (Vector2){ 280, 320 }, 22, 1, textColor);
    }
    else if (cbcState == CBC_STATE_QUESTION) {
        Rectangle tabRec = { 460, 20, 260, 45 };
        DrawRectangleRec((Rectangle){ tabRec.x - 4, tabRec.y - 4, tabRec.width + 8, tabRec.height + 8 }, darkBorder);
        DrawRectangleRec((Rectangle){ tabRec.x - 2, tabRec.y - 2, tabRec.width + 4, tabRec.height + 4 }, woodBorder);
        DrawRectangleRec(tabRec, bgParchment);

        char statsText[50];
        sprintf(statsText, u8"Máu: %d/%d | Skill: %d", pPlayer->cbcStats.hp, pPlayer->cbcStats.maxHp, pPlayer->cbcStats.skillUses);
        DrawTextEx(globalFont, statsText, (Vector2){ tabRec.x + 20, tabRec.y + 12 }, 22, 1, textColor);

        char qTitle[50];
        sprintf(qTitle, u8"- CÂU %d/%d -", currentQuestion + 1, quizBank[currentSetID].count);
        DrawTextEx(globalFont, qTitle, (Vector2){ 100, 90 }, 22, 1, textColor);
        DrawTextEx(globalFont, quizBank[currentSetID].qList[currentQuestion].question, (Vector2){ 100, 130 }, 24, 1, textColor);

        Vector2 mPos = GetVirtualMousePos();
        const char* labels[] = {"A.", "B.", "C.", "D."};

        for (int i = 0; i < 4; i++) {
            if (hiddenAnswers[i]) continue;

            bool hover = CheckCollisionPointRec(mPos, answerRecs[i]);
            Color cardColor = hover ? btnHover : btnNormal;

            if (showHint && i == quizBank[currentSetID].qList[currentQuestion].correctAnswer)
                cardColor = (Color){ 160, 220, 100, 255 };

            DrawRectangleRec(answerRecs[i], cardColor);
            DrawRectangleLinesEx(answerRecs[i], 3, darkBorder);

            char ansFull[150];
            sprintf(ansFull, "%s  %s", labels[i], quizBank[currentSetID].qList[currentQuestion].answers[i]);
            DrawTextEx(globalFont, ansFull, (Vector2){ answerRecs[i].x + 15, answerRecs[i].y + 10 }, 22, 1, textColor);
        }

        if (pPlayer->cbcStats.skillUses > 0) {
            bool hoverBtn = CheckCollisionPointRec(mPos, skillBtnRec);
            DrawRectangleRec(skillBtnRec, hoverBtn ? btnHover : btnNormal);
            DrawRectangleLinesEx(skillBtnRec, 2, darkBorder);
            DrawTextEx(globalFont, u8"Kỹ Năng", (Vector2){ skillBtnRec.x + 35, skillBtnRec.y + 8 }, 20, 1, textColor);
        }

        // Thông báo đầy máu (Soái Ca)
        if (fullHpMsgTimer > 0.0f) {
            DrawTextEx(globalFont, u8"Đã đầy máu!",
                       (Vector2){ skillBtnRec.x - 10, skillBtnRec.y + 40 }, 18, 1, RED);
        }
        // Thông báo chặn skill (Phú Nhị Đại)
        if (skillBlockMsgTimer > 0.0f) {
            DrawTextEx(globalFont, u8"Mỗi câu chỉ dùng skill này 1 lần!",
                       (Vector2){ skillBtnRec.x - 110, skillBtnRec.y + 40 }, 18, 1, RED);
        }
    }
    else if (cbcState == CBC_STATE_RESULT) {
        bool correct = (selectedAnswer == quizBank[currentSetID].qList[currentQuestion].correctAnswer);

        DrawTextEx(globalFont, correct ? u8"- CHÍNH XÁC -" : u8"- SAI RỒI -",
                   (Vector2){ 310, 170 }, 30, 1, correct ? (Color){ 20, 100, 20, 255 } : RED);

        char rightAns[100];
        sprintf(rightAns, u8"Đáp án đúng là: %d", quizBank[currentSetID].qList[currentQuestion].correctAnswer + 1);
        DrawTextEx(globalFont, rightAns, (Vector2){ 290, 240 }, 24, 1, textColor);

        DrawTextEx(globalFont, u8"(Bấm để tiếp tục)", (Vector2){ 320, 320 }, 20, 1, darkBorder);
    }
    else if (cbcState == CBC_STATE_FINAL) {
        DrawTextEx(globalFont, isPass ? u8"- VƯỢT ẢI THÀNH CÔNG -" : u8"- THẤT BẠI -",
                   (Vector2){ 250, 160 }, 30, 1, isPass ? (Color){ 20, 100, 20, 255 } : RED);

        char scoreText[50];
        sprintf(scoreText, u8"Bạn trả lời đúng: %d / %d", correctCount, quizBank[currentSetID].count);
        DrawTextEx(globalFont, scoreText, (Vector2){ 270, 240 }, 24, 1, textColor);

        DrawTextEx(globalFont, u8"(Bấm để thoát)", (Vector2){ 330, 330 }, 20, 1, darkBorder);
    }
}

bool CBC_IsJustLost(void) {
    if (cbcJustLost) { cbcJustLost = false; return true; }
    return false;
}
bool CBC_IsActive() { return cbcActive; }
void CBC_Shutdown() { }