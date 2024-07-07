#include "cpu.hpp"

//PC reset and initialisation
void CPU_reset(RISCV_cpu *cpu) {
    cpu->pc = RISCV_MEM_BASE_INSTR;                           // Set program counter to the base address
    for(int i=0;i<REG_LEN;i++){
        cpu->x[i]=0;
    }
    for(int i=0;i<MM_REG_LEN;i++){
        cpu->mem_map_reg[i]=0;
    }
    pipeline_reset(cpu->__pipe);
    cpu->__alu->flag_reset();
    cpu->register_instr_counter=0;
    cpu->memory_instr_counter=0;
    cpu->noc_type_instr=0;
    cpu->simd_type_instr=0;
}

// cpu initialization
RISCV_cpu* CPU_init() {
    RISCV_cpu *cpu= (RISCV_cpu*) malloc(sizeof(RISCV_cpu)); 
    if (cpu ==NULL ) {
        fprintf(stderr, "[-] ERROR-> CPU_init: malloc failed\n");
        exit(1);
    }

    cpu->__bus = cache_bus_init();
    if (cpu->__bus==NULL) {
        fprintf(stderr, "[-] ERROR-> cache_bus_init: malloc failed\n");
        exit(1);
    }
    cpu->__pipe = pipe_init();
    if (cpu->__pipe==NULL) {
        fprintf(stderr, "[-] ERROR-> pipe_init : malloc failed\n");
        exit(1);
    }
    cpu->__alu = alu_init();
    if (cpu->__alu==NULL) {
        fprintf(stderr, "[-] ERROR-> alu_init : malloc failed\n");
        exit(1);
    }
    cpu->register_instr_counter = 0;
    cpu->memory_instr_counter = 0;
    CPU_reset(cpu);
    return cpu;
}


//PC write function
void fetch_pc_update(RISCV_cpu*cpu,u32 pc_new){
    cpu->pc=pc_new;
}

//FETCH_STAGE
void cpu_fetch(RISCV_cpu *cpu,u32 new_pc,bool early_exit) {
    if(early_exit){
        fetch_pc_update(cpu,new_pc);
    }
    if(cpu->__pipe->fetch->done)return;
    fetch_pc_update(cpu,new_pc);
    cpu ->__pipe->fetch->inst = i_cache_ld(cpu->__bus->i_cache_bus, cpu->pc,32);
    cpu->__pipe->fetch->pcf=cpu->pc;
    cpu->__pipe->fetch->done=true;
}

//Pc adder
u32 pc_adder(u32 b,u32 a){
    return b+a;
}

//PC update function
u32 cpu_pc_update(RISCV_cpu*cpu){
    u32 return_pc=0;
    if(cpu->__pipe->isbranch){
        return_pc= pc_adder(cpu->__pipe->execute->pce,cpu->__pipe->newpc_offset);
    }
    else if(cpu->__pipe->isbranch2){
        if(cpu->__pipe->isjalr){
            return_pc= cpu->__pipe->newpc_offset2;
        }
        else{
            return_pc= pc_adder(cpu->__pipe->decode->pcd,cpu->__pipe->newpc_offset2);
        }
    }
    else
    return_pc= pc_adder(cpu->pc,4);
    cpu->__pipe->bypass->new_pc=return_pc;
    return return_pc;
}

//Helpr functions for decoding
ALWAYS_INLINE u32 rd(u32 instr__) {
    return (instr__ >> 7) & 0x1f;    // rd :: 11..7
}
ALWAYS_INLINE u32 rs1(u32 instr__) {
    return (instr__ >> 15) & 0x1f;   // rs1 :: 19..15
}
ALWAYS_INLINE u32 rs2(u32 instr__) {
    return (instr__ >> 20) & 0x1f;   // rs2 :: 24..20
}

ALWAYS_INLINE i32 imm_I_TYPE(u32 instr__) {
    // imm[11:0] :: inst[31:20]
    return ((i32) (instr__ & 0xfff00000)) >> 20; 
}
ALWAYS_INLINE i32 imm_S_TYPE(u32 instr__) {
    // imm[11:5] :: inst[31:25], imm[4:0] :: inst[11:7]
    return ((i32)(instr__ & 0xfe000000) >> 20)
        | ((instr__ >> 7) & 0x1f); 
}
ALWAYS_INLINE i32 imm_SB_TYPE(u32 instr__) {
    // imm[12|10:5|4:1|11] :: inst[31|30:25|11:8|7]
    return ((i32)(instr__ & 0x80000000) >> 19)
        | ((instr__ & 0x80) << 4) // imm[11]
        | ((instr__ >> 20) & 0x7e0) // imm[10:5]
        | ((instr__ >> 7) & 0x1e); // imm[4:1]
}

