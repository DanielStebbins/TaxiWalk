import numpy as np
from numpy import linalg

with open("alm\\out.txt") as file:
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