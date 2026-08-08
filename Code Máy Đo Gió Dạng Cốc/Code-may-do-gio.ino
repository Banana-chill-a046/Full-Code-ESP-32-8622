#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// --- CẤU HÌNH AP WIFI ---
const char* ap_ssid = "MAY_DO_GIO_AN_NAM";
const char* ap_pass = "12345678";

// --- TÀI KHOẢN ADMIN ---
const char* admin_user = "admin";
const char* admin_pass = "123456";

ESP8266WebServer server(80);

const byte HALL_PIN = 14; // Chân D5 (GPIO14)

// --- CÁC BIẾN CẤU HÌNH EEPROM ---
float radius = 0.10;             // Bán kính (m)
float calibrationFactor = 2.5;   // Hệ số hiệu chuẩn
String selectedUnit = "kmh";     // kmh, ms, knots
String currentTheme = "cyber";   // cyber, matrix, sunset
int timeZone = 7;                // UTC+7 (Việt Nam)
String currentLang = "vi";       // vi, en

volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
unsigned long lastCalcTime = 0;

float rawSpeedMS = 0.0;
float maxSpeedMS = 0.0;
float rpm = 0.0;

// Lịch sử đo để xuất File TXT/DOCS
String logData = "--- LICH SU DO TOC DO GIO - AN NAM WEATHER ---\nTime(s)\tRPM\tToc Do(m/s)\n";
int logCounter = 0;

ICACHE_RAM_ATTR void countPulse() {
  unsigned long currentTime = millis();
  if (currentTime - lastPulseTime > 15) { 
    pulseCount++;
    lastPulseTime = currentTime;
  }
}

// --- XỬ LÝ EEPROM ---
void saveSettings() {
  EEPROM.begin(512);
  EEPROM.put(0, radius);
  EEPROM.put(10, calibrationFactor);
  
  char u[10], t[10], l[10];
  selectedUnit.toCharArray(u, 10);
  currentTheme.toCharArray(t, 10);
  currentLang.toCharArray(l, 10);
  
  EEPROM.put(20, u);
  EEPROM.put(30, t);
  EEPROM.put(40, timeZone);
  EEPROM.put(50, l);
  EEPROM.commit();
  EEPROM.end();
}

void loadSettings() {
  EEPROM.begin(512);
  float tempR, tempC;
  char tempU[10], tempT[10], tempL[10];
  int tempTZ;

  EEPROM.get(0, tempR);
  EEPROM.get(10, tempC);
  EEPROM.get(20, tempU);
  EEPROM.get(30, tempT);
  EEPROM.get(40, tempTZ);
  EEPROM.get(50, tempL);
  EEPROM.end();

  if (!isnan(tempR) && tempR > 0.001) radius = tempR;
  if (!isnan(tempC) && tempC > 0.1) calibrationFactor = tempC;
  if (String(tempU) != "") selectedUnit = String(tempU);
  if (String(tempT) != "") currentTheme = String(tempT);
  if (tempTZ >= -12 && tempTZ <= 14) timeZone = tempTZ;
  if (String(tempL) != "") currentLang = String(tempL);
}

