# import numpy as np
# import json
# A = json.load(open("PFDdelay1.txt"))["tests"]
# i = 0
# for a in A:
#     b = np.array(a).T
#     np.savetxt("test" + str(i) + ".csv", b)

import json
import csv

# Load data from JSON file
with open("PFDdelay1.txt") as json_file:
    data = json.load(json_file)["tests"]

# Loop through the tests and create CSV files
for i, test in enumerate(data):
    filename = f"test_{i+1}.csv"
    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Value"])
        for value in test:
            writer.writerow([value])

print("CSV files created successfully.")
