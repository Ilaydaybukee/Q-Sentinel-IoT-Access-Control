#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "config.h"

// -------------------- PIN DEFINITIONS --------------------
#define SS_PIN D2
#define RST_PIN D1
#define SERVO_PIN D4
#define IR_RECV_PIN D3

// -------------------- WEB SERVER --------------------
ESP8266WebServer server(80);

// -------------------- MODULE OBJECTS --------------------
MFRC522 rfid(SS_PIN, RST_PIN);
Servo lockServo;

IRrecv irrecv(IR_RECV_PIN);
decode_results irResults;

// -------------------- AUTH CONFIG --------------------
byte authorizedUID[] = {0x45, 0xB6, 0x0A, 0x01};
byte authorizedUIDSize = 4;

const uint64_t AUTHORIZED_IR_CODE = QS_AUTHORIZED_IR_CODE;
const unsigned long IR_VALID_WINDOW_MS = QS_IR_VALID_WINDOW_MS;
const String DASHBOARD_PIN = QS_DASHBOARD_PIN;

// -------------------- SERVO CONFIG --------------------
const int LOCK_POSITION = 0;
const int UNLOCK_POSITION = 90;
const unsigned long UNLOCK_DURATION_MS = 2000;

// -------------------- SYSTEM STATES --------------------
String lastUID = "No card scanned yet";
String accessStatus = "WAITING";
String lockStatus = "LOCKED";
String lastEvent = "System started. Waiting for RFID card.";
String securityMode = "NORMAL";

String irChannelStatus = "WAITING";
String lastIRProtocol = "None";
String lastIRCode = "None";
String irKeyStatus = "NOT VALIDATED";

unsigned long scanCount = 0;
unsigned long irSignalCount = 0;
unsigned long lastValidIRTime = 0;

bool unlockActive = false;
unsigned long unlockEndTime = 0;

String accessLog[8] = {
  "No access record yet.",
  "",
  "",
  "",
  "",
  "",
  "",
  ""
};

