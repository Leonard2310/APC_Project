# MACS - Museum Access Control System

<p align="center">
   <img src="https://img.shields.io/badge/Platform-STM32F303VCT6-blue" alt="Platform">
   <img src="https://img.shields.io/badge/IDE-STM32CubeIDE-green" alt="IDE">
   <img src="https://img.shields.io/badge/Language-C-orange" alt="Language">
   <img src="https://img.shields.io/badge/License-GPLv3-yellow" alt="License">
</p>

## 📋 Overview

**MACS** (Museum Access Control System) is an embedded system for automated access control in museums, developed on the **STM32F303VCT6 Discovery Board**. The system manages the full visitor flow (entry and exit) using PIR motion sensors, servo motors for turnstiles/gates, an SSD1306 OLED display for user feedback, and status LEDs.

The project was developed using **STM32CubeIDE** and the firmware package **STM32Cube_FW_F3_V1.11.0**.

### 🎯 Project Goals

- Automate ticket purchase and entrance flow
- Enforce a maximum museum capacity (4 visitors)
- Provide an independent ambient lighting system
- Offer a clear and friendly OLED-based user interface
- Ensure smooth and realistic motion of all servomotors

---

## 🏗️ Hardware Architecture

### Development Board
- **MCU**: STM32F303VCT6 (ARM Cortex-M4, 72MHz, 256KB Flash, 48KB SRAM)
- **Board**: STM32F3 Discovery

### External Components

| Component | Pin | Function |
|-----------|-----|----------|
| **PIR1** | PB4 | Entrance sensor – detects visitors approaching the museum |
| **PIR2** | PB2 | Turnstile sensor – confirms that the visitor has passed |
| **PIR3** | PB9 | Ambient light sensor (independent lighting system) |
| **PIR4** | PB15 | Exit sensor – detects visitors leaving the museum |
| **Servo1** | PB0 (TIM3_CH3) | Ticket turnstile actuator |
| **Servo2** | PB10 (TIM2_CH3) | Entrance gate actuator |
| **Servo3** | PB1 (TIM1_CH3N) | Exit gate actuator |
| **OLED Display** | PB6/PB7 (I2C1) | SSD1306 128x64 – user feedback |
| **Blue LED** | PE8 | System idle/ready |
| **Red LED** | PE9 | Transaction in progress |
| **Green LED** | PE10 | Access granted |
| **Ambient LEDs** | PB3, PB11 | Lighting controlled by PIR3 |
| **Push Button** | PB8 | Ticket purchase confirmation (active low) |

### Connection Diagram

```
                    STM32F303VCT6 Discovery
                    ┌─────────────────────┐
                    │                     │
    PIR1 ──────────►│ PB4 (EXTI4/Polled)  │
    PIR2 ──────────►│ PB2 (EXTI2)         │
    PIR3 ──────────►│ PB9 (EXTI9)         │
    PIR4 ──────────►│ PB15 (EXTI15)       │
                    │                     │
    Servo1 ◄────────│ PB0 (TIM3_CH3)      │
    Servo2 ◄────────│ PB10 (TIM2_CH3)     │
    Servo3 ◄────────│ PB1 (TIM1_CH3N)     │
                    │                     │
    OLED SDA ◄─────►│ PB7 (I2C1_SDA)      │
    OLED SCL ◄──────│ PB6 (I2C1_SCL)      │
                    │                     │
    Button ────────►│ PB8 (Input Pull-up) │
                    │                     │
    LED Blu ◄───────│ PE8                 │
    LED Rosso ◄─────│ PE9                 │
    LED Verde ◄─────│ PE10                │
    LED PIR3 ◄──────│ PB3, PB11           │
                    └─────────────────────┘
```

---

## ⚙️ Timer Configuration

### PWM Timers (Servo Control)

| Timer | Prescaler | Period | PWM Frequency | Usage |
|-------|-----------|--------|---------------|-------|
| TIM1  | 7         | 19999  | 50Hz (20ms)   | Servo3 – Exit gate |
| TIM2  | 7         | 19999  | 50Hz (20ms)   | Servo2 – Entrance gate |
| TIM3  | 7         | 19999  | 50Hz (20ms)   | Servo1 – Ticket turnstile |

**PWM formula**: `Timer clock = 8MHz / 8 = 1MHz` → `Period = 20000 ticks = 20ms`.

**Angle-to-pulse mapping**:
- 0° → 500µs
- 90° → 1500µs  
- 180° → 2500µs

### Base Timers (Timing)

| Timer | Prescaler | Period | Interval | Usage |
|-------|-----------|--------|----------|-------|
| TIM4  | 7999      | 1999   | 2 s      | Auto-off ambient LEDs |
| TIM6  | 199       | 999    | 25 ms    | Exit gate state machine (non-blocking) |

---

## 🔄 System Behaviour

### 1. Idle State
```
┌─────────────────────────────────────┐
│  Display: "MUSEUM - Welcome!"       │
│  Blue LED: ON                       │
│  Waiting for PIR1 trigger...        │
└─────────────────────────────────────┘
```

