# Smooth Encoder Implementation Details

The rotary encoder implementation in this project feels "smooth" and responsive because it combines **Hardware Interrupts** with a **State-Table Lookup algorithm**. This approach eliminates the lag associated with polling and the "jitter" associated with mechanical contact bouncing.

## 1. The Core Logic: State-Table Lookup

Mechanical encoders generate "Quadrature" signals (Gray Code). As you turn the knob, pins A and B change states in a specific pattern.

Instead of writing complex `if/else` logic to check "If A went high while B was low...", we use a **Lookup Table**.

### The Concept
We combine the **Previous State (2 bits)** and the **Current State (2 bits)** into a 4-bit index (0-15). This index tells us exactly what happened.

```c
// Construct a 4-bit index: [Old A][Old B] [New A][New B]
uint8_t index = (last_ab_state << 2) | current_ab;
int8_t delta = encoder_transition_table[index];
```

### The Magic Table
The array below maps every possible implementation of pin changes to a movement value (-1, 0, or +1).

```c
static const int8_t encoder_transition_table[16] = {
    0,  -1, 1,  0,   // 00->00, 00->01, 00->10, 00->11(invalid)
    1,  0,  0,  -1,  // 01->00, 01->01, 01->10(invalid), 01->11
    -1, 0,  0,  1,   // 10->00, 10->01(invalid), 10->10, 10->11
    0,  1, -1,  0    // 11->00(invalid), 11->01, 11->10, 11->11
};
```

**Why this makes it smooth:**
1.  **Validity Check:** Valid turns only change 1 bit at a time (e.g., `00` -> `01`). If both bits change at once (`00` -> `11`), it is a mechanical error or extreme bounce. The table returns `0`, effectively filtering out noise without any expensive math.
2.  **No "Dead Spots":** Every valid tick is registered immediately.

## 2. Hardware Interrupts (IRQ)

Instead of the CPU constantly asking "Did the pin change?" (Polling), we configure the RP2040 hardware to tap the CPU on the shoulder only when a change occurs.

*   **Mechanism:** `gpio_set_irq_enabled_with_callback` fires on `EDGE_RISE` or `EDGE_FALL`.
*   **Speed:** The transition logic happens in microseconds inside the Interrupt Service Routine (ISR).
*   **Result:** Even if the main program is busy drawing to the screen or calculating math, the encoder turns are captured in the background.

## 3. Data Integrity (`volatile`)

Because variables are shared between the Interrupt (background) and the Main Loop (foreground), we use the `volatile` keyword.

```c
volatile int encoder_count = 0;
volatile bool encoder_changed = false;
```

 This forces the C compiler to always read the actual memory address rather than caching the value in a CPU register, ensuring the main loop never works with stale data.

## 4. Button Debouncing

Unlike the rotation (which uses the state table for filtering), the Push Button is a simple "make/break" contact that bounces physically.

We handle this using **Time-based Debouncing** inside the ISR:

```c
if (current_time - last_button_interrupt_time < BUTTON_DEBOUNCE_TIME_MS) {
    return; // Ignore this interrupt, it's just noise
}
```

This simple check (50ms) ensures that one physical press registers as exactly one software event.