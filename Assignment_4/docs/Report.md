# Assignment 4: Digital Audio Input via PIO Extended

**Group Name/Number:** Group 0x05 
**Team Members:** Uzair Tariq, Adam Yin 
**Date:** April 4, 2026  

---

## 1. Purpose
The goal of this assignment is to extend the I2S audio acquisition system from Assignment 3 by implementing a **Finite Impulse Response (FIR) Bandpass Filter**. The system is designed to specifically detect **500 Hz tones** while rejecting out-of-band noise and higher-frequency interference (specifically a 1000 Hz tone).

## 2. Introduction
In embedded signal processing, detecting a specific frequency requires a balance between frequency selectivity and computational resources. We implemented a Windowed-Sinc FIR filter on the RP2040 microcontroller. By processing the digital I2S stream from an INMP441 microphone, the system calculates the "energy" of the filtered signal to determine if a 500 Hz tone is present in real-time.

## 3. Assignment Specific Details

### 3.1 Filter Design & Specification
We designed a Bandpass FIR filter with the following parameters:
* **Sampling Frequency ($f_s$):** 48,000 Hz
* **Target Center Frequency:** 500 Hz
* **Passband:** 400 Hz – 600 Hz
* **Window Type:** Hamming Window (to reduce side-lobe leakage)
* **Nyquist Limit:** 24,000 Hz (Our target of 500 Hz is well below the half-Nyquist limit, ensuring no aliasing issues).

### 3.2 Optimization & Iteration Process
The implementation involved several iterations to find the "Sweet Spot" for embedded performance:

* **Iteration 1 (51 Taps):** * **Result:** The filter transition band was too wide. 
    * **Observation:** Testing with a 1000 Hz tone produced higher energy readings than the 500 Hz target due to leakage. It was impossible to set a reliable threshold.
* **Iteration 2 (151 Taps):** * **Result:** The filter was extremely narrow and precise. 
    * **Observation:** While selectivity was high, the filter became "too sharp." Any slight frequency drift from the playback source caused the signal to be rejected.
* **Final Solution (101 Taps):**
    * **Result:** Optimal balance. 
    * **Observation:** This configuration successfully suppressed 1000 Hz interference while remaining robust enough to consistently detect the 500 Hz input.

### 3.3 Tone Detection Logic
To detect the tone, we implemented an **Energy Accumulation** method:
1.  **DC Offset Removal:** Real-time calculation of the mean to center the signal at zero.
2.  **FIR Convolution:** Samples are passed through the 101-tap array using a circular buffer.
3.  **Energy Calculation:** Filtered samples are squared and averaged over the buffer.
4.  **Signal Scaling:** To handle the small floating-point values inherent in high-tap filters, a **100.0f scaling factor** was applied to the energy result for stable thresholding.
5.  **Thresholding:** A final `DETECT_THRESHOLD` of **0.00030** was set based on empirical testing of the 500Hz vs 1000Hz energy gap.

### 3.4 Trade-offs in Embedded Signal Processing
Following the learning objectives of the assignment, we considered the following trade-offs:

* **Selectivity vs. Computational Latency:** Increasing the number of Taps from 51 to 101 improved the "steepness" of the filter but doubled the number of multiply-accumulate (MAC) operations per sample. The RP2040 handled 101 Taps comfortably at 48kHz without dropping I2S frames.
* **Filter Length vs. Detection Delay:** A longer filter introduces a group delay ($M/2$ samples). At 101 samples and a 48kHz sample rate, the delay is approximately 2.1ms, which is negligible for real-time audio detection.
* **Bandwidth vs. Robustness:** A narrower bandwidth is better for picking out a tone in a noisy environment but requires a more stable source. Our 200 Hz bandwidth (400-600 Hz) provides a safety margin for slight variations in common playback devices.