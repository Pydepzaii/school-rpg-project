# ⚔️ PROJECT GAME OSG - HƯỚNG DẪN & LOG

## 🛠 1. Thiết lập môi trường Code

Sử dụng **VS Code** đã cài môi trường lập trình C.

Để kiểm tra đã cài trình biên dịch chưa, hãy mở terminal (cmd) và gõ:
```bash
gcc --version
```
> Nếu hiện `xx.xx.xx` là OK. Nếu chưa ra thì lên AI hỏi cách cài.
> *Note: Phiên bản gcc của Trung là 14.2.0*

### Các bước cấu hình VS Code:

**B1:** Mở VS Code. Tại giao diện làm việc nhấn tổ hợp `CTRL + SHIFT + P`.

**B2:** Trên thanh tìm kiếm gõ: `Edit Configurations`.

**B3:** Chọn dòng **C/C++: Edit Configurations (UI)** hoặc **(JSON)**. Copy đoạn mã sau vào file `c_cpp_properties.json` (nếu file không tự mở thì mở bừa 1 folder lên rồi làm lại nó sẽ mở ngay):

```json
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

**B4:** Nhấn `CTRL + S` rồi thoát VS Code.

**B5:** Tạo 1 file `helloworld.c` và test, nếu chạy được là OK.

---

## 📂 2. Cấu trúc chương trình cơ bản

- Mọi hình ảnh, âm thanh, animation, v.v... đều phải để trong thư mục `resources`.
- Code viết vào các file `.c` hoặc `.h` trong thư mục `src`.
- **Quy tắc tạo file:**
    - Khi tạo file mới chưa tồn tại trong `src`, phải làm thành cặp `.c` và `.h`, **không tự ý tạo lẻ**.
    - Khi tạo file mới, nhớ khai báo đường dẫn vào file `tasks.json` (hoặc file config build tương ứng) trong thư mục `.vscode`. Nếu không file sẽ không chạy.
    - *Nếu không biết khai báo thì nhờ AI.*

---

## 🚀 3. Quy trình Github Desktop

### a. Cho Developer (Code)

**B1:** Chọn **Current branch** -> **New branch** -> Đặt tên và tạo nhánh.

**B2:** Sửa hoặc thêm code.

**B3:** Code xong thì ghi **Commit** và **Push** lên nhánh vừa tạo.

**B4:** Lên trang Github tìm **Pull requests** -> Chọn nút xanh **New pull request**.
- Để `base: main` và `compare: nhánh-vừa-tạo`.
- Ấn **Create pull request**.

**B5:** Add title (có thể giữ nguyên), ghi **Description** là những gì bạn vừa làm/fix -> Hoàn thiện thì ấn **Create pull request**.

**B6:** Chờ anh Trung review và feedback.

### b. Cho Designer (Art)

**B1:** Chọn **Current branch** -> `art` -> **Fetch origin** (tải phần của người trước về).

**B2:** Sửa hoặc thêm art.

**B3:** Xong thì ghi commit và push lên nhánh `art` (Commit to art -> Push origin).

**B4:** Nếu muốn sửa/thêm hoặc bị anh Trung đòi sửa art thì làm lại như bước đầu.

### c. Cho anh Trung
- Review code và Merge vào Main.

---

## 🎮 4. Debug Tool & Developer Guide (Quan trọng)

### ⌨️ Phím tắt Debug cần nhớ

| Phím | Chức năng | Ghi chú |
| :--- | :--- | :--- |
| **0** | Bật/Tắt Tool vẽ tường (Map Tool) | Chế độ tường va chạm |
| **P** | Bật/Tắt Tool cắt ảnh (Prop Tool) | Chế độ Render vật thể |
| **=** | Bật/Tắt Tool quản lý Menu | Chế độ chỉnh UI |
| **S** | **LƯU MỌI THAY ĐỔI** | Lưu vào file data (Wall, Prop, Menu) |
| **DEL / K** | Bật chế độ XÓA | Click vào đối tượng để xóa |
| **C** | Undo | Xóa nhanh đối tượng nháp vừa vẽ |
| **X** | Bật/Tắt khung giới hạn frame | Debug Animation |
| **V** | Ẩn/Hiện bảng thông tin Tool | |

### 🛠 Hướng dẫn thêm nút chức năng mới (Sync với Debug Tool)
Để thêm một nút mới (ví dụ: nút "HELP") mà không làm hỏng hệ thống, hãy làm đúng theo 3 bước "Thần chú" sau:

**Bước 1: Khai báo trong Code (Backend)**
- **Thêm ID:** Mở `menu_system.h`, thêm `ACT_OPEN_HELP` vào enum.
- **Đăng ký tên:** Vào `menu_system.c`, thêm case cho `ACT_OPEN_HELP` trong hàm `GetActionName` để Tool Debug hiển thị chữ "HELP".
- **Thêm Logic:** Viết code xử lý trong hàm `ProcessButtonAction` (`menu_system.c`).
- **Đồng bộ Tool:** Mở `debug.c`, thêm `ACT_OPEN_HELP` vào mảng tương ứng (actionsForPause hoặc actionsForTitle) và tăng `currentValidActionCount`.

**Bước 2: Tạo nút bằng Tool (Frontend)**
1.  Compile lại code và mở game.
2.  Đến Menu cần thêm nút -> Bấm **`=`** bật Debug Tool.
3.  **Tạo nút:** Kéo chuột trái vẽ hình chữ nhật.
4.  **Gán chức năng:** Click Chuột Phải vào nút vừa vẽ -> Menu Popup hiện ra -> Chọn "HELP".

**Bước 3: Lưu lại**
- Nhấn **`S`** để Tool tự động ghi tọa độ và ID vào `resources/data/menus.txt`.

> **Lưu ý cho dev:** Khi thêm nút, nhớ dùng hàm `GetVirtualMousePos()` để tọa độ luôn chuẩn xác kể cả khi phóng to/thu nhỏ màn hình.

---

## 📜 5. Update Log (Lịch sử cập nhật)

### Ver 0.0.1 - 0.0.3 (Khởi động)
**Ver 0.0.1**
- Thiết lập khung, bản đồ "thư viện" test, hệ thống NPC, tool lấy tọa độ cơ bản.

**Ver 0.0.2**
- Thêm Intro Logo, font "Roboto" hỗ trợ tiếng Việt.

**Ver 0.0.3**
- Update Debug Menu (lấy tọa độ UI), Title Screen cơ bản, nút thoát game.

### 🔥 BIG UPDATE Ver 0.0.4
- Đã thêm font tiếng Việt chuẩn và ổn định.
- Tính năng toàn màn hình (Fullscreen) hoạt động ổn định với scale 100%.
- Logic chuẩn map dùng cho tương lai.
- Model đọc file viết thoại hỗ trợ Art team.
- **Debug Update:** Debug on map/menu hoạt động tốt trên chế độ Window (Không mở fullscreen để debug vì tọa độ ảo có thể sai lệch).
- *Tip: Để test chuẩn map, đến vùng exit thư viện nhấn "E", muốn về lại nhấn "E".*

### 🔥 BIG UPDATE Ver 0.0.5
- Menu Setting: Chỉnh Master Volume, chuyển chế độ Window/Fullscreen.
- Nhạc nền test cho Title Screen và Map Test.
- Animation chuyển cảnh game.
- Camera Zoom kéo theo nhân vật (Bấm **`Y`** để về chế độ toàn bản đồ - Debug only).
- Tool lấy tọa độ đã hoạt động tốt trên cả Fullscreen và Window.
- Fix lỗi màn hình trên các máy có scale > 100%.

### Ver 0.0.6
- Update Animation: 6 lên 22 frame (chia giai đoạn đứng yên/đi bộ).
- Tool Debug: Thêm xem cửa ra hitbox với độ chính xác tuyệt đối.
- Bấm **`0`** cho Debug Map và **`=`** cho Debug Menu.
- Refactor Code: Áp dụng OOP, sửa biến số linh hoạt hơn.

### -------------------- BETA STAGE --------------------

### 🔥 BIG UPDATE Ver 0.1 (Gameplay & Flowchart)
**GAMEPLAY:**
- Cập nhật flow: Title -> Start -> Chọn Class -> Xác nhận -> (Cutscene - Dev by Đạt) -> Game.
- Thêm 3 map: Alpha -> Nhà Vệ Sinh -> Thư Viện. (Dùng F1, F2, F3 để chuyển map - Debug only).
- Di chuyển bằng WASD (Nhớ tắt Unikey).
- Tốc độ di chuyển giảm từ 4.0 về 2.5 theo feedback.
- Camera Zoom tăng độ zoom để tạo cảm giác map to hơn.
- Giảm bán kính tương tác NPC/Cửa (phải lại gần mới bấm E được).

**GRAPHIC:**
- Update texture hộp thoại.
- Hiệu ứng chuyển cảnh Intro -> Title -> Menu Pause mượt mà.
- Video Intro giảm xuống 480p (tối ưu GPU), chỉnh sửa đoạn kết cho chuyên nghiệp.
- Hiệu ứng vòng tròn ma thuật "ảo ma Canada" lúc chọn class.

**DEBUG TOOL:**
- Mode Zero (`0`) hiển thị rõ phạm vi tương tác, thêm chấm đỏ tâm để dễ fix lỗi.
- Cải thiện độ chính xác, chuẩn bị cho render 2.5D.

**LƯU Ý CHỌN CLASS:**
- Hiện tại chỉ có Class 1 & 2 là có ảnh Main. Chọn Class 3 & 4 sẽ tàng hình (Tính năng, không phải lỗi). *Ae tự hỏi sao tôi không copy con main1 ra thì do tôi không thích được chưa!*

### Ver 0.2 (Render & Save System)
**GRAPHIC:**
- Hoàn thành hệ thống render cho nhân vật, NPC, đồ vật (trước/sau).

**SYSTEM & FIXBUG:**
- Menu Setting: Chỉnh riêng biệt Master Volume, BGM, SFX.
- Save/Load Game: Save trong Menu Pause, Load ở Title Screen (file `save_data.dat`).

**DEBUG TOOL:**
- Hỗ trợ chế độ lấy tọa độ render (Phím `P`).

### Ver 0.2.1
- Thêm túi đồ (Inventory) - Bấm **`B`** để mở.
- Fix lỗi hiển thị infobox tool `P` trong map.

### Ver 0.3 (Debug Tool Overhaul)
**Chế độ Tường (`0`):**
- Vẽ tường nháp (Xanh lục) -> Hiện nút Save -> Lưu thẳng vào code (không cần copy paste thủ công).
- Bấm **`K`** để xóa tường đã lưu (Đỏ).
- Bấm **`C`** để undo tường nháp.

**Chế độ Render (`P`):**
- Tương tự như chế độ tường.

**Chế độ UI (`=`):**
- Vẽ nút và đặt tùy ý, không cần chỉnh tọa độ code.

### Ver 0.4.1
**DEBUG TOOL:**
- Vẽ menu hỗ trợ thanh trượt (slider) và công tắc (toggle) cho Setting.
- Sửa lỗi hiển thị.

**GRAPHIC:**
- Thêm các map còn lại (F1 - F6 để chuyển). Hiện chưa có cửa thông map.
### Ver 0.5
 **Gameplay:**
 -Đã thêm hệ thống combat sơ khai
### Ver 0.6
**DEBUG TOOL:**
-Thêm tính năng debug cho hội thoại. Giờ đây mọi người có thể thêm thoại trực tiếp vào game trên giao diện người dùng
-Cách sử dụng: khi nói chuyện với NPC phím 'E', dồng thời nhấn shift+d để mở giao diện dialog debug tool
-trên giao diện mọi người có thể chỉnh từng câu thoại, thêm sự kiện mới hoạc xóa câu thoại đó(nhớ save lại nhé)
 **Gameplay:**
 -Đã thêm hầu hết NPC và Item để test
 ### Ver 0.6.5
 **Gameplay:**
 -Hoàn thành menu chọn class có kèm animation
 ### Ver 0.7
 **GRAPHIC:**
 -Hoàn thành main menu cùng với texture nút bấm
 -tăng cường auraanimation tại selection menu
 -đã cài 2 icon animation cho các nút back và next
 **Gameplay:**
 - nâng cấp hội thoại từ bản liner(tuyến tính 1 dòng chuyện) sang bản rẽ nhánh nhiều lựa chọn
 - sửa cơ chế đọc thoại thay vì ấn E liên tục giờ có thể thao tác bằng chuột
 **DEBUG TOOL:**
 - Nâng cấp cho chế độ dialig debug tool giờ đây có thể khởi tạo sự kiện rẽ nhánh theo ý muốn.
### Ver 0.8
**GRAPHIC:**
-Đã nâng UI lên bản chính thức
-Thêm rất nhiều icon và animation của UI
**MENU:**
-Đẫ thêm 2 chức năng mute music và mute sfx
-Đã thêm menu info để xem thông tin