### 2. Visitor Detection (PIR1)
```
┌─────────────────────────────────────┐
│  Check museum capacity              │
│  ├─► If full: show "FULL"           │
│  └─► If available: start process    │
└─────────────────────────────────────┘
```

### 3. Ticket Purchase Process
```
┌─────────────────────────────────────┐
│  Red LED: ON                        │
│  Display: 10-second countdown       │
│  Waiting for button press...        │
│  ├─► Confirmed: open turnstile      │
│  └─► Timeout: "Purchase Cancelled"  │
└─────────────────────────────────────┘
```

### 4. Visitor Entry
```
┌─────────────────────────────────────┐
│  Servo1: Opens turnstile (0°→180°)  │
│  Wait for PIR2 (passage)            │
│  Green LED: ON                      │
│  Servo2: Opens entrance gate        │
│  visitor_count++                    │
└─────────────────────────────────────┘
```

### 5. Visitor Exit (Independent)
```
┌─────────────────────────────────────┐
│  PIR4 trigger (EXTI15)              │
│  If visitor_count > 0:              │
│  ├─► TIM6 starts state machine      │
│  ├─► Servo3: Opens (0°→180°, 2.25s) │
│  ├─► Wait (~8 seconds)              │
│  ├─► Servo3: Closes (180°→0°)       │
│  └─► visitor_count--                │
└─────────────────────────────────────┘
```

### 6. Ambient Lighting (Independent)
```
┌─────────────────────────────────────┐
│  PIR3 trigger (EXTI9)               │
│  LEDs PB3, PB11: ON                 │
│  TIM4: 2-second timeout             │
│  LEDs PB3, PB11: OFF                │
└─────────────────────────────────────┘
```

---

## 📁 Project Structure

```
APC_Project/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # Pin definitions and prototypes
│   │   ├── gpio.h              # GPIO header
│   │   ├── i2c.h               # I2C header
│   │   ├── tim.h               # Timer header
│   │   ├── ssd1306.h           # OLED display driver
│   │   ├── ssd1306_fonts.h     # Display fonts
│   │   └── stm32f3xx_*.h       # HAL headers
│   ├── Src/
│   │   ├── main.c              # MACS main application logic
│   │   ├── gpio.c              # GPIO/EXTI configuration
│   │   ├── i2c.c               # I2C configuration
│   │   ├── tim.c               # Timer configuration
│   │   ├── ssd1306.c           # OLED display driver
│   │   ├── ssd1306_fonts.c     # Font data
│   │   └── stm32f3xx_*.c       # HAL sources
│   └── Startup/
│       └── startup_stm32f303vctx.s
├── Drivers/
│   ├── CMSIS/                  # Core ARM
│   └── STM32F3xx_HAL_Driver/   # HAL ST
├── Debug/                      # Build output
├── APC_Project.ioc             # STM32CubeMX configuration
├── STM32F303VCTX_FLASH.ld      # Linker script
├── LICENSE
└── README.md
```

---

## 🛠️ Build and Flash

### Prerequisites
- **STM32CubeIDE** (v1.13.0 or later)
- **STM32Cube_FW_F3_V1.11.0** firmware package installed in STM32CubeIDE
- **ST-Link** debugger (embedded on the STM32F3 Discovery board)

### Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/Leonard2310/APC_Project.git
   ```

2. **Open the project in STM32CubeIDE**
   ```text
   File → Open Projects from File System → Select APC_Project folder
   ```

3. **Build the project**
   ```text
   Project → Build Project (Ctrl+B)
   ```

4. **Flash the board**
   ```text
   Run → Debug (F11) or Run → Run (Ctrl+F11)
   ```

---

## 📊 Technical Specifications

| Parameter | Value |
|-----------|-------|
| System clock | 8 MHz (HSI) |
| Max visitors | 4 |
| Ticket purchase timeout | 10 seconds |
| Ambient LED timeout | 2 seconds |
| Exit gate open time | ~8 seconds |
| Servo movement speed | 2°/25ms (~2.25s for 180°) |
| Servo PWM frequency | 50 Hz |
| Servo pulse range | 500–2500 µs |

---

## 🔧 Key Functions

### `Servo_SetAngle(htim, channel, angle)`
Sets the servo angle (0–180°) by mapping the angle to the PWM pulse width.

### `HAL_GPIO_EXTI_Callback(GPIO_Pin)`
Handles external interrupts for PIR2, PIR3, and PIR4.

### `HAL_TIM_PeriodElapsedCallback(htim)`
Handles timer callbacks for TIM4 (ambient LED auto-off) and TIM6 (exit gate state machine).

### `Update_Display_Idle(buffer)`
Updates the OLED display with the current system state and visitor counter.

---

## 👥 Authors

- **Salvatore Maione**
- **Luisa Ciniglio**
- **Roberta Granata**
- [Leonardo Catello](https://github.com/Leonard2310)
- [Salvatore Maione](https://github.com/salvatore22maione)
- [Luisa Ciniglio](https://github.com/luisaciny)
- [Roberta Granata](https://github.com/robi1203)

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**.  
See the [LICENSE](LICENSE) file for full details.