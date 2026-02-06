# Project 1
Team: Group 0x05 (Uzair Tariq, Adam Yin)
Target Device: Raspberry Pi Pico (RP2040)

## 1. Project Overview
For this project, we developed an interactive system that explores the mechanical feedback of a rotary encoder through three distinct software modes.

### 1.1 Core Functionalities
1. Rotation Speed Visualization: The LED ring acts as a tachometer. The faster the user spins the encoder, the more the ring fills up and changes color (transitioning from cool to warm hues).

2. Rotation Inertia Simulation: We implemented a "virtual flywheel" effect. When the user stops spinning the physical encoder, the LEDs continue to "coast" and slowly decelerate based on a programmed friction coefficient, demonstrating software-defined inertia.

3. Target Catch Game: A "target" LED appears at a random position on the ring. The user must spin the encoder to align their cursor LED with the target and press the encoder button to "catch" it, testing speed and precision.

## 2. Technical Implementation & Challenges

### 2.1 Decoding with Gray Code
Challenge: During early testing, we encountered significant "contact bounce" and signal noise. This caused the LED ring to flicker or jump backward even when spinning in one direction.
Resolution: We moved away from simple state checking and implemented a robust Gray Code state machine. By analyzing the bitwise transition between Phase A and Phase B, we ensured that only valid state changes (00 -> 01 -> 11 -> 10) were registered. This filtered out the mechanical noise and made the tracking feel 1:1 with physical movement.

## 3. Conclusion
We successfully bypassed the limitations of mechanical hardware through clever software logic. By implementing Gray Code decoding, we created a high-fidelity HMI that feels professional and responsive. The addition of the "Target Catch" game demonstrates the versatility of the RP2040 when combining GPIO interrupts with PIO-driven visuals.