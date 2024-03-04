

import json
import csv

# # Load data from JSON file
# with open("PFDdelay1.txt") as json_file:
#     data = json.load(json_file)["tests"]

# # Loop through the tests and create CSV files
# for i, test in enumerate(data):
#     filename = f"test_{i+1}.csv"
#     with open(filename, 'w', newline='') as csvfile:
#         writer = csv.writer(csvfile)
#         writer.writerow(["Value"])
#         for value in test:
#             writer.writerow([value])

# print("CSV files created successfully.")

import json
import csv

# Load data from JSON file
with open("PFDdelay1.txt") as json_file:
    data = json.load(json_file)["tests"]

# Create a CSV file to write the data
filename = "output.csv"
with open(filename, 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    for test in data:
        for value in test:
            writer.writerow([value])

print("CSV file created successfully.")

