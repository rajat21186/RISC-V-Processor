import sys
sys.path.append('../')
def Read_Input(): # Reads input commands in assembly language from "Input.txt"
    try:
        file = open("Input/Input.txt", "r")
        Lines = file.readlines()
        Instructions = []
        for i in Lines:
            words = i.strip().replace(',', ' ').split()
            Instructions.append(words)
    finally:
        file.close()
    return Instructions

def Write_Output(res): # Writes binary output in "Output.bin" 
    with open('Assembler/Output/Output.bin', 'wb') as binary_file:
        for inst in res:
                binary_data = int(inst, 2).to_bytes(4, byteorder='little')
                binary_file.write(binary_data)


def Create_Registers(n): # Initializes 32 registers as per RISC V convention
    if n == 0:
        return [""]
    temp_list = Create_Registers(n - 1)
    Reg = []
    for i in temp_list:
        Reg.append(i + "0")
        Reg.append(i + "1")
    return Reg

def Decimal_Binary(num, bits): # Converts immediate values in decimal to 2's complement binary format
    sign = 0

    if num < 0:
        sign = 1 
        num = abs(num)

    res = ""
    
    while num > 0:
        mod = num % 2
        if mod == 1:
            res = "1" + res
        else:
            res = "0" + res
        num //= 2

    if bits > len(res):
        diff = bits - len(res)
        res = "0" * diff + res
    
    if sign == 1:
        reversed_res = "".join(["1" if bit == "0" else "0" for bit in res])
        
        carry = 1
        result = ""
        
        for bit in reversed(reversed_res):
            if bit == "0" and carry == 1:
                result = "1" + result
                carry = 0
            elif bit == "1" and carry == 1:
                result = "0" + result
                carry = 1
            else:
                result = bit + result
        
        res = result
    
    return res[::-1]
        

def Decimal_Binary_Register(num, bits): # Converts register numbers in decimal to binary format
    res = ""
    while num > 0:
        mod = num % 2
        if mod == 1:
            res = "1" + res
        else:
            res = "0" + res
        num //= 2

    if bits > len(res):
        diff = bits - len(res)
        res = "0" * diff + res
    
    return res[::-1]

    
U =  {"LUI", "AUIPC"}
UJ = {"JAL"}
S =  {"SW","SB","SH"}
SB = {"BEQ","BNE","BLT","BGE", "BLTU", "BGEU"}
I =  {"JALR", "LB", "LH", "LW", "LBU", "LHU", "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI"}
R  = {"ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND"}
Z =  {"LOADNOC","STORENOC"}   # add storenoc and loadnoc (any opcode value will work)
Y =  {"SIMD_ADD", "SIMD_SUB"} # SIMD instructions added

Registers = Create_Registers(5)

Template = {                                  # This is the template we use for processing all 37 commands
    "LUI"   :   "0" * 25 + "0110111" ,
    "AUIPC" :   "0" * 25 + "0010111" ,
    
    "JAL"   :   "0" * 25 + "1101111" ,

    "JALR"  :   "0" * 17 + "000" + "0"*5 + "1100111",
    
    "BEQ"   :   "0" * 17 + "000" + "0"*5 + "1100011",
    "BNE"   :   "0" * 17 + "001" + "0"*5 + "1100011",
    "BLT"   :   "0" * 17 + "100" + "0"*5 + "1100011",
    "BGE"   :   "0" * 17 + "101" + "0"*5 + "1100011",
    "BLTU"  :   "0" * 17 + "110" + "0"*5 + "1100011",
    "BGEU"  :   "0" * 17 + "111" + "0"*5 + "1100011",
    
    "LB"   :   "0" * 17 + "000" + "0"*5 + "0000011",
    "LH"   :   "0" * 17 + "001" + "0"*5 + "0000011",
    "LW"   :   "0" * 17 + "010" + "0"*5 + "0000011",
    "LBU"   :  "0" * 17 + "100" + "0"*5 + "0000011",
    "LHU"   :  "0" * 17 + "101" + "0"*5 + "0000011",
    
    "SB"   :   "0" * 17 + "000" + "0"*5 + "0100011",
    "SH"   :   "0" * 17 + "001" + "0"*5 + "0100011",
    "SW"   :   "0" * 17 + "010" + "0"*5 + "0100011",

    "ADDI"   :   "0" * 17 + "000" + "0"*5 + "0010011",
    "SLTI"   :   "0" * 17 + "010" + "0"*5 + "0010011",
    "SLTIU"  :   "0" * 17 + "011" + "0"*5 + "0010011",
    "XORI"   :   "0" * 17 + "100" + "0"*5 + "0010011",
    "ORI"    :   "0" * 17 + "110" + "0"*5 + "0010011",
    "ANDI"   :   "0" * 17 + "111" + "0"*5 + "0010011",

    "SLLI"   :   "0000000" + "0"*10 + "001" + "0"*5 +  "0010011",
    "SRLI"   :   "0000000" + "0"*10 + "101" + "0"*5 +  "0010011",
    "SRAI"   :   "0100000" + "0"*10 + "101" + "0"*5 +  "0010011",
    
    "ADD"    :   "0000000" + "0"*10 + "000" + "0"*5 + "0110011",
    "SUB"    :   "0100000" + "0"*10 + "000" + "0"*5 + "0110011", 
    "SLL"    :   "0000000" + "0"*10 + "001" + "0"*5 + "0110011",
    "SLT"    :   "0000000" + "0"*10 + "010" + "0"*5 + "0110011",
    "SLTU"   :   "0000000" + "0"*10 + "011" + "0"*5 + "0110011", 
    "XOR"    :   "0000000" + "0"*10 + "100" + "0"*5 + "0110011",
    "SRL"    :   "0000000" + "0"*10 + "101" + "0"*5 + "0110011",
    "SRA"    :   "0100000" + "0"*10 + "101" + "0"*5 + "0110011", 
    "OR"     :   "0000000" + "0"*10 + "110" + "0"*5 + "0110011",
    "AND"    :   "0000000" + "0"*10 + "111" + "0"*5 + "0110011",

    "LOADNOC"   :   "0" * 17 + "000" + "0"*5 + "0000111",   # 2 EXTRA instructions implemented 
    "STORENOC"  :   "0" * 17 + "001" + "0"*5 + "0000111",  # note the opcode chosen (template similar to S-type)

    "SIMD_ADD"    :   "0000000" + "0"*10 + "000" + "0"*5 + "1010101", # 7 bit opcode for SIMD
    "SIMD_SUB"    :   "0100000" + "0"*10 + "000" + "0"*5 + "1111111",
}

