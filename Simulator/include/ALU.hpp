#ifndef ALU_H
#define ALU_H
#include "utils.hpp"
class ALU{
    private:
        u8 flags; // 8: overflow, 7:equal, 6:less, 5:greater,4:notequal 1:valid , (3,2 reserved) 
    public:
        ALU(){};
        bool isoverflow();
        void flag_reset();
        u8 getflag();
        u32 sub(i32 a, i32 b);
        u32 add(i32 a, i32 b);

        void comp(i32 a, i32 b);
        void compu(u32 a, u32 b);

        u32 sl(i32 a,i32 offset);
        u32 sr(i32 a,i32 offset); 
        u32 sra(i32 a,i32 offset);
        
        u32 andi(i32 a,i32 b);
        u32 ori(i32 a,i32 b);
        u32 xori(i32 a,i32 b);

        ~ALU(){};
};
ALU* alu_init();
#endif

