# Assignment 3 Report: I2S Digital Audio Acquisition

**Group Name/Number:** Group 0x05 
**Team Members:** Uzair Tariq, Adam Yin 

---

## Purpose
The primary objective of this assignment is to successfully acquire digital audio data from an INMP441 I2S omnidirectional microphone using the RP2040. This involves configuring the RP2040's Programmable I/O (PIO) to implement the I2S protocol, utilizing Direct Memory Access (DMA) for efficient data transfer, and properly decoding the raw digital signals into readable, centered decimal waveforms that accurately represent physical sound.

## Introduction
Unlike standard analog microphones that output simple voltage waves, the INMP441 transmits digital data using the I2S protocol. Reading this digital stream requires strict timing. To avoid overloading our main processor, we used the RP2040's PIO (Programmable I/O) hardware to manage the clock signals and read the incoming audio bits. We combined this with DMA to continuously collect the data in the background. The main challenge of this assignment was not just wiring the hardware, but decoding this raw stream of background data into readable, accurate audio waveforms.

## Assignment Specific Details

### 1. Data Parsing: The Hexadecimal to Decimal Challenge
Our initial successful data acquisition resulted in a continuous stream of raw hexadecimal values printing to the serial monitor. While this confirmed that our PIO state machine and hardware wiring were functional, the raw data was unreadable as an audio waveform.

The challenge stemmed from a mismatch between the sensor's output resolution and the microcontroller's memory architecture:
* The INMP441 microphone outputs **24-bit** signed audio samples.
* The RP2040's DMA and PIO FIFO read data in standard **32-bit** words.

To fix this, we implemented a bit-wise arithmetic shift (`raw_data >> 8`). This discarded the lower 8 bits of padding, properly sign-extending the 24-bit two's complement value into a standard 32-bit signed integer. This successfully transformed the chaotic hex stream into a readable decimal waveform that reacted proportionally to ambient noise and direct sound.

### 2. Channel Configuration and Hardware-Software Synchronization
During our testing, we encountered an issue where the sensor appeared unresponsive to voices (outputting `0`), but generated arbitrary numbers when the physical sensor was tapped. We discovered this was a synchronization issue between the hardware channel selection and our software array indexing.

Since we are only using a single sensor, we can only pull data from one channel slot in the I2S stereo frame. The INMP441 determines its channel based on the logic level of the `L/R` pin:
* **Left Channel:** `L/R` pin connected to GND.
* **Right Channel:** `L/R` pin connected to 3V3.

Once we realized our software was polling the right channel's indices, we switched the L/R pin wire from GND to 3V3. The issue got solved successfully.