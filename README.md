============
RISC-V CPU
============


How to Run?

a) Write the assembly test cases in Input/Input.txt and the corresponding binary output in Input/binary.txt.

b) To run this code, open the terminal in the repository directory and type the command make (for Linux environment).

c) The binary output will be generated in Assembler/Output/Output.bin.

d) The assembler evaluation results will be in Assembler/Result/Assembler_Evaluation.txt.

e) Logs generated from the simulator will be placed in logs/logfile.log.

f) Plot figures will be generated in the figure folder.

File Description:

=> There are 2 files for the assembler and 9 files for the simulator.

=> The assembler files are Assembler.py and Assembler_Evaluator.py.

=> The simulator files are ALU.cpp, LRU_Cache.cpp, LogFile.cpp, cache_bus.cpp, cpu.cpp, membus.cpp, memory.cpp, pipeline.cpp, and simulator.cpp.

Code Overview:

=> Assembler.py: Converts the assembly code in Input/Input.txt to binary code in Assembler/Output/Output.bin.

=> Assembler_Evaluator.py: Checks the output generated by Assembler.py against Input/binary.txt and stores the result in Assembler/Result/Assembler_Evaluation.txt.

=> ALU.cpp: Implements the ALU for the CPU.

=> LRU_Cache.cpp: Implements the Least Recently Used (LRU) cache policy for the CPU.

=> LogFile.cpp: Implements the log file writing component of the simulator.

=> cache_bus.cpp: Implements the cache bus for the CPU.

=> cpu.cpp: Implements the raw CPU.

=> membus.cpp: Implements the memory bus, which connects memory to the cache.

=> memory.cpp: Implements the memory.

=> pipeline.cpp: Contains the buffers and logic for implementing the cycle-accurate simulator.

=> simulator.cpp: The main file for the simulator.