// -------------------- HTML PAGE --------------------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Q-Sentinel V4 Pro</title>

  <style>
    :root {
      --bg1: #07111c;
      --bg2: #0d2635;
      --card: rgba(21, 37, 52, 0.82);
      --card2: rgba(31, 54, 74, 0.92);
      --cyan: #00e0c6;
      --green: #39ff88;
      --red: #ff4f5e;
      --yellow: #ffd166;
      --purple: #c084fc;
      --blue: #75c7ff;
      --text: #f3f7fb;
      --muted: #9fb5c8;
      --border: rgba(255, 255, 255, 0.10);
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      min-height: 100vh;
      font-family: Arial, Helvetica, sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at 20% 10%, rgba(0, 224, 198, 0.16), transparent 34%),
        radial-gradient(circle at 85% 12%, rgba(192, 132, 252, 0.15), transparent 32%),
        linear-gradient(145deg, var(--bg1), var(--bg2));
      overflow-x: hidden;
    }

    .background-grid {
      position: fixed;
      inset: 0;
      background-image:
        linear-gradient(rgba(255,255,255,0.03) 1px, transparent 1px),
        linear-gradient(90deg, rgba(255,255,255,0.03) 1px, transparent 1px);
      background-size: 34px 34px;
      mask-image: radial-gradient(circle, black, transparent 78%);
      pointer-events: none;
    }

    .wrap {
      position: relative;
      max-width: 1180px;
      margin: 0 auto;
      padding: 28px;
    }

    .hero {
      display: flex;
      justify-content: space-between;
      gap: 20px;
      align-items: center;
      margin-bottom: 20px;
      padding: 24px;
      border: 1px solid var(--border);
      border-radius: 28px;
      background: rgba(11, 25, 38, 0.62);
      box-shadow: 0 24px 70px rgba(0,0,0,0.28);
      backdrop-filter: blur(10px);
    }

    .title-block { text-align: left; }

    h1 {
      margin: 0;
      font-size: 54px;
      line-height: 1;
      color: var(--cyan);
      letter-spacing: -1.5px;
      text-shadow: 0 0 20px rgba(0, 224, 198, 0.35);
    }

    .subtitle {
      margin-top: 10px;
      color: var(--muted);
      font-size: 17px;
    }

    .live-pill {
      display: inline-flex;
      align-items: center;
      gap: 9px;
      padding: 12px 16px;
      border-radius: 999px;
      background: rgba(0, 224, 198, 0.10);
      border: 1px solid rgba(0, 224, 198, 0.25);
      color: var(--cyan);
      font-weight: 800;
    }

    .pulse {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: var(--cyan);
      box-shadow: 0 0 0 rgba(0,224,198,.7);
      animation: pulse 1.6s infinite;
    }

    @keyframes pulse {
      0% { box-shadow: 0 0 0 0 rgba(0,224,198,.72); }
      70% { box-shadow: 0 0 0 14px rgba(0,224,198,0); }
      100% { box-shadow: 0 0 0 0 rgba(0,224,198,0); }
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 16px;
    }

    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 22px;
      padding: 18px;
      min-height: 124px;
      box-shadow: 0 18px 50px rgba(0,0,0,0.22);
      backdrop-filter: blur(10px);
      transition: transform 0.2s ease, border-color 0.2s ease;
    }

    .card:hover {
      transform: translateY(-3px);
      border-color: rgba(0, 224, 198, 0.28);
    }

    .wide { grid-column: span 2; }

    .label {
      color: var(--muted);
      font-size: 13px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    .value {
      margin-top: 12px;
      font-size: 24px;
      font-weight: 900;
      word-break: break-word;
    }

    .small-value { font-size: 18px; }

    .state-granted { color: var(--green); }
    .state-denied { color: var(--red); }
    .state-waiting { color: var(--yellow); }
    .state-secure { color: var(--purple); }
    .state-normal { color: var(--blue); }

    .status-orb {
      width: 84px;
      height: 84px;
      margin: 8px auto 0;
      border-radius: 50%;
      display: grid;
      place-items: center;
      background: radial-gradient(circle, rgba(0,224,198,.42), rgba(0,224,198,.06));
      border: 1px solid rgba(0,224,198,.25);
      box-shadow: 0 0 28px rgba(0,224,198,.22);
      font-size: 28px;
    }

    .event-panel {
      margin-top: 18px;
      padding: 20px;
      border-radius: 22px;
      background: rgba(9, 22, 34, 0.78);
      border: 1px solid var(--border);
      text-align: left;
    }

    .event-title {
      color: var(--muted);
      font-size: 13px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      margin-bottom: 8px;
    }

    .event-text {
      font-size: 18px;
      font-weight: 700;
    }

    .controls {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 12px;
      margin: 22px 0;
    }

    button, a.button {
      border: none;
      cursor: pointer;
      text-decoration: none;
      border-radius: 999px;
      padding: 13px 18px;
      font-weight: 900;
      color: #06151f;
      background: var(--cyan);
      transition: transform 0.18s ease, filter 0.18s ease;
    }

    button:hover, a.button:hover {
      transform: translateY(-2px);
      filter: brightness(1.05);
    }

    .btn-blue { background: var(--blue); }
    .btn-yellow { background: var(--yellow); }
    .btn-red { background: var(--red); color: white; }
    .btn-purple { background: var(--purple); color: white; }

    .log-panel {
      margin-top: 18px;
      padding: 20px;
      border-radius: 22px;
      background: var(--card2);
      border: 1px solid var(--border);
      text-align: left;
    }

    .log-panel h2 {
      margin: 0 0 12px;
      color: var(--cyan);
    }

    .log-list {
      list-style: none;
      padding: 0;
      margin: 0;
      display: grid;
      gap: 8px;
    }

    .log-list li {
      padding: 12px 14px;
      border-radius: 14px;
      background: rgba(255,255,255,0.05);
      color: #dce8f3;
      border: 1px solid rgba(255,255,255,0.06);
      font-family: Consolas, monospace;
      font-size: 14px;
    }

    .progress {
      height: 10px;
      margin-top: 14px;
      background: rgba(255,255,255,0.08);
      border-radius: 999px;
      overflow: hidden;
    }

    .bar {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, var(--cyan), var(--green));
      border-radius: 999px;
      transition: width 0.4s ease;
    }

    .footer {
      margin: 22px 0 8px;
      color: var(--muted);
      font-size: 13px;
      text-align: center;
    }

    @media (max-width: 980px) {
      .grid { grid-template-columns: repeat(2, 1fr); }
      .hero { flex-direction: column; align-items: flex-start; }
      h1 { font-size: 44px; }
    }

    @media (max-width: 620px) {
      .wrap { padding: 16px; }
      .grid { grid-template-columns: 1fr; }
      .wide { grid-column: span 1; }
      h1 { font-size: 38px; }
    }
  </style>
