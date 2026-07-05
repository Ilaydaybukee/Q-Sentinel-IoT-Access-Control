#ifndef QS_CONFIG_H
#define QS_CONFIG_H

// Wi-Fi settings
// Replace these values with your own 2.4 GHz Wi-Fi name and password.
#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Dashboard demo PIN
// Used for Manual Unlock and Reset Logs actions.
#define QS_DASHBOARD_PIN "1234"

// Authorized IR remote code
// Code detected during testing: 0x150F447C18800F00
#define QS_AUTHORIZED_IR_CODE 0x150F447C18800F00ULL

// IR key validity window in milliseconds
// 30000 = 30 seconds. Use 10000 for a stricter 10-second window.
#define QS_IR_VALID_WINDOW_MS 30000UL

#endif
