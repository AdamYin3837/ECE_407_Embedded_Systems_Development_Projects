# Lab 1: Introduction to Pico and GDB

Uzair Tariq, Adam Yin

## 1. Introduction
The objective of this laboratory exercise was to establish a command-line development environment for the Raspberry Pi Pico (RP2040). This involved installing the necessary build tools (CMake, GCC ARM toolchain), configuring the Pico SDK, building the `picotool`, and setting up the OpenOCD/GDB debugging workflow using a second Pico as a debug probe.

## 2. Development Environment Setup
We performed this lab and setup on two macOS machines:
*   **Machine A:** MacBook Pro (M3 Pro) - *Uzair Tariq*
*   **Machine B:** MacBook Air (Apple Silicon) - *Adam Yin*

### 2.1 Toolchain Installation
We followed the standard installation procedure using Homebrew to install `cmake`, `arm-none-eabi-gcc`, and `libusb`. We then cloned the `pico-sdk` and `picotool` repositories.
We made a small deviation from the lab manual at this stage. Rather than cloning these two repositories directly within our own repository, we instead initialized them (along with the debugprobe repository) as git submodules.
This technique ensures that updates to these upstream projects are more easily incorporated into our repository, and keeps the changes/code separate from the upstream.

**Observations:**
*   **Machine A:** The installation process for the toolchain, SDK, and `picotool` proceeded without errors. The environment variables were set successfully, and the toolchain was immediately verified.
*   **Machine B:** Several issues were encountered during the build process, specifically regarding `picotool` and `openocd`.

### 2.2 Troubleshooting for Machine B
During the installation on Machine B, we encountered and resolved the following issues:

1.  **Capstone Header Error:**
    *   *Error:* During the compilation of OpenOCD, the build failed with a fatal error: `'capstone/capstone.h' file not found` in `src/target/arm_disassembler.c`.
    *   *Resolution:* This matches the known issue documented in the lab guidelines. We located the source files causing the error and modified the include directive to point to the correct header location (or ensured the include path was correct).

2.  **Permission Errors:**
    *   *Error:* We encountered permission denied errors when attempting to compile OpenOCD.
    *   *Resolution:* We cleaned the build directory using `make clean`. We then used `chown` to take ownership of the directory files to ensure the user had write access, and re-ran the compilation process. This resulted in a successful build.

## 3. GDB and Blink Implementation

### 3.1 Building Blink
We created a `Lab1/blink` directory within our `Development_Group0x05` repository, adding the provided files: `blink.c`, `CMakeLists.txt`, and `pico_sdk_import.cmake`.

*   We generated the build files using `cmake -DCMAKE_BUILD_TYPE=Debug ..` to ensure debug symbols were included.
*   The compilation using `make` was successful on the first attempt on both machines.

### 3.2 Debugging Session
We connected the target Pico to the Debug Probe Pico and initiated the OpenOCD server.

*   **Connection:** GDB (`arm-none-eabi-gdb`) successfully connected to the OpenOCD server via `target remote localhost:3333`.
*   **Loading:** The `blink.elf` binary was successfully loaded onto the target.
*   **Execution:**
    1.  We executed `monitor reset init` to reset the target.
    2.  A breakpoint was set at the main function (`b main`).
    3.  Upon typing `continue`, the program ran and halted at the breakpoint as expected.
    4.  We verified the physical output: **The LED on the Pico was observed blinking.**
 
<img width="617" height="667" alt="image" src="https://github.com/user-attachments/assets/eccb228d-0907-4079-b7b0-d5b3cb013268" />

#### Figure 1. Screenshot of the GDB debugging session connected to the target Pico via OpenOCD, demonstrating successful program loading and breakpoint execution.

## 4. Conclusion
Our group, Group 0x05, successfully established the development environment for the Raspberry Pi Pico on macOS. While the M3 Pro installation was seamless, the MacBook Air required troubleshooting regarding header paths and file permissions for OpenOCD. Both machines are now fully configured to build, flash, and debug RP2040 applications using the command line and GDB.