</head>

<body>
  <div class="background-grid"></div>

  <div class="wrap">
    <section class="hero">
      <div class="title-block">
        <h1>Q-Sentinel</h1>
        <div class="subtitle">RFID IoT Access Control + IR Optical Key Validation</div>
      </div>
      <div class="live-pill"><span class="pulse"></span>LIVE DASHBOARD</div>
    </section>

    <section class="grid">
      <div class="card">
        <div class="label">Access Status</div>
        <div id="accessStatus" class="value state-waiting">WAITING</div>
        <div id="accessOrb" class="status-orb">◇</div>
      </div>

      <div class="card"><div class="label">Lock Status</div><div id="lockStatus" class="value">LOCKED</div></div>
      <div class="card"><div class="label">Security Mode</div><div id="securityMode" class="value state-normal">NORMAL</div></div>

      <div class="card">
        <div class="label">IR Key Status</div>
        <div id="irKeyStatus" class="value state-waiting">NOT VALIDATED</div>
        <div class="progress"><div id="irBar" class="bar"></div></div>
        <div id="irRemaining" class="label" style="margin-top:10px;">0 s remaining</div>
      </div>

      <div class="card wide"><div class="label">Last RFID UID</div><div id="lastUID" class="value">No card scanned yet</div></div>
      <div class="card wide"><div class="label">Last IR Code</div><div id="lastIRCode" class="value small-value">None</div></div>
      <div class="card"><div class="label">IR Protocol</div><div id="lastIRProtocol" class="value">None</div></div>
      <div class="card"><div class="label">IR Channel</div><div id="irChannel" class="value small-value">WAITING</div></div>
      <div class="card"><div class="label">Wi-Fi RSSI</div><div id="wifiRssi" class="value">0 dBm</div><div id="wifiQuality" class="label" style="margin-top:10px;">Unknown</div></div>
      <div class="card"><div class="label">IP Address</div><div id="ipAddress" class="value small-value">0.0.0.0</div></div>
      <div class="card"><div class="label">RFID Scans</div><div id="scanCount" class="value">0</div></div>
      <div class="card"><div class="label">IR Signals</div><div id="irSignalCount" class="value">0</div></div>
      <div class="card"><div class="label">Uptime</div><div id="uptime" class="value">00:00:00</div></div>
      <div class="card"><div class="label">Authorized RFID</div><div class="value small-value">45 B6 0A 01</div></div>
    </section>

    <section class="event-panel"><div class="event-title">Last Event</div><div id="lastEvent" class="event-text">System started.</div></section>

    <section class="controls">
      <button class="btn-blue" onclick="setMode('normal')">Normal Mode</button>
      <button class="btn-purple" onclick="setMode('secure')">Secure Mode</button>
      <button onclick="manualUnlock()">Manual Unlock</button>
      <button class="btn-red" onclick="resetLogs()">Reset Logs</button>
      <a class="button btn-yellow" href="/status" target="_blank">JSON API</a>
    </section>

    <section class="log-panel"><h2>Recent Access Log</h2><ul id="logs" class="log-list"><li>No access record yet.</li></ul></section>
    <div class="footer">Q-Sentinel V4 Pro | Live JSON polling every 1 second | Secure Mode requires RFID + valid IR key</div>
  </div>

  <script>
    function setText(id, value) { const el = document.getElementById(id); if (el) el.textContent = value; }
    function stateClass(value) {
      if (!value) return "state-waiting";
      const v = String(value).toUpperCase();
      if (v.includes("GRANTED") || v === "VALID") return "state-granted";
      if (v.includes("DENIED") || v === "UNKNOWN") return "state-denied";
      if (v.includes("SECURE")) return "state-secure";
      if (v.includes("NORMAL")) return "state-normal";
      return "state-waiting";
    }
    function setState(id, value) { const el = document.getElementById(id); if (!el) return; el.textContent = value; el.className = "value " + stateClass(value); }
    function updateOrb(status) {
      const orb = document.getElementById("accessOrb"); if (!orb) return;
      const s = String(status).toUpperCase();
      if (s === "GRANTED") { orb.textContent = "✓"; orb.style.background = "radial-gradient(circle, rgba(57,255,136,.50), rgba(57,255,136,.06))"; orb.style.boxShadow = "0 0 30px rgba(57,255,136,.28)"; }
      else if (s === "DENIED") { orb.textContent = "✕"; orb.style.background = "radial-gradient(circle, rgba(255,79,94,.50), rgba(255,79,94,.06))"; orb.style.boxShadow = "0 0 30px rgba(255,79,94,.28)"; }
      else { orb.textContent = "◇"; orb.style.background = "radial-gradient(circle, rgba(255,209,102,.42), rgba(255,209,102,.06))"; orb.style.boxShadow = "0 0 30px rgba(255,209,102,.22)"; }
    }
    function renderLogs(logs) {
      const list = document.getElementById("logs"); if (!list) return;
      list.innerHTML = "";
      if (!logs || logs.length === 0) { const li = document.createElement("li"); li.textContent = "No access record yet."; list.appendChild(li); return; }
      logs.forEach(item => { if (!item) return; const li = document.createElement("li"); li.textContent = item; list.appendChild(li); });
    }
    async function refreshStatus() {
      try {
        const response = await fetch("/status", { cache: "no-store" });
        const data = await response.json();
        setState("accessStatus", data.access_status);
        setText("lockStatus", data.lock_status);
        setState("securityMode", data.security_mode);
        setState("irKeyStatus", data.ir_key_status);
        setText("lastUID", data.last_uid);
        setText("lastIRCode", data.last_ir_code);
        setText("lastIRProtocol", data.last_ir_protocol);
        setText("irChannel", data.ir_channel);
        setText("wifiRssi", data.wifi_rssi_dbm + " dBm");
        setText("wifiQuality", data.wifi_quality);
        setText("ipAddress", data.ip_address);
        setText("scanCount", data.scan_count);
        setText("irSignalCount", data.ir_signal_count);
        setText("uptime", data.uptime);
        setText("lastEvent", data.last_event);
        updateOrb(data.access_status);
        const remaining = Number(data.ir_valid_remaining_s || 0);
        const windowSec = Number(data.ir_valid_window_s || 30);
        const percent = Math.max(0, Math.min(100, (remaining / windowSec) * 100));
        document.getElementById("irBar").style.width = percent + "%";
        setText("irRemaining", remaining + " s remaining");
        renderLogs(data.logs);
      } catch (err) {
        setText("lastEvent", "Dashboard connection lost. Waiting for ESP8266...");
      }
    }
    async function setMode(mode) { await fetch("/api/mode?value=" + encodeURIComponent(mode), { cache: "no-store" }); refreshStatus(); }
    async function manualUnlock() { const pin = prompt("Enter demo PIN for manual unlock:"); if (pin === null) return; await fetch("/api/unlock?pin=" + encodeURIComponent(pin), { cache: "no-store" }); refreshStatus(); }
    async function resetLogs() { const pin = prompt("Enter demo PIN to reset logs:"); if (pin === null) return; await fetch("/api/reset?pin=" + encodeURIComponent(pin), { cache: "no-store" }); refreshStatus(); }
    refreshStatus();
    setInterval(refreshStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Q-Sentinel V4 Pro");
  Serial.println("-----------------");

  connectToWiFi();
  setupWebServer();

  SPI.begin();
  rfid.PCD_Init();

  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);

  Serial.print("RC522 Firmware Version: 0x");
  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: RC522 not detected.");
    lastEvent = "ERROR: RC522 not detected.";
  } else {
    Serial.println("SUCCESS: RC522 detected.");
  }

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCK_POSITION);

  irrecv.enableIRIn();

  Serial.println("Servo attached to D4.");
  Serial.println("IR receiver attached to D3.");
  Serial.println("System ready.");
  Serial.println();
}

