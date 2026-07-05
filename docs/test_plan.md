# Q-Sentinel Test Plan

## 1. Boot Test

Open Serial Monitor at `115200` baud. Expected output:

```text
Wi-Fi connected!
IP Address: ...
Web server started.
RC522 Firmware Version: 0x92
SUCCESS: RC522 detected.
Servo attached to D4.
IR receiver attached to D3.
System ready.
```

## 2. Normal Mode Test

1. Open the dashboard in a browser.
2. Select Normal Mode.
3. Scan the authorized RFID card.
4. The servo should unlock and then lock again.

Expected Serial Monitor output:

```text
ACCESS GRANTED
Servo: UNLOCK position
Servo: LOCK position
```

## 3. Secure Mode Without IR Key

1. Select Secure Mode.
2. Scan the authorized RFID card without sending the IR key first.

Expected output:

```text
ACCESS DENIED: IR key required.
```

The servo should not move.

## 4. Secure Mode With IR Key + RFID

1. Select Secure Mode.
2. Send the authorized IR code from the remote controller.
3. Scan the authorized RFID card within the valid IR time window.

Expected output:

```text
IR KEY: VALID
ACCESS GRANTED: RFID + IR
Servo: UNLOCK position
Servo: LOCK position
```

## 5. Unauthorized RFID Test

1. Scan the unauthorized RFID card.

Expected output:

```text
ACCESS DENIED
Servo remains locked.
```

## 6. Dashboard Button Test

Verify the following dashboard controls:

- Normal Mode
- Secure Mode
- Manual Unlock with PIN `1234`
- Reset Logs with PIN `1234`
- JSON API button
