in_file = open("stepsToOrigin.txt", "r")
nums = in_file.read().split()
print(len(nums))
flipped = []
offsets = [0, 2, 1, 3]
for offset in offsets:
    for x in range(-100, 101):
        for y in range(-100, 101):
            flipped.append(nums[offset * 40401 + (x + 100) * 201 + y + 100])

out_file = open("StepsToOriginFlipped.txt", "w")
out_file.write(" ".join(flipped))

in_file.close()
out_file.close()