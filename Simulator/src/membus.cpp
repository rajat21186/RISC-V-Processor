#include "membus.hpp"

u32 data_bus_ld(DATA_MEM_BUS* _bus, u32 addr, u32 size) {
    return d_mem_ld((_bus->data_mem), addr,size);
}

void data_bus_st(DATA_MEM_BUS* _bus, u32 addr, u32 size, u32 value){
    d_mem_st((_bus->data_mem), addr, size, value);
}

u32 instr_bus_ld(INSTR_MEM_BUS* _bus, u32 addr, u32 size){
    return i_mem_ld(_bus->instr_mem,addr,size);
}
void instr_bus_st(INSTR_MEM_BUS* _bus, u32 addr, u32 size, u32 value){
    i_mem_st(_bus->instr_mem,addr,size,value);
}
DATA_MEM_BUS* data_bus_init(DATA_mem* mem){
    DATA_MEM_BUS* _bus = (DATA_MEM_BUS*)malloc(sizeof(DATA_MEM_BUS));
    _bus->data_mem = mem;
    return _bus;
}

INSTR_MEM_BUS* instr_bus_init(INSTR_mem* mem){
    INSTR_MEM_BUS* _bus = (INSTR_MEM_BUS*)malloc(sizeof(INSTR_MEM_BUS));
    _bus->instr_mem = mem;
    return _bus;
}

MEM_BUS* mem_bus_init(){
    MEM_BUS* __bus = (MEM_BUS*)malloc(sizeof(MEM_BUS));
    if (__bus == NULL) {
        fprintf(stderr, "[-] ERROR-> mem_bus_init: malloc failed\n");
        exit(1);
    }
    DATA_mem* mem_d = data_init();
    if (mem_d == NULL) {
        fprintf(stderr, "[-] ERROR-> data_mem_init: malloc failed\n");
        exit(1);
    }
    __bus->data_bus = data_bus_init(mem_d);
    if (__bus->data_bus == NULL) {
        fprintf(stderr, "[-] ERROR-> data_mem_bus_init: malloc failed\n");
        exit(1);
    }
    INSTR_mem* mem_i = instr_init();
    if (mem_i == NULL) {
        fprintf(stderr, "[-] ERROR-> instr_mem_init: malloc failed\n");
        exit(1);
    }
    __bus->instr_bus = instr_bus_init(mem_i);
    if (__bus->instr_bus == NULL) {
        fprintf(stderr, "[-] ERROR-> instr_mem_bus_init: malloc failed\n");
        exit(1);
    }
    return __bus;
}

