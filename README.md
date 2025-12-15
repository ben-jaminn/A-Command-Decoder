# Robot Command Decoder

## Overview
This project is a **Bare-Metal Firmware Simulation** written in C. It mimics the behavior of a microcontroller driver that listens for serial commands from a host (like a UART interface), parses the data, and controls a robotic hardware system.

The system features a **Pipeline Architecture** that captures raw character streams into a safety buffer before processing them, simulating real-world "Interrupt Request" (IRQ) or polling behavior.

## Features 🚀
* **Serial Pipeline:** Implements a circular-style line buffer to capture raw input character-by-character.
* **Protocol Parsing:** Uses `strtok` and pointer arithmetic to slice standard ASCII commands (e.g., `SPEED:100`).
* **Data Conversion:** Safely converts text-based payloads into integers for arithmetic logic using `atoi`.
* **Hardware Safety:** Includes logic safeguards (e.g., Over-Speed Protection) to trigger emergency ALARM states if inputs exceed defined limits.
* **Noise Filtering:** (Optional) Ignores comments (`//`) and prevents buffer overflow attacks.

## System Model
The software controls a simulated Robot Struct with the following state:
* **Speed:** 0 - 100%
* **Temperature:** Celsius
* **State:** IDLE, ACTIVE, ALARM

## Supported Commands
| Command | Format | Description |
| :--- | :--- | :--- |
| **Set Speed** | `SPEED:<value>` | Sets motor speed (0-100). Triggers ALARM if > 100. |
| **Set State** | `STATE:<text>` | Manually overrides system state (e.g., ACTIVE). |
| **Calibrate Temp** | `TEMP:<value>` | Updates internal temperature sensor reading. |

## How to Run 💻

### 1. Compile
```bash
gcc shell.c -o robot_driver
