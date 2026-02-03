
# Cách dùng file project gameOSG

## 1. Cách thiết lập môi trường code:

Sử dụng VS code đã cài môi trường lập trình C:

Để kiểm tra đã cài trình biên dịch chưa hẫy làm như sau, mở cmd:

```bash
gcc --version
```
thấy hiện xx.xx.xx là ok còn chưa ra thì lên AI hỏi cách cài. phiên bản gcc của trung 14.2.0

**B1: Mở VS code tại giao diện làm việc nhấn tổ hợp CTR+SHIFT+P**

**B2: Trên thanh tìm kiếm gõ:"Edit Configurations"**

**B3:Chọn dòng C/C++ edit configuration(json) rồi copy đoạn mã sau vào file c/cpp propoties nếu file không tự mở thì mở bừa 1 folder lên ròi làm lại nó sẽ mở ngay**

```bash
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/include",
                "${workspaceFolder}/src"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:\\msys64\\ucrt64\\bin\\gcc.exe",
            "cStandard": "c17",
            "cppStandard": "gnu++17",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```
**B4: Nhấn CRT+S rồi thoát vs code**

**B5: tạo 1 file `helloworld.c` và test nếu chạy dc là ok.**



## 2. Cấu trúc chương trình cơ bản
- Mọi hình ảnh âm thanh animetion, vvv đều phải để trong resource
- Code viết vào các file .C hoạc.h trong thư mục src
- Khi tạo file mới mà chưa tồn tại trong src thì làm thành cặp .c và .h không tự ý tạo lẻ
- Khi tạo file mới trong src nhớ khai báo đường dẫn vào file config.jasion trong thư mục .vscode(nếu không file sẽ không chạy)
- Nếu không biết khai báo thì nhờ AI

## 3. Sử dụng github desktop để up file

### a. Cho bên dev

**B1:** Chọn Current branch -> New branch -> đặt tên và tạo nhánh.

**B2:** Sửa hoặc thêm code.

**B3:** Code xong thì ghi commit và push lên nhánh vừa tạo.

**B4:** Lên web vào trang github tìm pull requests -> chọn nút màu xanh New pull requests -> Để base là main và compare là nhánh bạn vừa tạo -> Create pull requests.

**B5:** Add title (có thể giữ nguyên), ghi desception là những gì bạn vừa làm vừa fix -> hoàn thiện thì ấn Create pull requests.

**B6:** Chờ a Trung review và feedback.

### b. Cho bên design

**B1:** Chọn Current branch -> art -> Fetch origin (tải phần của người trước về).

**B2:** Sửa hoặc thêm art.

**B3:** Xong thì ghi commit và push lên nhánh art (commit to art -> Push origin).

**B4:** Nếu muốn sửa hoặc thêm hoặc bị a Trung đòi art thì làm lại như bước đầu.

### c. Cho a Trung khi review code và merge vào main

## 4. Update note
### Ver 0.0.1
**Thiết lập khung chương trình cơ bản.
**Thêm bản đô "thư viện" thử nghiệm.
**Thêm hệ thống tương tác npc.
^^Thêm tool lấy tọa độ bằng kéo thả chuột hỗ trợ dev game.
### Ver 0.0.2
**Thêm intro logo nhóm khi chạy game
**Thêm font "roboto" hỗ trợ thoại và giao diện tiếng việt
### Ver 0.0.3
**Update debug menu giờ đây đã hỗ trợ cho lấy tọa độ menu UI
**Thêm title screen cơ bản
**Thêm nút thoát game và fix 1 số lỗi
### BIGUPDATE Ver 0.0.4
**Đa thêm font tiếng việt chuẩn và ổn định
**Đã thêm tính năng toàn màn hình hoạt động ổn định với scale 100%
**Đã thêm logic chuẩn map dùng cho tương lai
**Đã thêm model đọc file viết thoại hỗ trợ artteam viết thoại
**Debug on map và on menu đã hoạt động tốt trên chế độ window(không mở fullscreen để dubug vì fullscreen là vẽ lên màn hình ảo nên tọa độ lấy được có thể sai lệch vấn đề này sẽ update lại tool sau)
**Để test chuẩn map đến vùng exit của thư viện sau đó nhấn "E" muốn về thư viện nhấn "E" lần nữa
### BIGUPDATE ver 0.0.5
**Thêm menu setting cho phép chỉnh master volume và chuyển chế độ window, fullscreen
**Thêm nhạc nền test cho tilte screen và map test
**Thêm animation chuyển cảnh game
**Thêm camera zoom kéo theo nhân vật(bấm y để về chế độ toàn bản đồ chỉ dành cho debug và tester)
**Nâng cấp tool lấy tọa độ giờ đây đã hoạt động tốt trên cả chế độ toàn màn hình và chế độ cửa sổ
**Sửa lỗi màn hình trên cấc máy thiết lập scale>100%
### ver 0.0.6
**Animation update 6 lên 22 flame chia thành dai đoạn đứng yên và đi bộ
**tool debug update giờ đây có thêm xem cửa ra hitbox với độ chính xác tuyệt đối
***bấm '0' cho debug map và '=' cho debug mmenu.
**đã sửa 1 số biến số trong code giờ đây đã liên kết linh hoạt hơn giữa các file áp dụng kiến thức OOB sửa 1 chỗ được tất cả.



