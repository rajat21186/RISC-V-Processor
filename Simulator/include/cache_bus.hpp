#ifndef CACHE_BUS_HPP
#define CACHE_BUS_HPP

#include "LRU_cache.hpp"

struct DATA_CACHE_BUS{
    set_associative* data_cache;
};

struct INSTR_CACHE_BUS{
    set_associative* instr_cache;
};

struct cache_bus{
    struct DATA_CACHE_BUS* d_cache_bus;
    struct INSTR_CACHE_BUS* i_cache_bus;
};

DATA_CACHE_BUS* d_cache_bus_init();
INSTR_CACHE_BUS* i_cache_bus_init();
cache_bus* cache_bus_init();

u32 d_cache_ld(DATA_CACHE_BUS* d_cache_bus, u32 addr, u32 size);
void d_cache_st(DATA_CACHE_BUS* d_cache_bus, u32 addr, u32 size, u32 _val);
u32 i_cache_ld(INSTR_CACHE_BUS* i_cache_bus, u32 addr, u32 size);
void i_cache_st(INSTR_CACHE_BUS* i_cache_bus, u32 addr, u32 size, u32 _val);

#endif
