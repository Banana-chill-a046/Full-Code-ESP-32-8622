# 📡 Hướng Dẫn Nạp Code Wi-Fi Beacon Spam (50 SSID Fake Tiếng Việt Có Dấu) Cho ESP8266
# Từ Ngữ Rất Láo Hãy Cận Trọng
Dự án phát hàng loạt 50 tên Wi-Fi giả (Mạng mở / Không mật khẩu) với tên tiếng Việt có dấu gắt gỏng, cà khịa cực độ bằng mạch **ESP8266 (NodeMCU V3)**.

---

### 🛠️ Thiết Bị Cần Chuẩn Bị

* **1x** Bo mạch ESP8266 (NodeMCU V3 Lua / Wemos D1 Mini / ESP-12E)
* **1x** Cáp sạc/truyền dữ liệu USB (Micro-USB hoặc Type-C tùy bản mạch)
* Máy tính đã cài đặt **Arduino IDE**

---

### 💻 Code Arduino Cực Chuẩn (Copy & Paste vào Arduino IDE)

```cpp
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

// Danh sách 50 SSID Tiếng Việt CÓ DẤU cực gắt (Mạng Mở / Không mật khẩu)
const char* fakeSSIDs[50] = {
  "Xô Vào Đây Mà Ăn", "Đóng Tiền Mạng Đi Cụ", "Mạng Nhà Mày Đéo Có À", "Bật Wi-Fi Làm Đéo Gì", "Kết Nối Cái Buồi Ấy",
  "Nhà Nghèo Xài Wi-Fi Chùa", "Xuýt Nữa Thì Ăn Được", "Nhìn Cái Lồn Gì", "Ảo Tưởng Ăn Chùa", "Mua Gói 4G Đi Cụ",
  "Thích Kết Nối Không", "Mạng Cày Nhà Lá Vườn", "Đồ Cho Tụi Lười", "Chờ Kết Nối Còn Lâu", "Chuyển Tiền Mạng Cho Tớ",
  "Wi-Fi Ngủ Hắc Ra Nữa", "Kéo Mãi Không Xong À", "Đéo Có Pass Đâu Con", "Nói Giỏi Thì Ẩn Đi", "Đừng Click Tùm Lum",
  "Máy Tính Của Ngu", "Xin Pass Hỏi Ông Nội", "Wi-Fi Free Cho Chó", "Ngân Hàng Vỡ Nợ", "Trật Mắt Với Mạng",
  "Cay Chưa Cụ Em", "Tập Gym Cho Khỏe Bố", "Đừng Hòng Ăn Free", "Xài Chùa Đi Chỗ Khác", "Đéo Cho Log In",
  "Mạng Cùi Ốc Bươu", "Cứ Bấm Vào Mà Chờ", "Đã Xin Các Cụ Chưa", "Wi-Fi Dành Cho Bò", "Thích Săn Bao A-F-K",
  "Kết Nối Con Cặc", "Đừng Thấp Thỏm Nữa", "Xem Phim Sếch Ít Thôi", "Đi Làm Kiếm Tiền Đi", "Đồ Ăn Hại Chưa",
  "Log Vào Là Ăn Đạn", "Mạng Nhà VIP Pro", "Đéo Phải Cho Bố", "Bật 4G Lên Bu", "Wi-Fi Cho Mày Cúp",
  "Thích Cày View À", "Ăn Lồn Đi Thằng Cùi", "Ở Nhà Cùi Đốc", "Chưa Chơi Đồ Chơi À", "Vô Cơ Giới Mà Đít"
};

// Cấu trúc khung gói tin Beacon (802.11 Beacon Frame Header)
uint8_t packet[128] = {
  0x80, 0x00,                         // Type: Beacon
  0x00, 0x00,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (Sẽ tạo ngẫu nhiên)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID (Sẽ tạo ngẫu nhiên)
  0x00, 0x00,                         // Sequence control
  // Timestamp
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00,                         // Beacon interval (100ms)
  0x01, 0x04                          // Capability info (Open System / Mạng mở không mật khẩu)
};

void setup() {
  // Đưa ESP8266 về chế độ Promiscuous phát gói tin thô
  WiFi.mode(WIFI_OFF);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(1);
}

void loop() {
  for (int i = 0; i < 50; i++) {
    // 1. Tạo Địa chỉ MAC ngẫu nhiên cho từng SSID
    for (int mac_idx = 10; mac_idx < 22; mac_idx++) {
      packet[mac_idx] = random(0, 255);
    }
    packet[10] = 0x02; // Byte hợp lệ cho thiết bị phát

    // 2. Chèn SSID Tiếng Việt vào gói tin
    const char* ssid = fakeSSIDs[i];
    int ssidLen = strlen(ssid);
    if (ssidLen > 32) ssidLen = 32;

    packet[36] = 0x00;              // Tag Number: SSID
    packet[37] = ssidLen;           // Tag Length
    
    for (int j = 0; j < ssidLen; j++) {
      packet[38 + j] = ssid[j];
    }

    // 3. Cấu hình tốc độ và Kênh phát (Channel)
    int packetLen = 38 + ssidLen;
    
    // Tag Supported Rates
    packet[packetLen] = 0x01;
    packet[packetLen + 1] = 0x08;
    packet[packetLen + 2] = 0x82; packet[packetLen + 3] = 0x84;
    packet[packetLen + 4] = 0x8b; packet[packetLen + 5] = 0x96;
    packet[packetLen + 6] = 0x24; packet[packetLen + 7] = 0x30;
    packet[packetLen + 8] = 0x48; packet[packetLen + 9] = 0x6c;
    packetLen += 10;

    // Đảo kênh liên tục từ Kênh 1 đến 11
    uint8_t channel = (i % 11) + 1;
    wifi_set_channel(channel);
    packet[packetLen] = 0x03;
    packet[packetLen + 1] = 0x01;
    packet[packetLen + 2] = channel;
    packetLen += 3;

    // 4. Bắn gói tin ra không gian
    wifi_send_pkt_freedom(packet, packetLen, 0);
    
    delay(2); // Tránh nghẽn vi điều khiển
  }
}