// -------------------- LOOP --------------------
void loop() {
  server.handleClient();
  handleIR();
  handleRFID();
  handleLockTimer();
}

// -------------------- RFID HANDLING --------------------
void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lastUID = getUIDString();
  scanCount++;

  Serial.print("Card UID: ");
  Serial.println(lastUID);

  bool rfidAuthorized = isAuthorizedCard();

  if (rfidAuthorized) {
    if (securityMode == "SECURE") {
      if (isIRKeyFresh()) {
        accessStatus = "GRANTED";
        lastEvent = "RFID authorized and valid IR optical key detected. Door unlocked.";
        addLog("GRANTED | RFID + IR | UID: " + lastUID);
        Serial.println("ACCESS GRANTED: RFID + IR");
        startUnlock();
      } else {
        accessStatus = "DENIED";
        lockStatus = "LOCKED";
        lastEvent = "RFID authorized, but valid IR key is missing or expired.";
        addLog("DENIED | RFID OK, IR MISSING | UID: " + lastUID);
        Serial.println("ACCESS DENIED: IR key required.");
      }
    } else {
      accessStatus = "GRANTED";
      lastEvent = "Authorized RFID card scanned. Door unlocked.";
      addLog("GRANTED | RFID | UID: " + lastUID);
      Serial.println("ACCESS GRANTED");
      startUnlock();
    }
  } else {
    accessStatus = "DENIED";
    lockStatus = "LOCKED";
    lastEvent = "Unauthorized RFID card scanned. Door remained locked.";
    addLog("DENIED | RFID | UID: " + lastUID);
    Serial.println("ACCESS DENIED");
    Serial.println("Servo remains locked.");
  }

  Serial.println();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(350);
}

