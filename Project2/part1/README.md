# Part 1: Wokwi Emulation

This directory contains the Wokwi simulation for the analogue sensing and control project. Because Wokwi does not natively feature a DC motor or H-bridge, we emulated the H-bridge inputs using two LEDs (Red for Forward / Channel A, Blue for Reverse / Channel B).

## How to Run the Simulation
1. Extract the provided ZIP file containing the Wokwi project.
2. Open the project in Wokwi (or VSCode using the Wokwi extension).
3. Start the simulation. 

## Expected Behavior
* **Center (Deadzone):** When the potentiometer is near the middle (~50%), both the PWM LEDs and the RGB ring will be off. The motor is "stopped."
* **Forward (Turn Right):** Turning the potentiometer to the right increases the duty cycle on Channel A (Red LED gets brighter). The RGB ring fills with **Red** pixels proportional to the speed.
* **Reverse (Turn Left):** Turning the potentiometer to the left increases the duty cycle on Channel B (Blue LED gets brighter). The RGB ring fills with **Blue** pixels proportional to the speed.

## Logic Analyzer Testing
A Logic Analyzer is wired to GPIO 16 (D1) and GPIO 17 (D0). 
1. While the simulation runs, adjust the potentiometer.
2. Stop the simulation to generate the `.vcd` file.
3. Open the `.vcd` file in VSCode using the **Vaporview** extension to verify the PWM square waves and varying duty cycles.