ALWAYS_INLINE i32 imm_U_TYPE(u32 instr__) {
    // imm[31:12] :: inst[31:12]
    return (i32)((instr__ & 0xfffff000) >> 12);
}
ALWAYS_INLINE i32 imm_UJ_TYPE(u32 instr__) {
    // imm[20|10:1|11|19:12] :: inst[31|30:21|20|19:12]
    return ((i32)(instr__ & 0x80000000) >> 11)
        | (instr__ & 0xff000) // imm[19:12]
        | ((instr__ >> 9) & 0x800) // imm[11]
        | ((instr__ >> 20) & 0x7fe); // imm[10:1]
}

ALWAYS_INLINE i32 shamt(u32 instr__) {
    // shamt(shift amount) [for immediate shift instructions]
    // shamt[4:5] :: imm[5:0]
    return (i32) (u32) (imm_I_TYPE(instr__) & 0x1f); //  0x1f / 0x3f ?
}

ALWAYS_INLINE u32 jalr_adder(u32 a, u32 b){
    return (a+b)&0xfffffffe;
}

//Decodes read registers
u32 cpu_read_reg(RISCV_cpu*cpu,u32 rs){
    return cpu->x[rs]; 
}

//control unit for decode stage
void cpu_control_unit(RISCV_cpu *cpu,pipeline *pipe,u32 inst) {
    pipe->decode->inst = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25
    switch (pipe->decode->inst){
        case RV32i_LUI:
            cpu->register_instr_counter++;
            pipe->decode->inst=1;
            pipe->decode->rd=rd(inst);
            pipe->decode->imm=(i32)(inst & 0xfffff000);
            pipe->decode->iswrite=true;
            // LUI_exe(cpu, inst);
            break;
        case RV32i_AUIPC:
            cpu->register_instr_counter++;
            pipe->decode->inst=2; 
            pipe->decode->rd=rd(inst);
            pipe->decode->imm= imm_U_TYPE(inst);
            pipe->decode->iswrite=true;
            // AUIPC_exe(cpu, inst); 
            break;

        case RV32i_JAL:
            cpu->register_instr_counter++;
            pipe->decode->inst=3; 
            pipe->isbranch2=true;
            pipe->newpc_offset2= imm_UJ_TYPE(inst);
            pipe->decode->rd=rd(inst);
            pipe->decode->iswrite=true;
            pipe->decode->imm=pipe->decode->pcd;
            // JAL_exe(cpu, inst);
            break;
        case RV32i_JALR:
            cpu->register_instr_counter++;
            pipe->decode->inst=4; 
            pipe->decode->rd=rd(inst);
            pipe->decode->rs1=rs1(inst);
            pipe->newpc_offset2=imm_I_TYPE(inst);
            pipe->newpc_offset2=jalr_adder(pipe->newpc_offset,cpu->x[pipe->decode->rs1]);//Adding jalr offset
            pipe->decode->iswrite=true;
            pipe->isbranch2=true;
            pipe->decode->imm=pipe->decode->pcd;
            pipe->isjalr=true;
            // JALR_exe(cpu, inst);
            break;

        case RV32i_SB_TYPE:
            cpu->register_instr_counter++;
            pipe->decode->isstype=true;
            switch (funct3) {
                case BEQ:
                    pipe->decode->inst=5; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);   
                    // BEQ_exe(cpu, inst); 
                    break;
                case BNE:
                    pipe->decode->inst=6; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);     
                    // BNE_exe(cpu, inst); 
                    break;
                case BLT:
                    pipe->decode->inst=7; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);   
                    // BLT_exe(cpu, inst); 
                    break;
                case BGE:
                    pipe->decode->inst=8;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);    
                    // BGE_exe(cpu, inst); 
                    break;
                case BLTU:  
                    pipe->decode->inst=9;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst); 
                    // BLTU_exe(cpu, inst); 
                    break;
                case BGEU:
                    pipe->decode->inst=10;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);  
                    // BGEU_exe(cpu, inst); 
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_LOAD:
            cpu->memory_instr_counter++;
            pipe->decode->isload=true;
            pipe->decode->iswrite=true;
            switch (funct3) {
                case LB  :
                    pipe->decode->inst=11;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // LB_exe(cpu, inst);
                    break;  
                case LH  :  
                    pipe->decode->inst=12;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LH_exe(cpu, inst);
                    break;  
                case LW  :  
                    pipe->decode->inst=13;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LW_exe(cpu, inst);
                    break;
                case LBU  :  
                    pipe->decode->inst=14;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LW_exe(cpu, inst);
                    break;    
                case LHU :  
                    pipe->decode->inst=15;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LHU_exe(cpu, inst); 
                    break; 
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            }
            break;

        case RV32i_S_TYPE:
            cpu->memory_instr_counter++;
            pipe->decode->isstype=true;
            pipe->decode->isstore=true;
            switch (funct3) {
                case SB  :
                    pipe->decode->inst=16;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);
                    // SB_exe(cpu, inst);
                    break;  
                case SH  :
                    pipe->decode->inst=17;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);  
                    // SH_exe(cpu, inst);
                    break;  
                case SW  : 
                    pipe->decode->inst=18;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);
                    // SW_exe(cpu, inst);
                    break;   
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_I_TYPE:
            pipe->decode->iswrite=true;
            cpu->register_instr_counter++;
            switch (funct3) {
                case ADDI: 
                    pipe->decode->inst=19;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // ADDI_exe(cpu, inst);
                    break;
                case SLLI:
                    pipe->decode->inst=25;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= shamt(inst);
                    // SLLI_exe(cpu, inst);
                    break;
                case SLTI:
                    pipe->decode->inst=20;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);  
                    // SLTI_exe(cpu, inst);
                    break;
                case SLTIU:
                    pipe->decode->inst=21;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // SLTIU_exe(cpu, inst);
                    break;
                case XORI: 
                    pipe->decode->inst=22;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // XORI_exe(cpu, inst);
                    break;
                case SRI:   
                    switch (funct7) {
                        case SRLI:
                            pipe->decode->inst=26;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->imm= shamt(inst);
                            // SRLI_exe(cpu, inst);
                            break;
                        case SRAI: 
                            pipe->decode->inst=27;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->imm= shamt(inst);
                            // SRAI_exe(cpu, inst);
                            break;
                        default:
                            fprintf(stderr, 
                                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                    , pipe->decode->inst,funct3,funct7);
                            exit(1);
                    } 
                    break;
                case ORI:
                    pipe->decode->inst=23;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);   
                    // ORI_exe(cpu, inst);
                    break;
                case ANDI:
                    pipe->decode->inst=24;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);  
                    // ANDI_exe(cpu, inst); 
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_R_TYPE:
            pipe->decode->isimm=false;
            pipe->decode->iswrite=true;
            cpu->register_instr_counter++;   
            switch (funct3) {
                case ADDSUB:
                    switch (funct7) {
                        case ADD: 
                            pipe->decode->inst=28;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // ADD_exe(cpu, inst);
                            break;
                        case SUB:
                            pipe->decode->inst=29;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // SUB_exe(cpu, inst);
                            break;
                        default:
                            fprintf(stderr, 
                                "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                , pipe->decode->inst, funct3,funct7);
                            exit(1);
                    } 
                    break;
                case SLL:
                    pipe->decode->inst=30;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);

                    // SLL_exe(cpu, inst);
                    break;
                case SLT:
                    pipe->decode->inst=31;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // SLT_exe(cpu, inst); 
                    break;
                case SLTU: 
                    pipe->decode->inst=32;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    // SLTU_exe(cpu, inst); 
                    break;
                case XOR:
                    pipe->decode->inst=33;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // XOR_exe(cpu, inst); 
                    break;
                case SR:   
                    switch (funct7) {
                        case SRL:
                            pipe->decode->inst=34;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);  
                            // SRL_exe(cpu, inst); 
                            break;
                        case SRA: 
                            pipe->decode->inst=35;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // SRA_exe(cpu, inst); 
                            break;
                        default:
                            fprintf(stderr, 
                                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                    , pipe->decode->inst, funct3,funct7);
                            exit(1);
                    }
                    break;
                case OR:
                    pipe->decode->inst=36;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);   
                    // OR_exe(cpu, inst); 
                    break;
                case AND:
                    pipe->decode->inst=37;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // AND_exe(cpu, inst);
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case 0x00:
            pipe->decode->inst=0;
            break;
        case LD_ST_NOC:
            cpu->noc_type_instr++;
            pipe->decode->isnoc=true;
            pipe->decode->isstore=true;
            switch (funct3) {
                case LOADNOC:
                    pipe->decode->inst=38;
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);
                    break;
                case STORENOC:
                    pipe->decode->inst=39;
                    pipe->decode->imm= 1;
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3, funct7);
                    exit(1);
            }
            break;
        case ADDSIMD:
            cpu->simd_type_instr++;
            pipe->decode->inst=40;
            pipe->decode->issimd=true;
            pipe->decode->isimm=false;
            pipe->decode->iswrite=true;
            pipe->decode->rd=rd(inst);
            pipe->decode->rs1=rs1(inst);
            pipe->decode->rs2=rs2(inst);
            break;
        case SUBSIMD:
            cpu->simd_type_instr++;
            pipe->decode->inst=41;
            pipe->decode->issimd=true;
            pipe->decode->isimm=false;
            pipe->decode->iswrite=true;
            pipe->decode->rd=rd(inst);
            pipe->decode->rs1=rs1(inst);
            pipe->decode->rs2=rs2(inst);
            break;
        default:
            fprintf(stderr, 
                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                    , pipe->decode->inst, funct3, funct7);
            exit(1);
    }
}

