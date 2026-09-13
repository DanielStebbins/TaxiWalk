import math
def binary_to_decimal(binary_string):
    return int(binary_string, 2)

def mu_taxi(n, taxi):
    return math.e ** (math.log(taxi) / n)

def constant(n, taxi):
    return mu_taxi(n, taxi) ** 4 - 1

i = 300
binary_string = "1001001001000110011110000100100110010100111111010111101010110101001111111010110011110011111111010110010100101101101000000101110001001011111101110110011100111010011000010011001111100101011011000000000"
taxi = binary_to_decimal(binary_string)
# taxi = 2189670407434
print(f"I={i}, taxi={taxi}")
print(f"mu > {mu_taxi(i, taxi)}")
print(f"lambda > {constant(i, taxi)}")


# with open("StepsToOriginFlipped.txt") as o:
#     o = o.readline().strip().split(" ")
#     with open("StepsToNarrowAtOrigin.txt") as n:
#         n = n.readline().strip().split(" ")
#         for a in range(0, 4):
#             for x in range(-100, 101):
#                 for y in range(-100, 101):
#                     if int(o[a * 40401 + (x + 100) * 201 + y + 100]) < int(n[a * 40401 + (x + 100) * 201 + y + 100]):
#                         print(f"{int(o[a * 40401 + (x + 100) * 201 + y + 100])} < {int(n[a * 40401 + (x + 100) * 201 + y + 100])} for {a}:({x}, {y})")


