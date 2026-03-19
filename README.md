# ⚔️ PROJECT GAME OSG - HƯỚNG DẪN & LOG

## 📜 Update Log (Lịch sử cập nhật)

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
### Ver 0.9 BIUUPDATE
**GRAPHIC:**
-Đã nâng cấp một số animation dịch chuyển và hiệu ứng nhặt đồ
**GAMEPLAY**
-Đã cập nhật hoàn tất cốt truyện
-đã nâng cấp giao diện và bộ cấu hỏi combatbychatting
-Đã thêm ending game outro
**System**
-Đã nâng cấp hệ thống save gamme giờ đây có thể lưu trạng thái cốt truyện khi save và load
**Fix bug**
-khắc phục 1 số lỗi khi loadgame
-khắc phục các sự cố về hiển thị menu
