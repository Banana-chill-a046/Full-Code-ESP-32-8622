# 🌀 Hướng Dẫn Tự Làm Máy Đo Tốc Độ Gió Thông Minh Với ESP8266 & WebServer

Dự án tự làm thiết bị đo tốc độ gió thời gian thực, quản lý và hiển thị qua giao diện Web Cyberpunk cực đẹp trên điện thoại/PC, không lo đụng hàng!

---

### 💸 Chi phí chuẩn bị (Khoảng 140.000 VNĐ)

* **1x** Mạch ESP8266 NodeMCU Lua V3 CH340 (Bản cổng Type-C)
* **1x** Màn hình OLED I2C 0.96 inch (Tùy chọn nếu thích hiển thị tại chỗ)
* **1x** Module cảm biến từ Hall A3144
* Dây điện, mỏ hàn, thiếc hàn, 1 con điện trở 10kΩ (để kéo trở cho cảm biến)

---

### 🛠️ Bước 1: Sơ đồ hàn mạch

**1. Hàn màn hình OLED I2C (Nếu sử dụng):**
* **GND** -> Hàn vào chân GND của NodeMCU
* **VCC** -> Hàn vào chân 3.3V của NodeMCU
* **SCL** -> Hàn vào chân D1 (GPIO5)
* **SDA** -> Hàn vào chân D2 (GPIO4)

**2. Hàn cảm biến Hall A3144 (Nhìn từ mặt dẹt có chữ):**
* **Chân 1 (Bên trái):** Hàn dây VCC -> Nối vào chân 3.3V (hoặc 5V) của NodeMCU
* **Chân 2 (Ở giữa):** Hàn dây GND -> Nối vào chân GND của NodeMCU
* **Chân 3 (Bên phải):** Hàn dây SIGNAL -> Nối vào chân D5 (GPIO14)

> **Mẹo nhỏ:** Nên hàn kẹp 1 con điện trở 10kΩ giữa Chân 1 (VCC) và Chân 3 (SIGNAL) để chống nhiễu tín hiệu cực tốt.

---

### 💻 Bước 2: Cài đặt & Nạp Code

1. Tải và cài đặt phần mềm **Arduino IDE** bản mới nhất trên máy tính.
2. Thêm bo mạch ESP8266 vào Arduino IDE:
   * Vào `File` -> `Preferences` -> Dán đường dẫn này vào ô *Additional Boards Manager URLs*:
     ```text
     [http://arduino.esp8266.com/stable/package_esp8266com_index.json](http://arduino.esp8266.com/stable/package_esp8266com_index.json)
     ```
   * Vào `Tools` -> `Board` -> `Boards Manager...`, tìm từ khóa **ESP8266** và bấm **Install**.
3. Cắm cáp Type-C kết nối ESP8266 với máy tính.
4. Chọn đúng bo mạch: `Tools` -> `Board` -> `ESP8266 Boards` -> **NodeMCU 1.0 (ESP-12E Module)** và chọn đúng cổng `COM` tương ứng.
5. Mở file code dự án trong repository này và bấm nút **Upload (➔)** để nạp code vào mạch.

---

### ⚙️ Bước 3: Lắp đặt & Đi vào sử dụng

* **Vị trí đặt mạch:** Hãy chọn vị trí cao, thoáng gió nhất.
* **Bảo vệ mạch:** Cực kỳ quan trọng: **Làm ơn đừng để mạch bị ướt mưa!** Bạn nên đầu tư 1 hộp nhựa/mica chống nước để bảo vệ bo mạch.
* **Cấp nguồn:** Có thể xài nguồn pin năng lượng mặt trời, cắm sạc dự phòng, hoặc kéo dây sạc Type-C từ trong nhà ra.
* **Kết nối:** Bật Wi-Fi điện thoại/PC, bắt mạng Wi-Fi do mạch phát ra (`MAY_DO_GIO_AN_NAM`), mở trình duyệt gõ `192.168.4.1` để vào hệ thống điều khiển Cyberstation.
