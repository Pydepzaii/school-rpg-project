Update cách dùng file project gameOSG
1. cách thiết lập môi trường code:
	-Sử dụng VS code đã cài môi trường lập trình C
		+để kiểm tra đã cài trình biên dịch chưa hẫy làm như sau:
		 (mở CMD gõ gcc --version) thấy hiện xx.xx.xx là ok còn 
		 chưa ra thì lên AI hỏi cách cài. phiên bản gcc của trung 14.2.0
		+Cấu hình(config để VS code chạy được C và load được raylib)
		B1 mở VS code tại giao diện làm việc nhấn tổ hợp CTR+SHIFT+P
		B2 Trên thanh tìm kiếm gõ:"Edit Configurations"
		b3 Chọn dòng C/C++ edit configuration(jasion) rồi copy đoạn mã sau vào file c/cpp propoties
		nếu file không tự mở thì mở bừa 1 folder lên ròi làm lại nó sẽ mở ngAy
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

		B4 Nhấn CRT+S rồi thoát vs code
		B5 tạo 1 file hello world.c và test nếu chạy dc là ok
	-Cách dùng AI đúng cách:
		+Trước khi nhờ AI code hãy copy hàng loạt thư mục trong resources và src và .vscode ném lên AI
		(khuyên dùng gemini vì tải được nhiều file 1 lúc nếu có free 1 năm)
		+Khi code xong dù bằng tay hay AI cũng nhớ ghi chú để có lỗi biết chỗ mà fix
		+nhớ clone ra máy rồi chạy không được làm trên file github chạy ok báo cáo rồi mới up lên
2. Cấu truc chương trình cơ bản
	-Mọi hình ảnh âm thanh animetion, vvv đều phải để trong resource
	-Code viết vào các file .C hoạc.h trong thư mục src
	-khi tạo file mới mà chưa tồn tại trong src thì làm thành cặp .c và .h không tự ý tạo lẻ
	-khi tạo file mới trong src nhớ khai báo đường dẫn vào file config.jasion trong thư mục .vscode(nếu không file sẽ không chạy)
		+Nếu không biết khai báo thì nhờ AI

QUAN TRONG MỖI KHI HỎI AI ĐỪNG TỰ Ý CRT+A RỒI DÁN ĐÈ RẤT DỄ LỖI NÊN DÁN TỪNG PHẦN NHÉ
											  VIẾT BỞI
											HÀ QUỐC TRUN