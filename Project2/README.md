# ECE 407 - Project 2: Analogue Sensing and Motor Control

## Project Purpose
The purpose of this project is to explore analogue sensing and physical control using a Raspberry Pi Pico. We designed and implemented a system that reads an analogue input from a potentiometer to control the speed and direction of a DC motor via a TC1508A H-bridge driver. Additionally, the system provides real-time visual feedback of the motor's state using a 12-LED WS2812B RGB ring and terminal outputs. 

The project includes a stretch goal implementation: bidirectional motor control (forward/reverse) mapped to a single potentiometer without the use of external buttons, complete with a software-defined deadzone for stability.

## Learning Objectives
Through the development of this two-part project, our team successfully learned how to:
* Interface a dual-channel DC motor driver (H-bridge) with a microcontroller.
* Generate and manipulate Pulse-Width Modulation (PWM) signals to precisely vary motor speed.
* Utilize Programmable I/O (PIO) to control an addressable RGB LED ring (WS2812/NeoPixel).
* Map raw ADC input values to complex visual feedback patterns and bidirectional motor control logic.
* Develop structured, well-documented, and hardware-agnostic embedded firmware.

## Repository Structure
* `/Part1`: Contains the Wokwi emulation files (ZIP).
* `/Part2`: Contains the hardware implementation source code (`main.c`, `CMakeLists.txt`), circuit sketches, and circuit photos.
* `/docs`: Contains the final project report and the demo video.