//Decode_stage
void cpu_decode(RISCV_cpu *cpu){
    if(cpu->__pipe->decode->done)return;
    cpu_control_unit(cpu,cpu->__pipe,cpu->__pipe->decode->inst);

    // Reading from reg file:
    if(cpu->__pipe->decode->issimd!=true){
        if(cpu->__pipe->decode->rs1!=34){
            cpu->__pipe->decode->rs1_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs1);
        }
        if(cpu->__pipe->decode->rs2!=34){
            cpu->__pipe->decode->rs2_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs2);
        }
    }
    else{
        for(int i =0 ;i<vector_size;i++){
            cpu->__pipe->decode->v1[i]=cpu->vec[cpu->__pipe->decode->rs1][i];
            cpu->__pipe->decode->v2[i]=cpu->vec[cpu->__pipe->decode->rs2][i];
        }
        
    }
    cpu->__pipe->decode->done=true;
}

//Execute_stage
void cpu_execute(RISCV_cpu *cpu){
    if(cpu->__pipe->execute->done)return;
    switch(cpu->__pipe->execute->inst){
        case 0:
            break;
        case 1:
            cpu->__pipe->execute->result=cpu->__pipe->execute->op2;
            break;
        case 2:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__alu->sl((i32)cpu->__pipe->execute->op2,12),(i32)cpu->__pipe->execute->pce);
            break;
        case 3:
            cpu->__pipe->execute->result=cpu->__alu->add(4,cpu->__pipe->execute->pce);
            break;
        case 4:
            cpu->__pipe->execute->result=cpu->__alu->add(4,cpu->__pipe->execute->pce);
            break;
        case 5:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x41)==0x41){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            cpu->__pipe->execute->rd=34;
            break;
        case 6:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x9)==0x9){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            cpu->__pipe->execute->rd=34;
            break;
        case 7:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            cpu->__pipe->execute->rd=34;
            break;
        case 8:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x51)==0x41||(cpu->__alu->getflag()&0x51)==0x11){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            
            cpu->__pipe->execute->rd=34;
            break;
        case 9:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            cpu->__pipe->execute->rd=34;
            break;
        case 10:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x51)==0x41||(cpu->__alu->getflag()&0x51)==0x11){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            cpu->__pipe->execute->rd=34;
            break;
        case 11:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=8;
            break;
        case 12:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=16;
            break;
        case 13:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=32;
            break;
        case 14:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=8;
            cpu->__pipe->execute->usign=true;
            break;
        case 15:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=16;
            cpu->__pipe->execute->usign=true;
            break;
        case 16:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            cpu->__pipe->execute->size=8;
            cpu->__pipe->execute->rd=34;
            break;
        case 17:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            cpu->__pipe->execute->size=16;
            cpu->__pipe->execute->rd=34;
            break;
        case 18:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            cpu->__pipe->execute->size=32;
            cpu->__pipe->execute->rd=34;
            break;
        case 19:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 20:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 21:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 22:
            cpu->__pipe->execute->result=cpu->__alu->xori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 23:
            cpu->__pipe->execute->result=cpu->__alu->ori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 24:
            cpu->__pipe->execute->result=cpu->__alu->andi((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 25:
            cpu->__pipe->execute->result=cpu->__alu->sl((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 26:
            cpu->__pipe->execute->result=cpu->__alu->sr((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            break;
        case 27:
            cpu->__pipe->execute->result=cpu->__alu->sra((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 28:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 29:
            cpu->__pipe->execute->result=cpu->__alu->sub((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 30:
            cpu->__pipe->execute->result=cpu->__alu->sl((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 31:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 32:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if((cpu->__alu->getflag()&0x21)==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 33:
            cpu->__pipe->execute->result=cpu->__alu->xori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 34:
            cpu->__pipe->execute->result=cpu->__alu->sr((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            break;
        case 35:
            cpu->__pipe->execute->result=cpu->__alu->sra((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 36:
            cpu->__pipe->execute->result=cpu->__alu->ori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 37:
            cpu->__pipe->execute->result=cpu->__alu->andi((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 38: //LoadNOC
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            cpu->__pipe->execute->size=32;
            cpu->__pipe->execute->rd=34;
            break;
        case 39: //STORENOC
            cpu->__pipe->execute->result=4;
            cpu->__pipe->execute->size=32;
            cpu->__pipe->execute->rd=34;
            break;
        case 40:
            if(cpu->__pipe->execute->issimd){
                for(int i=0;i<vector_size;i++){
                    cpu->__pipe->execute->vec_write[i]=cpu->__alu->add(cpu->__pipe->execute->v1[i],cpu->__pipe->execute->v2[i]);
                }
            }
            break;
        case 41:
            if(cpu->__pipe->execute->issimd){
                for(int i=0;i<vector_size;i++){
                    cpu->__pipe->execute->vec_write[i]=cpu->__alu->sub(cpu->__pipe->execute->v1[i],cpu->__pipe->execute->v2[i]);
                }
            }
            break;
        default:
        fprintf(stderr, 
                "[-] ERROR-> inst%d\n"
                , cpu->__pipe->execute->inst);
        exit(1);
    }
    cpu->__alu->flag_reset();
    cpu->__pipe->bypass->res_exec = cpu->__pipe->execute->result;
    cpu->__pipe->bypass->rde = cpu->__pipe->execute->rd;
    cpu->__pipe->execute->done=true;
    if(cpu->__pipe->execute->issimd){
        for(int i=0;i<vector_size;i++){
            cpu->__pipe->bypass->vec_ex[i]==cpu->__pipe->execute->vec_write[i];
        }
    }
}

//Helper Function for load and store
u32 cpu_ld(RISCV_cpu* cpu, u32 addr, u32 size) {
    return d_cache_ld(cpu->__bus->d_cache_bus,addr,size);
}
void cpu_st(RISCV_cpu* cpu, u32 addr, u32 size, u32 value) {
    d_cache_st(cpu->__bus->d_cache_bus, addr,size, value);
}

//Memory_stage
void cpu_memory(RISCV_cpu *cpu){
    if(cpu->__pipe->memory->done)return;
    if(cpu->__pipe->memory->isnoc){
        cpu->mem_map_reg[cpu->__pipe->memory->addr]=cpu->__pipe->memory->value;
        cpu->__pipe->memory->done=true;
        return;
    }
    if(cpu->__pipe->memory->isload){
        if(cpu->__pipe->memory->usign)
        cpu->__pipe->memory->value=cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
        else{
            if(cpu->__pipe->memory->size==8){
                cpu->__pipe->memory->value=(i32)(i8)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
            }
            else if(cpu->__pipe->memory->size==16){
                cpu->__pipe->memory->value=(i32)(i16)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
            }
            else{
                cpu->__pipe->memory->value=(i32)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
            }
            
        }
        
    }
    if(cpu->__pipe->memory->isstore){
        cpu_st(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size,cpu->__pipe->memory->value);
    }
    cpu->__pipe->bypass->res_mem = cpu->__pipe->memory->value;
    cpu->__pipe->bypass->rdm = cpu->__pipe->memory->rd;
    
    cpu->__pipe->memory->done=true;
}

//Writeback_stage
void cpu_writeback(RISCV_cpu*cpu){
    if(cpu->__pipe->writeback->done)return;
    if(cpu->__pipe->writeback->iswrite){
        if(cpu->__pipe->writeback->issimd){
            for(int i=0;i<vector_size;i++){
                cpu->vec[cpu->__pipe->writeback->rd][i]=cpu->__pipe->writeback->vec_write[i];
            }
        }
        else{
        cpu->x[cpu->__pipe->writeback->rd]=cpu->__pipe->writeback->imm;}
    }
    cpu->__pipe->writeback->done=true;
    if(cpu->__pipe->ex_stall){
        cpu->__pipe->execute->op2=cpu_read_reg(cpu,cpu->__pipe->execute->rs);
    }
    else if(cpu->__pipe->de_stall){
        if(cpu->__pipe->decode->rs1!=34){
            cpu->__pipe->decode->rs1_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs1);
        }
        if(cpu->__pipe->decode->rs2!=34){
            cpu->__pipe->decode->rs2_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs2);
        }
    }
}
