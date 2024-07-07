#ifndef CACHE_RISCV_H
#define CACHE_RISCV_H
#include "membus.hpp"

#define MA 0xffffffff

/**
OFFSET_ADDR :: lg(CACHE_LINE_SZ) :: lg(4) = 2 bits
CACHE_LINE_NUM :: 8
CACHE_MEM_SZ :: CACHE_LINE_SZ * CACHE_LINE_NUM  :: 4 * 8 = 32 bytes
SET_NUM ::  CACHE_LINE_NUM / ASSOC :: 8/2 :: 4
SET_ADDR_WIDTH :: lg(SET_NUM) = lg(4) = 2
TAG_WIDTH :: ADDR_WIDTH - SET_WIDTH - OFFSET_WIDTH :: 32-3-5 :: 24 Bits
*/

u32 bin_pow(u32 __bs, u32 __exp);
u32 bin_to_val(std::string __s);
std::string val_to_bin(u32 __v, u32 __SZ);
std::string make_sz(std::string __s, u32 __SZ);


//UNIFIED_CLASS
class set_associative{
private:
    int assoc;
    int cache_line;
    int blk_SZ;
    std::vector<u32> tag;
    std::vector<std::vector<std::vector<u32>>> data;
    std::vector<int> count;
    std::vector<bool> empty;
    
public:
    MEM_BUS* _bus;
    set_associative(MEM_BUS* bus_memory,int assoc, int cache_line, int blk_SZ);
    void __incre__();
    u32 cache_read(u32 byte_ADDR, u32 set_ADDR, u32 off);
    //off = bin_to_val(ADDR_STR.substr(32 - __lg(blk_SZ), __lg(blk_SZ)));
    //set_addr = bin_to_val(ADDR_STR.substr(32 - __lg(blk_SZ) - __lg(c_l / assoc), __lg(c_l / assoc)));
    //byte_addr = bin_to_val(ADDR_STR.substr(0, 32 - __lg(blk_SZ) - __lg(c_l / assoc)));
    void cache_write(u32 byte_ADDR, u32 set_ADDR, u32 off, u32 _val);
    void __print__(FILE *fp);
};

#endif


/**
struct CACHE_blk {
    bool valid;
    u32 __tag;
    std::vector<u32> datx_;
    CACHE_blk() : __tag(-1), datx_(32, 0), valid(false) {}
};

struct CACHE_set {
    std::array<CACHE_blk, 2> blks_;
    int __lru_idx;
    CACHE_set() : blks_(), __lru_idx(0) {}
};

class set_associative{
private:
    int assoc;
    int cache_lines;
    int blk_SZ;
    std::vector<CACHE_set> mulset_;
//    std::vector<list<CACHE_blk>> cache__;
//    std::unordered_map<u32, typename std::list<CACHE_blk>::iterator>tag_cache_blk_map;

public:
    set_associative(int assoc, int cache_lines, int blk_SZ);
    optional<u32> cache_ld(u32 addr);
    void cache_st(u32 addr, u32 data);
    //~set_associative();
};

*/
