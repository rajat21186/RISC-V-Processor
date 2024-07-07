#include "LRU_cache.hpp"

using namespace std;

u32 bin_pow(u32 __bs, u32 __exp) {
    u32 __res = 1;
    while (__exp > 0) {
        if (__exp & 1)
            __res = __res * __bs;
        __bs = __bs * __bs;
        __exp >>= 1;
    }
    return __res;
}

u32 bin_to_val(string __s){
	u32 __res = 0;
	u32 __len = __s.length();
	REP(_z, 0, __len){ __res += (__s[_z] - '0') * bin_pow(2, __len - _z - 1); }
	return __res;
}


string val_to_bin(u32 __v, u32 __SZ){
    string __res = "";
    if (!__v) { return make_sz("0", __SZ); }
    while(__v > 0) {
	    __res += char(__v % 2 + 48);
	    __v/=2;
    }
    reverse(all(__res));
    return make_sz(__res, __SZ);
}

string make_sz(string __s, u32 __SZ) {
    string __res = "";
    REP(_z, 0, (int)(__SZ - __s.length())) __res += '0';
    __res += __s;
    return __res;
}

set_associative::set_associative(MEM_BUS* bus_memory,int __asc, int c_l, int blk_SZ)
{	
	this->_bus = bus_memory;
    this->assoc = __asc;
    this->cache_line = c_l;
    this->blk_SZ = blk_SZ;
    data.resize(c_l / __asc, vector<vector<u32>> (__asc, vector<u32> (blk_SZ)));  //VECTOR OF SETS, EACH SET HAS DATA_ENTRIES(EACH W/ SIZE blk_SZ)
	count.resize(c_l, 0);	//NO. OF USES OF CACHE WHERE iTH INDEX IN tag IS NOT TOUCHED;
	empty.resize(c_l, true);	//iTH ENTRY OF tag STORED IF EMPTY;
	tag.resize(c_l);		//tag CONTAINS THE TAG OF ALL THE DATA ENTRIES;
}
// EVERY ELEMENT IN count IS INCREMENTED DURING READ AND WRITE OPS;
void set_associative::__incre__(){
    REP(_z, 0, cache_line){
	    if (!empty[_z]) {
		    if (count[_z] == MA) continue;
		    count[_z]++;
	    }
	}
}  

u32 set_associative::cache_read(u32 byte_ADDR, u32 set_ADDR, u32 off) {
	//CHECKS ALL ENTRIES IN THE set_ADDR'TH SET AND MATCHES THEM WITH THE TAG_ADDR(byte_ADDR)
	__incre__();
	REP(_z, 0, assoc) {
	    if (!empty[set_ADDR * assoc + _z] && tag[set_ADDR * assoc + _z] == byte_ADDR) {
		count[set_ADDR * assoc + _z] = 0;
		return data[set_ADDR][_z][off];
	    }
	}
	return -1;
}

void set_associative::cache_write(u32 byte_ADDR, u32 set_ADDR, u32 off, u32 _val) {
	//CHECKS ALL ENTRIES IN THE set_ADDR'TH SET AND MATCHES THEM WITH THE TAG_ADDR(byte_ADDR);
	//PERFORMS WRITE_OP ON THAT ENTRY W/ OFFSET(off);
	//IF NO MATCH, CHECK IF BLOCK EMPTY:
	//	INSERT BLOCK AND WRITE DATA W/ OFFSET
	//  IF NO, EVICT THE BLOCK W/ MOST COUNT AND INSERT NEW BLOCK AND WRITE THERE;
	__incre__();
	REP(_z, 0, assoc){
	    if (!empty[set_ADDR * assoc + _z] && tag[set_ADDR * assoc + _z] == byte_ADDR) {
		    data[set_ADDR][_z][off] = _val;
		    count[set_ADDR * assoc + _z] = 0;
		    return;
	    }
	}
	
	int u_lim {0}, idx {-1};
	REP(_z, 0, assoc) {
	    // [set_addr * assoc + iter] MAP (iter) ELEMS OFF set_addr-th SET TO ITS IDX IN empty, tag, count ARR
	    if (empty[set_ADDR * assoc + _z]) {
		    count[set_ADDR * assoc + _z] = 0;
		    tag[set_ADDR * assoc + _z] = byte_ADDR;
		    empty[set_ADDR * assoc + _z] = false;
		    REP(_j, 0, blk_SZ){
		        if (_j == off) { data[set_ADDR][_z][_j] = _val; }
                else{ data[set_ADDR][_z][_j] = 0; }
		    }
		    return;
	    }else {
		if (count[set_ADDR * assoc + _z] > u_lim) {
		        u_lim = count[set_ADDR * assoc + _z];
		        idx = _z;
		    }
	    }
	}
	count[set_ADDR * assoc + idx] = 0;
	tag[set_ADDR * assoc + idx] = byte_ADDR;
	REP(_z, 0, blk_SZ) {
	    if (_z == off) { data[set_ADDR][idx][_z] = _val; }
        else { data[set_ADDR][idx][_z] = 0; }
    }
}
    
void set_associative::__print__(FILE* fp) {
	REP(_z, 0, cache_line) {
	    if (empty[_z]) {
		    fprintf(fp,"##EMPTY##\n");
		    continue;
	    }
	
	    if (_z % assoc == 0) { fprintf(fp,"\n"); }
		// differentiate between sets
		fprintf(fp,"##BLOCK## ");
		fprintf(fp,"%s",val_to_bin(tag[_z], 32 - __lg(blk_SZ) - __lg(cache_line / assoc)).c_str());
	    fprintf(fp," ##DATA## : ");
	    REP(_j, 0, blk_SZ) { fprintf(fp,"0x%x ",data[_z / assoc][_z % assoc][_j]); }
		fprintf(fp,"\n");
	}
}
/**

//       cache__.resize(4, list<CACHE_blk>(assoc));
//      @param : (SET_NUM, list<CACHE_blk>(assoc));
//                  set_idx = addr % SET_NUM

optional<u32> set_associative::read(u32 addr) {
    int set_idx = addr / 4;
    u32 tag = addr / 4;
    auto it = tag_cache_blk_map.find(tag);
    if (it != tag_cache_blk_map.end() && it->second->valid) {
        // CACHE_HIT => UPDATE LRU
        cache__[idx].splice(cache__[idx].begin(), cache__[idx], it->second);
        return it->second->datx_[addr % 2]; //
    }
    return std::nullopt; // Cache miss
}
    
void set_associative::write(u32 addr, u32 data) {
    int idx = set_idx(addr);
    u64 tag = addr / 4;

    auto it = tag_cache_blk_map.find(tag);
    if (it != tag_cache_blk_map.end()) {
        // If tag exists, update the block and move to the front (LRU)
        it->second->datx_[addr % 8] = data;
        cache__[idx].splice(cache__[idx].begin(), cache__[idx], it->second);
    } else {
        // If tag doesn't exist, insert a new block and check for eviction
        CACHE_blk new_blk;
        new_blk.valid = true;
        new_blk.__tag = tag;
        new_blk.datx_[addr % 8] = data;
        cache__[idx].push_front(new_blk);
        tag_cache_blk_map[tag] = cache__[idx].begin();

        if (cache__[idx].size() > 2) {
            tag_cache_blk_map.erase(cache__[idx].back().__tag);
            cache__[idx].pop_back();
        }
    }
}

*/