// --- BẢO MẬT LOGIN BẮT BUỘC ---
bool isAuthorized() {
  if (!server.authenticate(admin_user, admin_pass)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

// --- TRANG WEB CHÍNH (HTML + CSS + JS) ---
void handleRoot() {
  if (!isAuthorized()) return; // Bắt đăng nhập ngay khi vào IP

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>An Nam Weather CyberStation Pro</title>
    <style>
        :root {
            --primary: #00f3ff;
            --bg: #030712;
            --panel: #0b1528;
            --text: #f8fafc;
            --accent: #ff0055;
            --border: #1e293b;
        }

        body.theme-matrix {
            --primary: #00ff66;
            --bg: #000d05;
            --panel: #021a0a;
            --text: #e8ffe8;
            --accent: #00ffcc;
            --border: #053b16;
        }

        body.theme-sunset {
            --primary: #ff9900;
            --bg: #1a0505;
            --panel: #2b0b0b;
            --text: #fff0f0;
            --accent: #ff0055;
            --border: #4a1515;
        }

        * { box-sizing: border-box; font-family: 'Segoe UI', Tahoma, sans-serif; transition: all 0.3s ease; }
        body { background-color: var(--bg); color: var(--text); margin: 0; padding: 0; overflow-x: hidden; }

        /* HEADER & HAMBURGER MENU */
        header { display: flex; justify-content: space-between; align-items: center; padding: 15px 20px; background: var(--panel); border-bottom: 2px solid var(--primary); box-shadow: 0 0 15px var(--primary); }
        .logo { font-weight: 900; font-size: 18px; color: var(--primary); letter-spacing: 2px; text-shadow: 0 0 8px var(--primary); }
        .menu-btn { font-size: 24px; cursor: pointer; color: var(--primary); background: none; border: none; }

        /* SIDEBAR SLIDE-OUT */
        .sidebar { position: fixed; top: 0; right: -320px; width: 300px; height: 100%; background: var(--panel); border-left: 2px solid var(--primary); z-index: 1000; padding: 20px; box-shadow: -10px 0 30px rgba(0,0,0,0.8); overflow-y: auto; }
        .sidebar.active { right: 0; }
        .close-btn { text-align: right; font-size: 24px; color: var(--accent); cursor: pointer; margin-bottom: 20px; }
        .menu-item { display: block; width: 100%; padding: 12px; margin: 8px 0; background: rgba(255,255,255,0.05); border: 1px solid var(--border); color: var(--text); border-radius: 8px; text-align: left; cursor: pointer; font-weight: bold; }
        .menu-item:hover { border-color: var(--primary); background: rgba(0,243,255,0.1); }

        /* MAIN DASHBOARD */
        .container { max-width: 500px; margin: 20px auto; padding: 0 15px; text-align: center; }
        .gauge-box { position: relative; width: 220px; height: 220px; margin: 10px auto; }
        .gauge-canvas { width: 100%; height: 100%; }
        .fan-icon { width: 60px; height: 60px; border: 4px dashed var(--primary); border-radius: 50%; margin: 10px auto; animation: spin 2s linear infinite; }
        @keyframes spin { 100% { transform: rotate(360deg); } }

        .speed-display { font-size: 64px; font-weight: 900; color: var(--primary); text-shadow: 0 0 20px var(--primary); line-height: 1; }
        .unit-display { font-size: 18px; color: #94a3b8; text-transform: uppercase; margin-bottom: 15px; }
        .status-badge { background: rgba(0,243,255,0.1); border: 1px solid var(--primary); padding: 8px 16px; border-radius: 20px; display: inline-block; font-size: 14px; font-weight: bold; margin-bottom: 20px; }

        .stats-grid { display: flex; gap: 10px; margin-bottom: 20px; }
        .stat-card { flex: 1; background: var(--panel); border: 1px solid var(--border); padding: 15px; border-radius: 12px; }
        .stat-title { font-size: 11px; color: #64748b; text-transform: uppercase; }
        .stat-val { font-size: 20px; font-weight: bold; color: var(--accent); margin-top: 5px; }

        /* MODAL POPUPS */
        .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.85); z-index: 2000; align-items: center; justify-content: center; }
        .modal-content { background: var(--panel); border: 2px solid var(--primary); width: 90%; max-width: 420px; padding: 25px; border-radius: 16px; text-align: left; max-height: 85vh; overflow-y: auto; }
        .modal-header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border); padding-bottom: 10px; margin-bottom: 15px; }
        .modal-title { font-size: 18px; font-weight: bold; color: var(--primary); }

        label { display: block; font-size: 12px; margin-top: 12px; color: #94a3b8; }
        input, select { width: 100%; padding: 10px; margin-top: 4px; background: var(--bg); border: 1px solid var(--border); color: var(--text); border-radius: 8px; }
        .btn-submit { width: 100%; padding: 12px; background: var(--primary); color: #000; font-weight: bold; border: none; border-radius: 8px; margin-top: 20px; cursor: pointer; }
    </style>
</head>
<body class="theme-)rawliteral" + currentTheme + R"rawliteral(">

    <!-- HEADER -->
    <header>
        <div class="logo">AN NAM CYBERSTATION</div>
        <button class="menu-btn" onclick="toggleSidebar()">&#9776;</button>
    </header>

    <!-- SIDEBAR SLIDE-OUT (MENU 3 DẤU GẠCH) -->
    <div class="sidebar" id="sidebar">
        <div class="close-btn" onclick="toggleSidebar()">&times;</div>
        <button class="menu-item" onclick="openModal('modal-settings')">&#9881;&#65039; <span id="lbl-settings">Cài Đặt Hệ Thống</span></button>
        <button class="menu-item" onclick="openModal('modal-export')">&#128190; <span id="lbl-export">Xuất File Báo Cáo</span></button>
        <button class="menu-item" onclick="openModal('modal-lang')">&#127760; <span id="lbl-lang">Đổi Ngôn Ngữ</span></button>
        <button class="menu-item" onclick="openModal('modal-about')">&#8505;&#65039; <span id="lbl-about">About Me</span></button>
    </div>

    <!-- MAIN DASHBOARD -->
    <div class="container">
        <div class="fan-icon" id="fan"></div>
        <div class="speed-display" id="speed">0.0</div>
        <div class="unit-display" id="unit">km/h</div>

        <div class="status-badge" id="beaufort">Đang kết nối hệ thống...</div>

        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-title" id="lbl-max">Gió Đỉnh (Max)</div>
                <div class="stat-val" id="max-speed">0.0</div>
            </div>
            <div class="stat-card">
                <div class="stat-title" id="lbl-rpm">Vòng Quay (RPM)</div>
                <div class="stat-val" id="rpm">0</div>
            </div>
        </div>
    </div>

    <!-- MODAL 1: CÀI ĐẶT HỆ THỐNG -->
    <div class="modal" id="modal-settings">
        <div class="modal-content">
            <div class="modal-header">
                <div class="modal-title">&#9881;&#65039; Cài Đặt Thông Số</div>
                <div class="close-btn" onclick="closeModal('modal-settings')">&times;</div>
            </div>
            <form action="/save-settings" method="POST">
                <label>Giao Diện (Theme):</label>
                <select name="theme">
                    <option value="cyber" )rawliteral" + String(currentTheme == "cyber" ? "selected" : "") + R"rawliteral(>Cyberpunk Neon (Blue)</option>
                    <option value="matrix" )rawliteral" + String(currentTheme == "matrix" ? "selected" : "") + R"rawliteral(>Matrix Hacker (Green)</option>
                    <option value="sunset" )rawliteral" + String(currentTheme == "sunset" ? "selected" : "") + R"rawliteral(>Sunset Orange (Red/Orange)</option>
                </select>

                <label>Múi Giờ (Timezone):</label>
                <select name="timezone">
                    <option value="7" )rawliteral" + String(timeZone == 7 ? "selected" : "") + R"rawliteral(>UTC+07:00 (Việt Nam)</option>
                    <option value="0" )rawliteral" + String(timeZone == 0 ? "selected" : "") + R"rawliteral(>UTC+00:00 (GMT)</option>
                </select>

                <label>Đơn Vị Đo Gió:</label>
                <select name="unit">
                    <option value="kmh" )rawliteral" + String(selectedUnit == "kmh" ? "selected" : "") + R"rawliteral(>km/h (Kilômét / Giờ)</option>
                    <option value="ms" )rawliteral" + String(selectedUnit == "ms" ? "selected" : "") + R"rawliteral(>m/s (Mét / Giây)</option>
                    <option value="knots" )rawliteral" + String(selectedUnit == "knots" ? "selected" : "") + R"rawliteral(>Knots (Hải Lý / Giờ)</option>
                </select>

                <label>Đường Kính Cánh Quạt (Mét):</label>
                <input type="number" step="0.001" name="diameter" value=")rawliteral" + String(radius * 2.0, 3) + R"rawliteral(">

                <label>Hệ Số Hiệu Chuẩn Calibration:</label>
                <input type="number" step="0.1" name="cal" value=")rawliteral" + String(calibrationFactor, 1) + R"rawliteral(">

                <button type="submit" class="btn-submit">Lưu Cấu Hình</button>
            </form>
        </div>
    </div>

    <!-- MODAL 2: XUẤT FILE BÁO CÁO -->
    <div class="modal" id="modal-export">
        <div class="modal-content">
            <div class="modal-header">
                <div class="modal-title">&#128190; Xuất Dữ Liệu Báo Cáo</div>
                <div class="close-btn" onclick="closeModal('modal-export')">&times;</div>
            </div>
            <p style="font-size: 13px; color: #94a3b8;">Tải log lịch sử đo gió từ chip ESP8266 về thiết bị:</p>
            <button class="btn-submit" onclick="location.href='/download-txt'">&#128196; Tải File TXT (Cho Máy Tính PC)</button>
            <button class="btn-submit" style="background: #2563eb; color: #fff; margin-top: 10px;" onclick="location.href='/download-docs'">&#128205; Tải File DOCS (Cho Điện Thoại)</button>
        </div>
    </div>

    <!-- MODAL 3: ĐỔI NGÔN NGỮ -->
    <div class="modal" id="modal-lang">
        <div class="modal-content">
            <div class="modal-header">
                <div class="modal-title">&#127760; Chọn Ngôn Ngữ / Language</div>
                <div class="close-btn" onclick="closeModal('modal-lang')">&times;</div>
            </div>
            <form action="/save-lang" method="POST">
                <select name="lang">
                    <option value="vi" )rawliteral" + String(currentLang == "vi" ? "selected" : "") + R"rawliteral(>Tiếng Việt</option>
                    <option value="en" )rawliteral" + String(currentLang == "en" ? "selected" : "") + R"rawliteral(>English</option>
                </select>
                <button type="submit" class="btn-submit">Chuyển Đổi</button>
            </form>
        </div>
    </div>

    <!-- MODAL 4: ABOUT ME -->
    <div class="modal" id="modal-about">
        <div class="modal-content">
            <div class="modal-header">
                <div class="modal-title">&#8505;&#65039; Thông Tin Dự Án</div>
                <div class="close-btn" onclick="closeModal('modal-about')">&times;</div>
            </div>
            <div style="font-size: 13px; line-height: 1.6; color: #cbd5e1;">
                <h3 style="color: var(--primary); margin-top:0;">AN NAM PC AI STATION</h3>
                <p><b>Thiết bị:</b> ESP8266 NodeMCU Lua V3 (ESP-12E Type-C)</p>
                <p><b>Cảm biến:</b> Hall A3144 (Digital Pulse Interrupt)</p>
                <p><b>Tính năng:</b> WebServer Realtime, WebSockets/JSON Update, EEPROM Flash Saver, Auto Beaufort Warning, Dynamic Multi-Theme Interface.</p>
                <hr style="border-color: var(--border);">
                <p style="text-align: center; color: var(--primary); font-weight: bold;">Được thiết kế & tối ưu tối đa bởi An Nam PC AI!</p>
            </div>
        </div>
    </div>

    <!-- JAVASCRIPT XỬ LÝ GIAO DIỆN -->
    <script>
        function toggleSidebar() {
            document.getElementById('sidebar').classList.toggle('active');
        }

        function openModal(id) {
            document.getElementById(id).style.display = 'flex';
            toggleSidebar();
        }

        function closeModal(id) {
            document.getElementById(id).style.display = 'none';
        }

        // F5 số liệu liên tục bằng AJAX JSON
        setInterval(function() {
            fetch('/data').then(r => r.json()).then(d => {
                document.getElementById('speed').innerText = d.speed;
                document.getElementById('unit').innerText = d.unit;
                document.getElementById('max-speed').innerText = d.max + " " + d.unit;
                document.getElementById('rpm').innerText = d.rpm;
                document.getElementById('beaufort').innerText = d.beaufort;

                let fan = document.getElementById('fan');
                if (d.rpm > 0) {
                    let speedSec = 60 / d.rpm;
                    fan.style.animationDuration = Math.max(0.08, speedSec) + "s";
                } else {
                    fan.style.animationDuration = "0s";
                }
            });
        }, 1000);
    </script>
</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Endpoint cấp dữ liệu JSON
void handleData() {
  float displaySpeed = rawSpeedMS * 3.6;
  float displayMax = maxSpeedMS * 3.6;
  String unitLabel = "km/h";

  if (selectedUnit == "ms") {
    displaySpeed = rawSpeedMS;
    displayMax = maxSpeedMS;
    unitLabel = "m/s";
  } else if (selectedUnit == "knots") {
    displaySpeed = rawSpeedMS * 1.94384;
    displayMax = maxSpeedMS * 1.94384;
    unitLabel = "Knots";
  }

  String beaufort = "Cấp 0 (Lặng gió)";
  if (rawSpeedMS >= 0.3 && rawSpeedMS < 1.6) beaufort = "Cấp 1 (Gió nhẹ)";
  else if (rawSpeedMS >= 1.6 && rawSpeedMS < 3.4) beaufort = "Cấp 2 (Gió nhẹ)";
  else if (rawSpeedMS >= 3.4 && rawSpeedMS < 5.5) beaufort = "Cấp 3 (Gió yếu)";
  else if (rawSpeedMS >= 5.5 && rawSpeedMS < 8.0) beaufort = "Cấp 4 (Gió vừa)";
  else if (rawSpeedMS >= 8.0 && rawSpeedMS < 10.8) beaufort = "Cấp 5 (Gió khá mạnh)";
  else if (rawSpeedMS >= 10.8 && rawSpeedMS < 13.9) beaufort = "Cấp 6 (Gió mạnh)";
  else if (rawSpeedMS >= 13.9) beaufort = "Cấp 7+ (Gió giật / Bão mạnh)";

  if (currentLang == "en") {
    if (rawSpeedMS < 0.3) beaufort = "Calm (Level 0)";
    else if (rawSpeedMS < 5.5) beaufort = "Light Breeze";
    else if (rawSpeedMS < 10.8) beaufort = "Moderate Breeze";
    else beaufort = "Strong Wind / Gale Warning!";
  }

  String json = "{";
  json += "\"speed\":\"" + String(displaySpeed, 1) + "\",";
  json += "\"max\":\"" + String(displayMax, 1) + "\",";
  json += "\"unit\":\"" + unitLabel + "\",";
  json += "\"rpm\":" + String((int)rpm) + ",";
  json += "\"beaufort\":\"" + beaufort + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// Xử lý lưu Cài Đặt (Theme, Timezone, Đơn vị, Đường kính)
void handleSaveSettings() {
  if (!isAuthorized()) return;

  if (server.hasArg("theme")) currentTheme = server.arg("theme");
  if (server.hasArg("timezone")) timeZone = server.arg("timezone").toInt();
  if (server.hasArg("unit")) selectedUnit = server.arg("unit");
  if (server.hasArg("diameter")) radius = server.arg("diameter").toFloat() / 2.0; // Nhập đường kính chia 2 ra bán kính
  if (server.hasArg("cal")) calibrationFactor = server.arg("cal").toFloat();

  saveSettings();

  server.sendHeader("Location", "/");
  server.send(303);
}

// Xử lý đổi ngôn ngữ
void handleSaveLang() {
  if (!isAuthorized()) return;
  if (server.hasArg("lang")) currentLang = server.arg("lang");
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(303);
}

// Xuất file TXT cho PC
void handleDownloadTXT() {
  if (!isAuthorized()) return;
  server.sendHeader("Content-Disposition", "attachment; filename=wind_log_pc.txt");
  server.send(200, "text/plain", logData);
}

// Xuất file DOCS (mô phỏng file Word/Docs) cho Điện thoại
void handleDownloadDOCS() {
  if (!isAuthorized()) return;
  server.sendHeader("Content-Disposition", "attachment; filename=wind_log_mobile.doc");
  server.send(200, "application/msword", logData);
}

void setup() {
  Serial.begin(115200);
  loadSettings();

  pinMode(HALL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), countPulse, FALLING);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/save-settings", HTTP_POST, handleSaveSettings);
  server.on("/save-lang", HTTP_POST, handleSaveLang);
  server.on("/download-txt", handleDownloadTXT);
  server.on("/download-docs", handleDownloadDOCS);
  server.begin();
}

void loop() {
  server.handleClient();

  unsigned long currentTime = millis();
  if (currentTime - lastCalcTime >= 1000) {
    detachInterrupt(digitalPinToInterrupt(HALL_PIN));

    rpm = (pulseCount * 60.0) / ((currentTime - lastCalcTime) / 1000.0);
    float distancePerRev = 2 * 3.14159 * radius;
    rawSpeedMS = (rpm / 60.0) * distancePerRev * calibrationFactor;

    if (rawSpeedMS > maxSpeedMS) maxSpeedMS = rawSpeedMS;

    // Ghi log dữ liệu
    logCounter++;
    if (logCounter % 5 == 0 && rawSpeedMS > 0) { // Cứ 5s ghi 1 dòng log nếu có gió
      logData += String(logCounter) + "s\t\t" + String((int)rpm) + "\t\t" + String(rawSpeedMS, 2) + " m/s\n";
    }

    pulseCount = 0;
    lastCalcTime = currentTime;
    attachInterrupt(digitalPinToInterrupt(HALL_PIN), countPulse, FALLING);
  }
}
