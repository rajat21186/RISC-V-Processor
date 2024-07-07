#include "cache_bus.hpp"

using namespace std;

DATA_CACHE_BUS* d_cache_bus_init(MEM_BUS* mem_bus){
    DATA_CACHE_BUS* _bus = (DATA_CACHE_BUS*)malloc(sizeof(DATA_CACHE_BUS));
    if (_bus==NULL) {
        fprintf(stderr, "[-] ERROR-> data_cache_bus_init: malloc failed\n");
        exit(1);
    }
    _bus->data_cache = new set_associative(mem_bus, 2, 8, 32);
    if (_bus->data_cache == NULL) {
        fprintf(stderr, "[-] ERROR-> data_cache_init: malloc failed\n");
        exit(1);
    }
    return _bus;
}

INSTR_CACHE_BUS* i_cache_bus_init(MEM_BUS* mem_bus){
    INSTR_CACHE_BUS* _bus = (INSTR_CACHE_BUS*)malloc(sizeof(INSTR_CACHE_BUS));
    if (_bus == NULL) {
        fprintf(stderr, "[-] ERROR-> instr_cache_bus_init: malloc failed\n");
        exit(1);
    }
    _bus->instr_cache = new set_associative(mem_bus,2, 8, 32);
    if (_bus->instr_cache == NULL) {
        fprintf(stderr, "[-] ERROR-> instr_cache_init: malloc failed\n");
        exit(1);
    }
    
    return _bus;
}




cache_bus* cache_bus_init(){
    MEM_BUS* mem_bus = mem_bus_init();
    if (mem_bus == NULL) {
        fprintf(stderr, "[-] ERROR-> mem_bus_init: malloc failed\n");
        exit(1);
    }
    cache_bus* __bus = (cache_bus*)malloc(sizeof(cache_bus));
    __bus->d_cache_bus = d_cache_bus_init(mem_bus);
    __bus->i_cache_bus = i_cache_bus_init(mem_bus);
    return __bus;
}
/*
cache_bus_data* d_cache_bus_init(MEM_BUS *__bus){
    __bus->data_cache_bus = new set_associative(2, 8, 4);
    __bus->instr_cache_bus = new set_associative(2, 8, 4);
    return __bus
*/

u32 d_cache_ld(DATA_CACHE_BUS* d_cache_bus, u32 addr, u32 size) {
    string ADDR_str = val_to_bin(addr, 32);
    u32 off = bin_to_val(ADDR_str.substr(27, 5));
	u32 set_ADDR = bin_to_val(ADDR_str.substr(25, 2));
	u32 byte_ADDR = bin_to_val(ADDR_str.substr(0, 25));	    
    u32 _val = d_cache_bus->data_cache->cache_read(byte_ADDR, set_ADDR, off);
    //CACHE_MISS
    if(_val == -1){ _val = data_bus_ld(d_cache_bus->data_cache->_bus->data_bus, addr, size); } 
    return _val;
}

void d_cache_st(DATA_CACHE_BUS* d_cache_bus, u32 addr, u32 size, u32 _val){
    string ADDR_str = val_to_bin(addr, 32);
    u32 off = bin_to_val(ADDR_str.substr(27, 5));
	u32 set_ADDR = bin_to_val(ADDR_str.substr(25, 2));
	u32 byte_ADDR = bin_to_val(ADDR_str.substr(0, 25));  
	d_cache_bus->data_cache->cache_write(byte_ADDR, set_ADDR, off, _val);
}

u32 i_cache_ld(INSTR_CACHE_BUS* i_cache_bus, u32 addr, u32 size){
    string ADDR_str = val_to_bin(addr, 32);
    u32 off = bin_to_val(ADDR_str.substr(27, 5));
	u32 set_ADDR = bin_to_val(ADDR_str.substr(25, 2));
	u32 byte_ADDR = bin_to_val(ADDR_str.substr(0, 25));	    
    u32 _val = i_cache_bus->instr_cache->cache_read(byte_ADDR, set_ADDR, off);
    //CACHE_MISS
    if(_val == -1){ _val = instr_bus_ld(i_cache_bus->instr_cache->_bus->instr_bus, addr, size); } 
    return _val;
}

void i_cache_st(INSTR_CACHE_BUS* i_cache_bus, u32 addr, u32 size, u32 _val){
    string ADDR_str = val_to_bin(addr, 32);
    u32 off = bin_to_val(ADDR_str.substr(27, 5));
	u32 set_ADDR = bin_to_val(ADDR_str.substr(25, 2));
	u32 byte_ADDR = bin_to_val(ADDR_str.substr(0, 25));  
	i_cache_bus->instr_cache->cache_write(byte_ADDR, set_ADDR, off, _val);
    instr_bus_st(i_cache_bus->instr_cache->_bus->instr_bus, addr, size,_val);
}


/*  CACHE_READ:   
    string ADDR_str = val_to_bin(addr, 32);
    u32 off = bin_to_val(addr.substr(32 - (int)log2(blk_SZ), (int)log2(blk_SZ)));
	u32 set_ADDR = bin_to_val(addr.substr(32 - (int)log2(blk_SZ) - (int)log2(c_l / assoc), (int)log2(c_l / assoc)));
	u32 byte_ADDR = bin_to_val(addr.substr(0, 32 - (int)log2(blk_SZ) - (int)log2(c_l / assoc)));
		    
    u32 _val = cache.cache_read(byte_ADDR, set_ADDR, off);
		    
	if (_val == -1) { "CACHE_MISS" }
    else { "PRINT_VAL" }
*/

/*  
    CACHE_WRITE:
    u32 off = bin_to_val(addr.substr(32 - (int)log2(blk_SZ), (int)log2(blk_SZ)));
	u32 set_ADDR = bin_to_val(addr.substr(32 - (int)log2(blk_SZ) - (int)log2(c_l / assoc), (int)log2(c_l / assoc)));
	u32 byte_ADDR = bin_to_val(addr.substr(0, 32 - (int)log2(blk_SZ) - (int)log2(c_l / assoc)));
		    
	cache.cache_write(byte_ADDR, set_ADDR, off, _val);
*/