// -------------------- IR HANDLING --------------------
void handleIR() {
  if (irrecv.decode(&irResults)) {
    irSignalCount++;

    lastIRProtocol = typeToString(irResults.decode_type);
    lastIRCode = "0x" + uint64ToString(irResults.value, HEX);
    lastIRCode.toUpperCase();
    irChannelStatus = "SIGNAL RECEIVED";

    Serial.println();
    Serial.print("IR Protocol: ");
    Serial.println(lastIRProtocol);
    Serial.print("IR Code: ");
    Serial.println(lastIRCode);
    Serial.print("IR Bits: ");
    Serial.println(irResults.bits);

    if (irResults.value == AUTHORIZED_IR_CODE) {
      irKeyStatus = "VALID";
      lastValidIRTime = millis();
      lastEvent = "Valid IR optical key received.";
      addLog("IR VALID | " + lastIRProtocol + " | " + lastIRCode);
      Serial.println("IR KEY: VALID");
    } else {
      irKeyStatus = "UNKNOWN";
      lastEvent = "Unknown IR signal received.";
      addLog("IR UNKNOWN | " + lastIRProtocol + " | " + lastIRCode);
      Serial.println("IR KEY: UNKNOWN");
    }

    Serial.println();
    irrecv.resume();
  }

  if (irKeyStatus == "VALID" && !isIRKeyFresh()) irKeyStatus = "EXPIRED";
}

