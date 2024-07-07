#include "ALU.hpp"

u8 ALU::getflag(){return flags;}
bool ALU::isoverflow(){return ((flags & 0x80)==0x80);}
void ALU::flag_reset(){flags=1;}

u32 ALU::sub(i32 a, i32 b){
    i32 res= a-b;
    if((u32) res > (u32) a || (u32) res > (u32) b) flags |= 0x80;
    return (u32) res;
}
u32 ALU::add(i32 a, i32 b){
    i32 res= a+b;
    if((u32) res < (u32) a || (u32) res < (u32) b) flags |= 0x80;
    return (u32) res;
}

void ALU::comp(i32 a, i32 b){
    if(a==b) flags |= 0x40;
    else flags |= 0x8;
    if(a<b) flags |= 0x20;
    if(a>b) flags |= 0x10;
}
void ALU::compu(u32 a, u32 b){
    if(a==b) flags |= 0x40;
    else flags |= 0x8;
    if(a<b) flags |= 0x20;
    if(a>b) flags |= 0x10;
}

u32 ALU::sl(i32 a,i32 offset){return a<<offset;} // shift left
u32 ALU::sr(i32 a,i32 offset){return ((u32)a)>>offset;}// shift right
u32 ALU::sra(i32 a,i32 offset){return a>>offset;} // shift right arithmetic


u32 ALU::andi(i32 a,i32 b){return a&b;}
u32 ALU::ori(i32 a,i32 b){ return a|b;}
u32 ALU::xori(i32 a,i32 b){return a^b;}

ALU* alu_init(){return (new ALU());}
