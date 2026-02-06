# Project 1: Embedded HMI with Rotary Encoder & LED Ring

## Overview
This project involves the design and implementation of an embedded Human-Machine Interface (HMI) using the RP2040 (Raspberry Pi Pico). The system mimics common interfaces found in consumer electronics by utilizing a mechanical rotary encoder for user input and an addressable RGB LED ring (WS2812B) for real-time visual feedback.

The core logic decodes quadrature signals to determine direction and position, mapping these physical actions to complex visual patterns on the LED ring.

## Learning Objectives
By developing this system, the following technical objectives were achieved:
* Interfacing a mechanical rotary encoder with a microcontroller.
* Decoding quadrature signals to determine rotation direction (CW/CCW) and position.
* Implementing input handling (Polling/Interrupts).
* Controlling addressable WS2812B LEDs via serialized data.
* Mapping user input to real-time visual feedback.
* Developing structured and documented embedded firmware.

## Application Description
**Imagined Application: Multi-Mode Haptic Physics Simulator & Reflex Trainer**

This system simulates a precision input dial with physics-based visual feedback. It demonstrates how a simple rotary input can be enhanced with software to mimic physical properties like momentum, as well as gamifying the user input.

The application operates in three distinct modes, toggled via the encoder's built-in push button:

### 1. Velocity Visualization (Speed Monitor)
In this mode, the system provides real-time visual feedback on the angular velocity of the rotation.
* **Behavior:** As the user spins the encoder, the number of illuminated LEDs (or their color intensity) increases relative to the speed of rotation.
* **Use Case:** Mimics a "jog wheel" on audio equipment where faster rotation seeks through audio tracks more quickly.

### 2. Inertia Simulation (Virtual Flywheel)
This mode implements a software-based physics model to simulate momentum and friction.
* **Behavior:** When the user spins the encoder and releases it, the LED indicator does not stop instantly. Instead, it "coasts" (continues rotating visually) and gradually decelerates to a halt, mimicking the behavior of a heavy physical flywheel.
* **Use Case:** Demonstrates how visual cues can create a sense of "weight" in a digital interface, similar to the scrolling momentum found on modern smartphones.

### 3. "Target Capture" Game
A gamified mode designed to test user precision and reaction time.
* **Behavior:** A specific LED on the ring lights up as a "Target." The user controls a second "Player" cursor via the encoder. The objective is to rotate the Player cursor to overlap with the Target. Once caught, the Target moves to a new random location.
* **Use Case:** Demonstrates precise position mapping and interactive logic loops.

## Hardware Components
* **Microcontroller:** Raspberry Pi Pico (RP2040) (Dual setup with Debug Probe).
* **Input:** Mechanical Rotary Encoder (24 Quadrature, Incremental) with Push Button.
* **Output:** 12-LED Ring (WS2812B / NeoPixel).
* **Miscellaneous:** Breakout board, current-limiting resistors (for encoder RGB), breadboard, and jumper wires.

## Wiring & Pinout

| Component | Pin Function | Pico GPIO Pin |
| :--- | :--- | :--- |
| **Encoder** | Channel A | GPIO **[X]** |
| | Channel B | GPIO **[X]** |
| | Switch (SW) | GPIO **[X]** |
| | LED Red | GPIO **[X]** |
| | LED Green | GPIO **[X]** |
| | LED Blue | GPIO **[X]** |
| **LED Ring** | Data In (DI) | GPIO **[X]** |
| **Power** | VCC | 3.3V / VBUS |
| | GND | GND |

> **Note:** The Encoder LEDs utilize resistors to limit current. The WS2812B ring is connected directly to the specific GPIO.

## Directory Structure
* `src/`: Contains all source code (`.c`, `.h`).
* `docs/`: Contains project documentation, datasheets, the project report, and the demo video.
* `CMakeLists.txt`: Build configuration.
* `README.md`: Project documentation (this file).

## How to Build and Run

### Prerequisites
* Raspberry Pi Pico SDK installed and configured.
* CMake and Make/Ninja.
* ARM GCC Toolchain (`arm-none-eabi-gcc`).
* VS Code (recommended) with CMake Tools.

### Build Steps
1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd <repo-name>
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake:**
    ```bash
    cmake ..
    ```

4.  **Compile:**
    ```bash
    make
    ```

5.  **Flash the Pico:**
    * Hold the `BOOTSEL` button on the Pico and plug it into USB.
    * Copy the generated `.uf2` file from the `build` directory to the `RPI-RP2` drive.
    * *Alternatively, use the Debug Probe via VS Code to flash and debug.*

## Features Implemented
- [x] Basic Quadrature Decoding (CW/CCW).
- [x] WS2812B LED Ring Control.
- [x] **Bonus:** Indication of speed of rotation (Mode 1).
- [x] **Bonus:** Complex animation/Inertia (Mode 2).
- [ ] **Bonus:** PWM control of the Encoder's built-in RGB LED. (Check if applicable)

---

**Course:** [Insert Course Name/Number]
**Team:** [Insert Team Name or Members]
**Date:** [Insert Date]