bool isIRKeyFresh() {
  if (lastValidIRTime == 0) return false;
  return (millis() - lastValidIRTime) <= IR_VALID_WINDOW_MS;
}

unsigned long getIRRemainingSeconds() {
  if (!isIRKeyFresh()) return 0;
  unsigned long elapsed = millis() - lastValidIRTime;
  unsigned long remainingMs = IR_VALID_WINDOW_MS - elapsed;
  return remainingMs / 1000;
}

// -------------------- SERVO LOCK HANDLING --------------------
void startUnlock() {
  lockStatus = "UNLOCKED";
  unlockActive = true;
  unlockEndTime = millis() + UNLOCK_DURATION_MS;
  Serial.println("Servo: UNLOCK position");
  lockServo.write(UNLOCK_POSITION);
}

void handleLockTimer() {
  if (unlockActive && (long)(millis() - unlockEndTime) >= 0) {
    unlockActive = false;
    lockStatus = "LOCKED";
    Serial.println("Servo: LOCK position");
    lockServo.write(LOCK_POSITION);
  }
}

// -------------------- WI-FI --------------------
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    dotCount++;
    if (dotCount > 80) {
      Serial.println();
      Serial.println("Wi-Fi connection timeout.");
      lastEvent = "Wi-Fi connection timeout.";
      return;
    }
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// -------------------- WEB SERVER ROUTES --------------------
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/status", handleStatusJson);
  server.on("/api/mode", handleModeApi);
  server.on("/api/unlock", handleUnlockApi);
  server.on("/api/reset", handleResetApi);
  server.begin();
  Serial.println("Web server started.");
}

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleStatusJson() {
  String json = "";
  json += "{";
  json += "\"project\":\"Q-Sentinel\",";
  json += "\"version\":\"V4 Pro\",";
  json += "\"device\":\"ESP8266 NodeMCU\",";
  json += "\"rfid\":\"RC522\",";
  json += "\"ip_address\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"wifi_rssi_dbm\":" + String(WiFi.RSSI()) + ",";
  json += "\"wifi_quality\":\"" + getWiFiQuality() + "\",";
  json += "\"uptime_ms\":" + String(millis()) + ",";
  json += "\"uptime\":\"" + getUptimeString() + "\",";
  json += "\"last_uid\":\"" + jsonEscape(lastUID) + "\",";
  json += "\"authorized_uid\":\"45 B6 0A 01\",";
  json += "\"access_status\":\"" + accessStatus + "\",";
  json += "\"lock_status\":\"" + lockStatus + "\",";
  json += "\"security_mode\":\"" + securityMode + "\",";
  json += "\"ir_channel\":\"" + irChannelStatus + "\",";
  json += "\"last_ir_protocol\":\"" + jsonEscape(lastIRProtocol) + "\",";
  json += "\"last_ir_code\":\"" + jsonEscape(lastIRCode) + "\",";
  json += "\"ir_key_status\":\"" + irKeyStatus + "\",";
  json += "\"ir_valid_remaining_s\":" + String(getIRRemainingSeconds()) + ",";
  json += "\"ir_valid_window_s\":" + String(IR_VALID_WINDOW_MS / 1000) + ",";
  json += "\"last_event\":\"" + jsonEscape(lastEvent) + "\",";
  json += "\"scan_count\":" + String(scanCount) + ",";
  json += "\"ir_signal_count\":" + String(irSignalCount) + ",";
  json += "\"logs\":[";

  bool first = true;
  for (int i = 0; i < 8; i++) {
    if (accessLog[i] != "") {
      if (!first) json += ",";
      json += "\"" + jsonEscape(accessLog[i]) + "\"";
      first = false;
    }
  }

  json += "]";
  json += "}";
  server.send(200, "application/json", json);
}

