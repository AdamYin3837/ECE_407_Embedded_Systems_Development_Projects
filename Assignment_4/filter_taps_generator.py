import numpy as np
from scipy.signal import firwin

fs = 48000
num_taps = 51
lowcut = 400.0
highcut = 600.0

# Generate bandpass FIR coefficients
taps = firwin(num_taps, [lowcut, highcut], pass_zero=False, fs=fs)

print(f"const float filter_taps[{num_taps}] = {{")
for i, tap in enumerate(taps):
    print(f"    {tap}f,", end="")
    if (i + 1) % 5 == 0: print()
print("\n};")