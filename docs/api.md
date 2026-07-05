# Q-Sentinel API

The ESP8266 web server provides the following endpoints.

## Dashboard

```text
GET /
```

Returns the live web dashboard.

## Status JSON

```text
GET /status
```

Example response:

```json
{
  "project": "Q-Sentinel",
  "version": "V4 Pro",
  "device": "ESP8266 NodeMCU",
  "rfid": "RC522",
  "ip_address": "10.141.192.102",
  "wifi_rssi_dbm": -53,
  "wifi_quality": "Excellent",
  "uptime": "00:02:14",
  "last_uid": "45 B6 0A 01",
  "access_status": "GRANTED",
  "lock_status": "LOCKED",
  "security_mode": "SECURE",
  "ir_channel": "SIGNAL RECEIVED",
  "last_ir_protocol": "XMP",
  "last_ir_code": "0X150F447C18800F00",
  "ir_key_status": "VALID",
  "ir_valid_remaining_s": 18,
  "scan_count": 3,
  "ir_signal_count": 7,
  "logs": []
}
```

## Mode Change

```text
GET /api/mode?value=normal
GET /api/mode?value=secure
```

`normal` mode grants access with the authorized RFID card only.

`secure` mode requires both the authorized RFID card and a valid IR optical key.

## Manual Unlock

```text
GET /api/unlock?pin=1234
```

Triggers the servo unlock action when the demo PIN is correct.

## Reset Logs

```text
GET /api/reset?pin=1234
```

Clears dashboard event logs when the demo PIN is correct.

> The PIN mechanism is for demonstration only. It is not production-grade security.
