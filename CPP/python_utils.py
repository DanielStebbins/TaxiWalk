import math
def binary_to_decimal(binary_string):
    return int(binary_string, 2)

def mu_taxi(n, taxi):
    return math.e ** (math.log(taxi) / n)

def constant(n, taxi):
    return mu_taxi(n, taxi) ** 4 - 1

i = 30
binary_string = "1000001010000010101000"
taxi = binary_to_decimal(binary_string)
print(f"I={i}, taxi={taxi}")
print(f"mu < {mu_taxi(i, taxi)}")
print(f"lambda < {constant(i, taxi)}")
