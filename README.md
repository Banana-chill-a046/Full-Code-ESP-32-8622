# Full-Code-ESP-32-8622

Repository tổng hợp toàn bộ mã nguồn lập trình, bài hướng dẫn kỹ thuật và dự án thực tế dành cho các dòng vi điều khiển **ESP8266** và **ESP32**, được phát triển và chia sẻ bởi **An Nam PC**.

<img width="500" height="444" alt="image" src="https://github.com/user-attachments/assets/296134ae-188e-4a57-9580-086b15f37a04" />


### 📌 Giới Thiệu Chung

Dự án này được tạo ra nhằm cung cấp giải pháp lập trình vi điều khiển ứng dụng thực tế từ cơ bản đến nâng cao. Tất cả mã nguồn trong kho lưu trữ đều được tối ưu hóa, chú thích rõ ràng và kiểm thử trực tiếp trên phần cứng trước khi tải lên.

Các mảng nội dung chính bao gồm:
* **IoT & Web Server:** Dựng giao diện điều khiển, giám sát thiết bị thời gian thực qua mạng Wi-Fi/Internet.
* **Giao tiếp Cảm biến & Hiển thị:** Kết nối các dòng cảm biến (tốc độ gió, nhiệt độ, độ ẩm, cảm biến từ...) và xuất dữ liệu ra màn hình OLED/LCD.
* **Xử lý Tín hiệu & Mạng:** Các ứng dụng phát gói tin thô (Beacon/SSID), thử nghiệm an ninh mạng và quản lý kết nối Wi-Fi.

<img width="720" height="490" alt="image" src="https://github.com/user-attachments/assets/5c170359-efac-4747-9002-da8612cdc809" />


### 🛠️ Yêu Cầu Phần Cứng & Phần Mềm

* **Bo mạch hỗ trợ:** ESP8266 (NodeMCU Lua V3, Wemos D1 Mini...) và các dòng ESP32.
* **Môi trường lập trình:** Arduino IDE (cần thêm URL quản lý bo mạch ESP8266/ESP32 trong mục Preferences).
* **Thư viện đi kèm:** Các thư viện tương ứng được ghi chú cụ thể ở đầu từng file code của từng dự án.

<img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/f0430043-02fc-47f5-99dc-d6106978b197" />


### 🚀 Hướng Dẫn Sử Dụng

1. Clone hoặc tải trực tiếp Repository này về máy tính.
2. Truy cập vào thư mục của dự án bạn cần sử dụng.
3. Đọc kỹ file `README.md` hoặc file hướng dẫn đi kèm trong từng thư mục để xem sơ đồ đấu nối chân (Pinout) và cấu hình phần mềm.
4. Mở file `.ino` bằng **Arduino IDE**, chọn đúng bo mạch, cổng COM và tiến hành Upload code.

<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/ec6a04b7-fa02-4e6c-a24b-4bdadef09f1f" />


💻 *Được đóng góp và phát triển bởi **An Nam PC**.*
