import matplotlib.pyplot as plt

# Read data from counters.txt
with open('logs/counters.txt', 'r') as file:
    lines = file.readlines()

# Extract register and memory instruction counts
register_instructions = int(lines[0].split(":")[1].strip())
memory_instructions = int(lines[1].split(":")[1].strip())

# Create a pie chart
labels = ['Register Instructions', 'Memory Instructions']
sizes = [register_instructions, memory_instructions]
colors = ['#ff9999', '#66b3ff']  # Red for register, Blue for memory
explode = (0.1, 0)  # explode the 1st slice (i.e., Register Instructions)

plt.pie(sizes, explode=explode, labels=labels, colors=colors, autopct='%1.1f%%', startangle=90)
plt.axis('equal')  # Equal aspect ratio ensures that the pie chart is circular.

# Show the pie chart
plt.title('RISC-V Instruction Distribution')
plt.savefig("Figure/r_m.pdf",format="pdf")
#plt.savefig("Figure/r_m.png",format="png")
plt.show()

cycles = []
def stalling(file_path):
    stalls = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

    for line in lines:
        if line.startswith("Cycle"):
            cycle_number = int(line.split()[1].strip(':'))
            cycles.append(cycle_number)
        elif line.startswith("Total number of stalls:"):
            stall_count = int(line.split(":")[1].strip())
            stalls.append(stall_count)

    plt.plot(cycles, stalls, 'o')
    plt.title('Stalls vs Cycles')
    plt.xlabel('Cycles')
    plt.ylabel('Stalls')
    plt.legend()
    #plt.grid(True)
    plt.savefig("Figure/s_c.pdf",format="pdf")
    plt.show()


def read_memory_patterns(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    instruction_memory = {}
    data_memory = {}
    current_memory_type = None
    current_cycle = None

    for line in lines:
        line = line.strip()

        if line.startswith("Cycle"):
            current_cycle = int(line.split()[1].strip(':'))
        elif line.startswith("Data Memory that was updated or accesed"):
            current_memory_type = "data"
        elif line.startswith("Using Instruction Memory"):
            current_memory_type = "instruction"
        elif line.startswith("Addr:"):
            addr = int(line.split(":")[1].strip())
            if current_cycle is not None:
                if current_memory_type == "instruction":
                    instruction_memory[current_cycle] = addr
                elif current_memory_type == "data":
                    data_memory[current_cycle] = addr

    return instruction_memory, data_memory


log_file_path = "logs/logfile.log"
stalling(log_file_path)
instruction_memory, data_memory = read_memory_patterns(log_file_path)

# Plot Instruction Memory vs Cycle
plt.figure(figsize=(10, 5))
plt.scatter(list(instruction_memory.keys()), list(instruction_memory.values()), label='Instruction Memory', marker='.')
plt.title('Instruction Memory vs Cycle')
plt.xlabel('Cycle')
plt.ylabel('Memory Address')
plt.legend()
plt.grid(True)
plt.savefig("Figure/i_c.pdf",format="pdf")
#plt.savefig("Figure/i_c.png",format="png")
plt.show()

# Plot Data Memory vs Cycle
plt.figure(figsize=(10, 5))
plt.scatter(list(data_memory.keys()), list(data_memory.values()), color='orange', label='Data Memory', marker='.')
plt.xlim(0, max(cycles))
plt.title('Data Memory vs Cycle')
plt.xlabel('Cycle')
plt.ylabel('Memory Address')
plt.legend()
plt.grid(True)
plt.savefig("Figure/d_c.pdf",format="pdf")
#plt.savefig("Figure/d_c.png",format="png")
plt.show()
