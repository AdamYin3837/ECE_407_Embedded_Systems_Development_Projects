# Project #2 Report
**Team:** Group 0x05 (Uzair Tariq, Adam Yin)
**Target Device:** Raspberry Pi Pico (RP2040)
**Subject:** Analogue Sensing, Motor Control, and Hardware Emulation

## Executive Summary
This project successfully integrates analogue ADC inputs and PWM signal generation to control a physical DC motor (fan). We prototyped the system in Wokwi, utilizing an RGB LED ring to visually simulate the fan's speed and direction. Upon moving to hardware, we successfully translated this logic to drive the real DC motor via an H-bridge, achieving both variable speed control and the bonus objective of single-potentiometer bidirectional control.

## Key Learnings & Architecture
The system's core architecture relies on mapping the Pico's 12-bit ADC (0-4095) to bidirectional PWM signals. By defining the potentiometer's center (~2048) as a "Stop" state with a +/- 100 software deadzone to prevent motor jitter, we seamlessly mapped the upper and lower bounds to forward and reverse PWM channels. Additionally, during the simulation phase, we utilized the RP2040's Programmable I/O (PIO) to drive a LED ring to simulate the fan spinning. This provided valuable experience with PIO state machines before we streamlined the final hardware code to focus directly on the H-bridge and motor.

## Challenges & Solutions
**Challenge 1: Wokwi PIO Compilation Limitations (Simulation Phase)**
During Part 1, we used the WS2812B ring to emulate our motor, but encountered a significant challenge. Wokwi's browser-based compilation environment struggled to process and compile custom `.pio` files into the generated C headers standard to the Pico SDK workflow.
* **Solution:** To successfully emulate the fan's behavior visually without sacrificing PIO functionality, we bypassed the `.pio` build step entirely. We compiled the PIO assembly instructions locally, extracted the resulting 16-bit machine code instructions, and hardcoded them directly into our `main.c` as a static array. This allowed us to fully utilize the PIO state machine within Wokwi's constraints.

**Challenge 2: Motor Jitter at Rest**
Initial testing of the bidirectional logic revealed that the ADC values naturally fluctuated slightly even when the potentiometer was untouched. This caused the H-bridge to rapidly stutter between low-speed forward and reverse when resting in the middle.
* **Solution:** We introduced a software deadzone (`const int deadzone = 100`). Any ADC reading within this threshold forces both PWM duty cycles to 0, ensuring the motor remains perfectly still and preventing unnecessary wear on the hardware or H-bridge shoot-through.
