# Q-Sentinel Qiskit QKD Demo

This folder contains a small **Qiskit-based BB84 simulation** for the Q-Sentinel project.

## Why it exists

Q-Sentinel is an embedded IoT access control demo using:

- ESP8266 NodeMCU
- RC522 RFID reader
- Servo lock
- IR optical key validation
- Live web dashboard

The Qiskit script adds a **quantum-inspired secure communication layer** to the repository. It does not perform real quantum communication. It simulates the BB84 key distribution workflow and estimates QBER.

## Install

```bash
pip install -r requirements.txt
```

## Run

```bash
python qiskit_bb84_demo.py
```

## Expected idea

A clean channel should produce low QBER. An intercept-resend eavesdropper should increase QBER, which lets the system classify the channel as suspicious.
