import sys
sys.path.append('../')
def read_binary_file(file_path):
    with open(file_path, 'rb') as binary_file:
        binary_data = binary_file.read()
        binary_strings = [format(int(binary_data[i:i+4][::-1].hex(), 16), '032b') for i in range(0, len(binary_data), 4)]
    return binary_strings

def compare_files(output_file_path, result_file_path):
    score = 0
    total = 0
    lines = []
    status = []
    remark = ""

    output = read_binary_file(output_file_path)

    with open(result_file_path, 'r') as result_file:
        result = result_file.read().splitlines()

    total = min(len(output), len(result))

    for i in range(total):        
        if result[i] == output[i]:
            score += 1
            remark = "Passed"
        else:
            remark = "Failed"
        lines.append(output[i])
        status.append(remark)
    return score, total, lines, status

input_file_path  = "Input/Input.txt"     # Input file which contains assembly instructions
output_file_path = "Assembler/Output/Output.bin"    # File generated from "Assembler.py"
result_file_path = "Input/binary.txt"    # File containing the correct conversion of "Input.txt" to binary, used to check "Output.bin"
evaluation_file_path = "Assembler/Result/Assembler_Evaluation.txt" #Text file storing the detailed evaluation

score, total, lines, status = compare_files(output_file_path, result_file_path)

with open(evaluation_file_path, 'w') as evaluation_file:
    evaluation_file.write(f"Test cases passed = {score}\n")
    evaluation_file.write(f"Test cases failed = {total - score}\n\n")
    evaluation_file.write("Status by line:\n")
    for i in range(total):
        evaluation_file.write(f"Test Case {i}: \t {lines[i].ljust(30)} - {status[i]}\n")

print(f"Test cases passed = {score}")
print(f"Test cases failed = {total - score}")
