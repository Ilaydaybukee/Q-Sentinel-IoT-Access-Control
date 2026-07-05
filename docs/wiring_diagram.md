# Q-Sentinel V4 Pro Wiring Guide

## ESP8266 NodeMCU + RC522 RFID

| RC522 Pin | ESP8266 NodeMCU Pin |
|---|---|
| SDA / SS | D2 |
| SCK | D5 |
| MOSI | D7 |
| MISO | D6 |
| RST | D1 |
| 3.3V | 3V3 |
| GND | GND |
| IRQ | Not connected |

> Do not power the RC522 from 5V. The RC522 should be powered from 3.3V.

## Servo Motor

| Servo Wire | ESP8266 NodeMCU Pin |
|---|---|
| Red | VIN / 5V |
| Brown / Black | GND |
| Yellow / Orange | D4 |

> The servo ground and ESP8266 ground must be common. If the servo jitters or resets the ESP8266, use an external regulated 5V supply for the servo and keep the grounds common.

## IR Receiver Module

| IR Receiver Pin | ESP8266 NodeMCU Pin |
|---|---|
| + | 3V3 |
| - | GND |
| OUT / S | D3 |

> D3 is a boot-related pin on ESP8266. If upload fails, temporarily disconnect the IR OUT wire, upload the firmware, then reconnect it.
