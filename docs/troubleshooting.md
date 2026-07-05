# Troubleshooting

## RC522 Returns `0x00` or `0xFF`

Check the following:

- RC522 is powered from 3.3V.
- GND is connected to ESP8266 GND.
- SDA is connected to D2.
- RST is connected to D1.
- SCK is connected to D5.
- MISO is connected to D6.
- MOSI is connected to D7.
- Jumper wires are firmly connected.

## Servo Does Not Move

Check the following:

- Servo red wire is connected to VIN / 5V.
- Servo ground is connected to ESP8266 GND.
- Servo signal wire is connected to D4.
- ESP8266 and servo share common ground.
- If USB power is not enough, use an external regulated 5V supply for the servo.

## Dashboard Does Not Open

Check the following:

- ESP8266 and computer/phone are connected to the same Wi-Fi network.
- The IP address from Serial Monitor is typed correctly in the browser.
- Some school, dorm, or corporate networks block device-to-device communication.
- If needed, use a mobile hotspot and connect both the ESP8266 and computer to it.

## Upload Fails

The IR OUT pin is connected to D3, which is a boot-related pin on ESP8266.

Try this:

1. Disconnect the IR OUT wire from D3.
2. Upload the firmware.
3. Reconnect the IR OUT wire after upload is complete.

## IR Remote Sends Different Codes

Different remote buttons send different IR codes. The currently authorized code is:

```text
0x150F447C18800F00
```

To authorize a different remote button, read the code from Serial Monitor and update `QS_AUTHORIZED_IR_CODE` in `config.h`.

## Secure Mode Denies Access Even With the Correct RFID Card

This is expected if the valid IR key was not received recently. In Secure Mode, the system requires:

```text
Authorized RFID card + Valid IR optical key
```

By default, the valid IR key window is 30 seconds.