# --------------------BETA--------------------#
# --bắt đầu code theo flowchar---
## BIGUPDATE ver 0.1
### GAMEPLAY:
**Cập Nhật tiến trình cụ thể theo flowchart: titlescreen->start->chọn class->xác nhận class->(cut scenne đang được phát triển bởi đạt)->vào game
**Thêm 3 map được sắp xếp theo thứ tự: alpha->nhavo->thuvien. dùng phím F1, F2, F3 để chuyển map(debug only)
**Cập nhật dùng phím WASD để di chuyển(nhớ tắt unikey hoạc gõ tiếng việt)
**Theo 1 số phản ánh của tester Trung đã điều chỉnh tốc độ di chuyển từ 4.0 về 2.5
**camera zoom bám theo main CHÍnh đã được tăng độ zoom để tạo cảm giác map to hơn
**Tất nhiên có map mới là phải có nhạc mới hẹ hẹ
**Đã giảm bán kính phạm vi tương tác của npc và cửa ra vào giờ đây phải đến gần hơn mới bấm E tương tác được
### GRAPHIC
**Đã cập nhật texture hộp thoại của team art
**Thêm hiệu ứng chuyển cạnh cho intro vào titlescreen cho mượt với từ menu POUSE về titlescreen
**Điều chỉnh nhỏ cho video Intro ha độ phân giải từ 720p xuống 480p cho phù hợp với thiệt lập cửa sổ và giảm gánh nặng cho GPU đồng thời chỉnh sửa video một chút để cuối logo và âm thanh dần biên mất nhìn cho chuyên nghiệp
**Thêm hiệu ứng vòng tròn ma thuật lúc chọn class và chuyển cảnh vào game siêu ảo ma canada
### DEBUG TOOL
**Nâng cấp hệ thống DEBUG TOOL mode ZERO(phím 0) giờ đây có thể hiển thị rõ ràng phạm vi tương tấc của npc và cửa ra vào để bấm E đồng thời kèm thêm 1 chấm màu đỏ ở tâm vòng tròn tương tác để sau này dễ fix lỗi tính phạm vi tương tác
**Tool cũng đã được cải thiện đáng kể vể độ chính xác và đồng bộ với các thông số của src code chuẩn bị cho Đạt vẽ tường va chạm và render 2.5D
### SYSTEM AND FIXBUG
**Đã dọn dẹp hàm main tách đa số logic không càn thiết trong main sang file gameplay.c và gameplay.h
**Đã thiết kế sắp xếp lại folder resources để dễ dàng quản lí tài nguyên đồ họa và âm thanh không còn lẫn lộn như bản trước
**Đã khắc phục lỗi tính toán phạm vi tương tác bản trước tâm phạm vi tương tác nằm ở góc trên trái ảnh giờ đã sửa công thức tính để tâm vòng tròn rơi vào đúng giữa hitbox của NPC đồng thời cũng sửa tương tự với hitbox của player cho đồng bộ
### LƯU Ý QUAN TRONG VỀ CHỌN CLASS
**Hiện tại trung mới nhận được 2 ảnh main1 và main2 phù hợp tiêu chuẩn đê cho vào class 1 và 2 nếu ae chọn class 3 và 4 thì chưa có ảnh nào load vào đâu nên con main sẽ vô hình nếu chọn class 3 và 4(đây là tính năng không phải lôi). ae tự hỏi sao tôi không copy con main1 ra thì do tôi không thích được chưa! 
<<<<<<< Updated upstream
### Nhắn nhủ team art mọi ảnh trong tương lai đều phải tuyệt đối tuân thủ quy tác tỉ lệ của các ảnh trong thư mục resources kể cả số fflame tôi lười sửa code lắm rồi

=======
## UPDATE ver 0.2
### GRAPHIC
**Đã hoàn thành hệ thống render cho nhân vật, npc và đồ vật trên bản đồ
### SYSTEM AND FIXBUG
**Menu setting giờ đây đã có thể mute master volume, tăng giảm BGM và SFX 1 cách riêng biệt
**Đã thêm tính năng savegame trong menu menu POUSE và tính năng loadgame trong titilescreen. save được lưu trong save_date.bat
### DEBUG TOOL
**Nâng cấp debug tool giờ đây đã hỗ trợ chế độ lấy tọa độ render cho vật thể
### nhắc lại 1 số phím tắt:
****Phím 0 : Mở chế độ kéo thả lấy tọa độ tường map
*****Phím C: Undo tường nhap vừa vẽ
*****Phím X: Bật tắt khung giới hạn 1 flame nhân vật
*****Phím V: Ẩn hiện bảng thông tin của tool
****Phím =: Mở chế độ debug menu kéo thả để lấy nút
*****Phím C: Undo
*****Phím V: Tắt info tool
****Phím P: Mở chế độ render debug tool
*****Phím C: undo
>>>>>>> Stashed changes