for Inst in Template:
    Template[Inst] = Template[Inst][::-1]

Instructions = Read_Input()

Binary_Instructions = []

for Inst in Instructions:
    type = Inst[0].upper()           #This gives the type of instruction

    res = Template[type]
    if type in R:

        rd  = int(Inst[1][1:])      #Reads the integer in rs1, rs2, etc. to determine register number
        rs1 = int(Inst[2][1:])
        rs2 = int(Inst[3][1:])    

        res = res[0:7]  + Decimal_Binary_Register(rd, 5) + res[12:32]
        res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]
        res = res[0:20] + Decimal_Binary_Register(rs2, 5) + res[25:32]

    elif type in I:
        rd  = int(Inst[1][1:])      
        rs1 = int(Inst[2][1:])


        res = res[0:7]  + Decimal_Binary_Register(rd, 5) + res[12:32]
        res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]

        if(type == "SLLI" or type == "SRLI" or type == "SRAI"):
            shamt = int(Inst[3])
            res = res[0:20]  + Decimal_Binary(shamt, 5) + res[25:32]
        else:
            imm = int(Inst[3])
            res = res[0:20]  + Decimal_Binary(imm, 12)


    elif type in S:
        rs1 = int(Inst[1][1:])
        rs2 = int(Inst[2][1:])
        imm = Decimal_Binary(int(Inst[3]), 12)

        res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]
        res = res[0:20] + Decimal_Binary_Register(rs2, 5) + res[25:32]
        res = res[0:7]  + imm[0:5] + res[12:32]
        res = res[0:25] + imm[5:12] 
    

    elif type in SB:
        rs1 = int(Inst[1][1:]) 
        rs2 = int(Inst[2][1:])
        imm = Decimal_Binary(int(Inst[3]), 13)

        res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]
        res = res[0:20] + Decimal_Binary_Register(rs2, 5) + res[25:32] 
        res = res[0:7]  + imm[11] + imm[1:5] + res[12:32]
        res = res[0:25] + imm[5:11] + imm[12]

    elif type in U:             
        rd  = int(Inst[1][1:])   
        imm = Decimal_Binary(int(Inst[2]), 20)

        res = res[0:7]  + Decimal_Binary_Register(rd, 5) + res[12:32]
        res = res[0:12] + imm

    elif type in UJ: 
        rd  = int(Inst[1][1:])   
        imm = Decimal_Binary(int(Inst[2]), 21)
        
        res = res[0:7]  + Decimal_Binary_Register(rd, 5) + res[12:32]
        res = res[0:12] + imm[12:20] + imm[11] + imm[1:11] + imm[20]
    
    elif type in Z:  # for LOADNOC AND STORENOC
        rs1 = int(Inst[1][1:])
        rs2 = int(Inst[2][1:])
        imm = Decimal_Binary(int(Inst[3]), 12)

        res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]
        res = res[0:20] + Decimal_Binary_Register(rs2, 5) + res[25:32]
        res = res[0:7]  + imm[0:5] + res[12:32]
        res = res[0:25] + imm[5:12]
    
    elif type in Y:

            rd  = int(Inst[1][1:])      #Reads the integer in rs1, rs2, etc. to determine register number
            rs1 = int(Inst[2][1:])
            rs2 = int(Inst[3][1:])    

            res = res[0:7]  + Decimal_Binary_Register(rd, 5) + res[12:32]
            res = res[0:15] + Decimal_Binary_Register(rs1, 5) + res[20:32]
            res = res[0:20] + Decimal_Binary_Register(rs2, 5) + res[25:32]


    res = res[::-1]
    Binary_Instructions.append(res)
       
Write_Output(Binary_Instructions)
