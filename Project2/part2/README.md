# Part 2: Hardware Implementation

This directory contains the C/C++ SDK source code to run the DC motor control system on physical hardware (Raspberry Pi Pico, TC1508A H-Bridge, and WS2812B LED Ring).

## Setup Instructions
1. Clone this repository and ensure the Pico C/C++ SDK is properly configured on your machine.
2. Navigate to the `/Part2` directory and create a build folder:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
3. Flash the resulting .uf2 file.