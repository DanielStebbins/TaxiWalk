def test_submultiplicative(s):
    flag = False
    for i in range(1, len(s)-1):
        for j in range(1, len(s) - i):
            if s[i] * s[j] < s[i + j]:
                flag = True
                print(f"{s[i]} * {s[j]} < {s[i + j]}")
    if flag == False:
        print("The sequence is submultiplicative (for the given elements)!")

def test_supermultiplicative(s):
    flag = False
    for i in range(1, len(s)-1):
        for j in range(1, len(s) - i):
            if s[i] * s[j] > s[i + j]:
                flag = True
                print(f"{s[i]} * {s[j]} > {s[i + j]}")
    if flag == False:
        print("The sequence is supermultiplicative (for the given elements)!")

# Ratio < 1 implies not submultiplicative.
def lowest_ratios(s, offset=1, n_multiplier=1):
    for i in range(len(s)-offset//n_multiplier):
        min_ratio = float("inf")
        min_j = float("inf")
        for j in range(len(s) - i - offset//n_multiplier):
            ratio = s[i] * s[j] / s[i + j + offset//n_multiplier]
            if ratio < min_ratio:
                min_ratio = ratio
                min_j = j
        print(f"i={i * n_multiplier + offset}: worst j={min_j * n_multiplier + offset}, with ratio {min_ratio} vs {i * n_multiplier + offset + min_j * n_multiplier + offset}")
        # print(f"{ratio}")

def highest_ratios(s, offset=1, n_multiplier=1):
    for i in range(len(s)-offset//n_multiplier):
        max_ratio = 0
        max_j = 0
        for j in range(len(s) - i - offset//n_multiplier):
            ratio = s[i] * s[j] / s[i + j + offset//n_multiplier]
            if ratio > max_ratio:
                max_ratio = ratio
                max_j = j
        print(f"i={i * n_multiplier + offset}: worst j={max_j * n_multiplier + offset}, with ratio {max_ratio} vs {i * n_multiplier + max_j * n_multiplier + 2 * offset}")

# Prints a table of s_i * s_j / s_{i+j}
def ratio_table(s, offset=1, n_multiplier=1):
    with open("ratio_table.csv", "w") as file:
        file.write("," + ",".join(list(map(str, range(1, len(s))))) + "\n")
        for i in range(len(s)-offset//n_multiplier):
            out = [str(i+1)]
            for j in range(len(s) - i - offset//n_multiplier):
                out.append(str(round(s[i] * s[j] / s[i + j + offset//n_multiplier], 5)))
            file.write(",".join(out) + "\n")
        file.flush()
        file.close()
        

def bounds(s, offset=1, n_multiplier=1):
    for n in range(len(s)):
        # print(f"{n_multiplier * n}: {round((s[n] ** (1/(n_multiplier * n))) ** 4 - 1, 4)}")
        print(round((s[n] ** (1/(n * n_multiplier + offset))) ** 4 - 1, 4))
        print("\n" * (n_multiplier - 2))


# Any of the 4 cardinal directions equal.
# s = [1, 2, 4, 6, 10, 16, 22, 38, 56, 82, 130, 188, 288, 420, 676, 942, 1496, 2094, 3576, 4844, 8226, 11094, 20160, 26140, 44694, 60398, 113032, 143800, 264054, 339658, 650678, 819138, 1488128, 1932820, 3810506, 4691186, 8920932, 11254046, 22262610, 27422366, 51866584, 65615248, 132266318, 160593806, 311968438, 388031466]
# s_odd = [1, 2, 6, 16, 38, 82, 188, 420, 942, 2094, 4844, 11094, 26140, 60398, 143800, 339658, 819138, 1932820, 4691186, 11254046, 27422366, 65615248, 160593806, 388031466]
s_even = [4, 10, 22, 56, 130, 288, 676, 1496, 3576, 8226, 20160, 44694, 113032, 264054, 650678, 1488128, 3810506, 8920932, 22262610, 51866584, 132266318, 311968438, 783580732, 1839049714]
s_mul_4 = [10, 56, 288, 1496, 8226, 44694, 264054, 1488128, 8920932, 51866584, 311968438, 1839049714]

c = [2, 4, 6, 10, 16, 26, 42, 68, 110, 178, 288, 460, 740, 1192, 1918, 3064, 4910, 7872, 12620, 20114, 32150, 51396, 82160, 130730, 208506, 332616, 530588, 843222, 1342662, 2138280, 3405346, 5406522, 8597632, 13674278, 21748530, 34501460, 54807754, 87077354, 138346766, 219324398, 348109128, 552582790, 877163942, 1389806294, 2204289314, 3496483316, 5546212122, 8783360626, 13922238632, 22069957494, 34986181158, 55383388278, 87740467384, 139014623272, 220254102104, 348536652664, 551914140382, 874039817792, 1384184997874, 2189670407434]
p = [6, 16, 90, 480, 2548, 13696, 74052, 402800, 2205148, 12146352, 67290626, 374798032, 2097959250]

c_start_straight = [1, 999, 2, 4, 6, 10, 16, 26, 42, 68, 110, 178, 284, 458, 736, 1186, 1892, 3036, 4864, 7802, 12430, 19876, 31764, 50794, 80794, 128910, 205592, 328044, 521182, 830120, 1321794, 2105468, 3341944]
c_ss_odd = [99, 4, 10, 26, 68, 178, 458, 1186, 3036, 7802, 19876, 50794, 128910, 328044, 830120, 2105468]
c_ss_even = [1, 2, 6, 16, 42, 110, 284, 736, 1892, 4864, 12430, 31764, 80794, 205592, 521182, 1321794, 3341944]

# L-R, U-D parity pairs.
# Straight start variation provides a 5.1 bound, regular provdes 5.3659 at N=40, which is not true.
parity_even = [1, 2, 6, 14, 36, 92, 232, 602, 1528, 3942, 9998, 25668, 64966, 165998, 419318, 1067134, 2690706, 6825636, 17182920, 43475306, 109296314]
parity_even_ss = [2, 4, 10, 24, 60, 148, 378, 954, 2446, 6198, 15860, 40160, 102448, 258986, 658504, 1661616, 4212660, 10611828, 26838528, 67507852, 170387398, 428044368, 1078524630]

# Any 2 parities equal:
any_parity_even_ss = [1, 2, 6, 16, 42, 110, 284, 736, 1892, 4864, 12430, 31764, 80794, 205592, 521182, 1321794, 3341944, 8453328, 21327558, 53832650, 135583622]

# Given +N steps can reach the origin:
origin = [0, 0, 0, 106, 1402, 12734, 99600, 713958, 4868292, 32246280, 209839580]

s = s_mul_4
# test_submultiplicative(s)
# test_supermultiplicative(s)
# lowest_ratios(s)
# highest_ratios(s, 12, 4)
ratio_table(s)
# bounds(s, 2, 2)