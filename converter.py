import json
import csv

# List of input JSON files
input_files = ["PFDdelay1.txt", "PFDdelay2.txt", "PFDdelay3.txt", "PFDdelay4.txt"]

for input_file in input_files:
    # Load data from JSON file
    with open(input_file) as json_file:
        data = json.load(json_file)["tests"]

    # Create a CSV file to write the data
    output_file = input_file.replace(".txt", ".csv")
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        for test in data:
            for value in test:
                writer.writerow([value])

    print(f"CSV file {output_file} created successfully.")
