import math

NUM_TAPS = 101
FS = 48000.0
FC1 = 400.0  # 下限頻率
FC2 = 600.0  # 上限頻率
M = NUM_TAPS - 1

taps = []

for i in range(NUM_TAPS):
    n = i - (M / 2.0)
    
    # Hamming Window 函數
    window = 0.54 - 0.46 * math.cos((2.0 * math.pi * i) / M)
    
    if n == 0.0:
        # 處理正中央的 Tap (避免除以零)
        h = 2.0 * ((FC2 - FC1) / FS)
    else:
        # Windowed-Sinc 帶通濾波器公式
        h = (math.sin(2.0 * math.pi * (FC2/FS) * n) - math.sin(2.0 * math.pi * (FC1/FS) * n)) / (math.pi * n)
        
    taps.append(h * window)

# 輸出成漂亮的 C 語言陣列格式
print(f"#define NUM_TAPS {NUM_TAPS}\n")
print(f"const float filter_taps[NUM_TAPS] = {{")
for i in range(0, NUM_TAPS, 5):
    row = taps[i:i+5]
    # 將每個浮點數格式化為帶有 f 後綴的字串
    formatted_row = ", ".join([f"{x: .10f}f" for x in row])
    print(f"    {formatted_row},")
print("};")