void handleModeApi() {
  if (!server.hasArg("value")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Missing mode value\"}");
    return;
  }

  String mode = server.arg("value");
  mode.toUpperCase();

  if (mode == "SECURE") {
    securityMode = "SECURE";
    lastEvent = "Security mode changed to SECURE. RFID + valid IR key required.";
    addLog("MODE | SECURE");
    server.send(200, "application/json", "{\"ok\":true,\"mode\":\"SECURE\"}");
    return;
  }

  if (mode == "NORMAL") {
    securityMode = "NORMAL";
    lastEvent = "Security mode changed to NORMAL. RFID card is enough.";
    addLog("MODE | NORMAL");
    server.send(200, "application/json", "{\"ok\":true,\"mode\":\"NORMAL\"}");
    return;
  }

  server.send(400, "application/json", "{\"ok\":false,\"message\":\"Invalid mode\"}");
}

void handleUnlockApi() {
  if (!server.hasArg("pin") || server.arg("pin") != DASHBOARD_PIN) {
    lastEvent = "Manual unlock rejected. Wrong PIN.";
    addLog("MANUAL UNLOCK REJECTED | Wrong PIN");
    server.send(403, "application/json", "{\"ok\":false,\"message\":\"Wrong PIN\"}");
    return;
  }

  accessStatus = "GRANTED";
  lastEvent = "Manual unlock triggered from dashboard.";
  addLog("MANUAL UNLOCK | Dashboard PIN accepted");
  Serial.println("MANUAL UNLOCK from dashboard.");
  startUnlock();
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Manual unlock triggered\"}");
}

void handleResetApi() {
  if (!server.hasArg("pin") || server.arg("pin") != DASHBOARD_PIN) {
    lastEvent = "Reset logs rejected. Wrong PIN.";
    addLog("RESET LOGS REJECTED | Wrong PIN");
    server.send(403, "application/json", "{\"ok\":false,\"message\":\"Wrong PIN\"}");
    return;
  }

  for (int i = 0; i < 8; i++) accessLog[i] = "";
  accessLog[0] = "[" + getUptimeString() + "] Logs cleared.";
  scanCount = 0;
  irSignalCount = 0;
  lastEvent = "Access logs cleared from dashboard.";
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Logs cleared\"}");
}

// -------------------- HELPERS --------------------
String getUIDString() {
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidString += "0";
    uidString += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidString += " ";
  }
  uidString.toUpperCase();
  return uidString;
}

bool isAuthorizedCard() {
  if (rfid.uid.size != authorizedUIDSize) return false;
  for (byte i = 0; i < authorizedUIDSize; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) return false;
  }
  return true;
}

void addLog(String newLog) {
  for (int i = 7; i > 0; i--) accessLog[i] = accessLog[i - 1];
  accessLog[0] = "[" + getUptimeString() + "] " + newLog;
}

String getUptimeString() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  String uptime = "";
  if (hours < 10) uptime += "0";
  uptime += String(hours);
  uptime += ":";
  if (minutes < 10) uptime += "0";
  uptime += String(minutes);
  uptime += ":";
  if (seconds < 10) uptime += "0";
  uptime += String(seconds);
  return uptime;
}

String getWiFiQuality() {
  int rssi = WiFi.RSSI();
  if (rssi >= -55) return "Excellent";
  if (rssi >= -65) return "Good";
  if (rssi >= -75) return "Fair";
  return "Weak";
}

String jsonEscape(String input) {
  String output = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == '\"') output += "\\\"";
    else if (c == '\\') output += "\\\\";
    else if (c == '\n') output += "\\n";
    else if (c == '\r') output += "\\r";
    else output += c;
  }
  return output;
}
