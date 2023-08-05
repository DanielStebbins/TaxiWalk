import math
def binary_to_decimal(binary_string):
    return int(binary_string, 2)

def mu_taxi(n, taxi):
    return math.e ** (math.log(taxi) / n)

def constant(n, taxi):
    return mu_taxi(n, taxi) ** 4 - 1

i = 400
binary_string = "1000011010001010010010111011011001010101001010000011110010100000001101000010011000000110000011101001100001011000110010001101110000110001000001001100001010001101001100111111011001001100100101110101011111100110011100000111110001000111011011101100001000001000010001101110"
taxi = binary_to_decimal(binary_string)
print(f"I={i}, taxi={taxi}")
print(f"mu < {mu_taxi(i, taxi)}")
print(f"lambda < {constant(i, taxi)}")
