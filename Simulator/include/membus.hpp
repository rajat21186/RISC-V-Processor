#ifndef MEMBUS_H
#define MEMBUS_H
#include "utils.hpp"
#include "memory.hpp"

struct DATA_MEM_BUS {
    struct RISC_DATA_mem* data_mem;
};

struct INSTR_MEM_BUS {
    struct RISC_INSTR_mem* instr_mem;
};

struct MEM_BUS {
    struct DATA_MEM_BUS* data_bus;
    struct INSTR_MEM_BUS* instr_bus;
};

INSTR_MEM_BUS* instr_bus_init(INSTR_mem* mem);
DATA_MEM_BUS* data_bus_init(DATA_mem* mem);
MEM_BUS* mem_bus_init();

u32 data_bus_ld(DATA_MEM_BUS* _bus, u32 addr, u32 size);
void data_bus_st(DATA_MEM_BUS* _bus, u32 addr, u32 size, u32 value);

u32 instr_bus_ld(INSTR_MEM_BUS* _bus, u32 addr, u32 size);
void instr_bus_st(INSTR_MEM_BUS* _bus, u32 addr, u32 size, u32 value);

#endif
