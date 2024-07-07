#ifndef CPU_H
#define CPU_H

#include "instruction.hpp"
#include "pipeline.hpp"
#include "ALU.hpp"
#include "cache_bus.hpp"
#define REG_LEN 32
#define MM_REG_LEN 5
#define MM_BASE_ADDR 0x10000 // 65536 (since mem is 64kb)

struct RISCV_cpu{
    u32 x[REG_LEN];
    u32 mem_map_reg[MM_REG_LEN];
    u32 vec[REG_LEN][vector_size];//vector array
    u32 pc;
    struct cache_bus* __bus;
    struct pipeline* __pipe;
    ALU* __alu;
    int memory_instr_counter;
    int register_instr_counter;
    int noc_type_instr;
    int simd_type_instr;
};

RISCV_cpu* CPU_init();
void CPU_reset(RISCV_cpu *cpu);
void cpu_fetch(RISCV_cpu *cpu,u32 new_pc,bool early_exit);
void cpu_decode(RISCV_cpu *cpu);
void cpu_execute(RISCV_cpu *cpu);
void cpu_memory(RISCV_cpu *cpu);
void cpu_writeback(RISCV_cpu*cpu);
u32 cpu_pc_update(RISCV_cpu*cpu);

#endif
