
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



