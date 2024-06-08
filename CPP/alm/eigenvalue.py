import numpy as np
from numpy import linalg
import time


start = time.time()

with open("alm\\m20n38.txt") as file:
    lines = file.readlines()

temp = []
for line in lines:
    temp.append(list(map(int, line.strip().split())))
A = np.array(temp)

eigs = linalg.eigvals(A)
max_eig = 0
for eig in eigs:
    if np.real(eig) > max_eig:
        max_eig = np.real(eig)
print(max_eig)

print(f"Running time: {time.time() - start} seconds.")