#include "memory.hpp"

u32 d_mem_ld_8(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data];
}
u32 d_mem_ld_16(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data]
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 1] << 8;
}
u32 d_mem_ld_32(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data]
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 1] << 8
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 2] << 16 
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 3] << 24;
}

u32 d_mem_ld(DATA_mem* mem, u32 addr, u32 sz_) {
    switch (sz_) {
        case 8:  return d_mem_ld_8(mem, addr);  break;
        case 16: return d_mem_ld_16(mem, addr); break;
        case 32: return d_mem_ld_32(mem, addr); break;
        default: ;
    }
    return 1;
}


void d_mem_st_8(DATA_mem* mem, u64 addr, u64 value) {
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data] = (u8) (value & 0xff);
}

void d_mem_st_16(DATA_mem* mem, u64 addr, u64 value) {
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data] = (u8) (value & 0xff);
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 1] = (u8) ((value >> 8) & 0xff);
}

void d_mem_st_32(DATA_mem* mem, u32 addr, u64 value) {
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data] = (u8) (value & 0xff);
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 1] = (u8) ((value >> 8) & 0xff);
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 2] = (u8) ((value >> 16) & 0xff);
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE_Data + 3] = (u8) ((value >> 24) & 0xff);
}

void d_mem_st(DATA_mem* data_mem, u32 addr, u32 sz_, u32 value){
    switch (sz_) {
        case 8:  d_mem_st_8(data_mem, addr, value);  break;
        case 16: d_mem_st_16(data_mem, addr, value); break;
        case 32: d_mem_st_32(data_mem, addr, value); break;
        default: ;
    }
}

DATA_mem *data_init(){
    DATA_mem *mem = (DATA_mem*)malloc(sizeof(DATA_mem));
    if (mem == NULL) {
        fprintf(stderr, "[-] ERROR-> data_mem_init : malloc failed\n");
        exit(1);
    }
    return mem;
}

INSTR_mem *instr_init(){
    INSTR_mem *mem = (INSTR_mem*)malloc(sizeof(INSTR_mem));
    if (mem == NULL) {
        fprintf(stderr, "[-] ERROR-> instr_mem_init : malloc failed\n");
        exit(1);
    }
    return mem;
}

u32 i_mem_ld(INSTR_mem* mem, u32 addr, u32 sz_){
    return (u32) mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR]
        |  (u32) mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 1] << 8
        |  (u32) mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 2] << 16 
        |  (u32) mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 3] << 24;
}

void i_mem_st(INSTR_mem* mem, u32 addr,u32 sz_, u32 value){
    mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR] = (u8) (value & 0xff);
    mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 1] = (u8) ((value >> 8) & 0xff);
    mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 2] = (u8) ((value >> 16) & 0xff);
    mem->rv32i_instr_mem[addr - RISCV_MEM_BASE_INSTR + 3] = (u8) ((value >> 24) & 0xff);
}
