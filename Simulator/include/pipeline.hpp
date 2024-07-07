#ifndef PIPELINERISCV_H
#define PIPELINERISCV_H

# include "utils.hpp"

struct f_unit{
    u32 inst; //readinst
    bool done; //executed once?
    u32 pcf;
};

struct d_unit{
    bool issimd; //simd instruction?
    bool isnoc;// noc instruction?
    bool isstore;// ->
    bool isload; // ->
    bool iswrite;//  signals
    bool isstype; // signal for stalling (without bypassing)
    int inst; //readinst then insttype
    u32 rs1; //rs1 decoded
    u32 rs2; //rs2 decoded
    bool isimm; //imm present?
    u32 rd; //rd decoded
    i32 imm; //imm decoded
    u32 rs1_val; //rs1 value
    u32 rs2_val; //rs2 value
    bool done; //executed once?
    u32 pcd; // pc of decode stage 
    u32 v1[vector_size];// vector 1
    u32 v2[vector_size];// vector 2
};

struct exec_unit{
    bool isnoc;// buffer
    bool issimd; //simd instruction?
    bool isstore;// ->
    bool isload; // ->
    bool iswrite;//  signals
    bool isstype; // signal for stalling (without bypassing)
    u8 inst; //insttype
    u32 op1; //rs1
    u32 op2; //imm ,rs2
    u32 result; //aluresult
    u32 rd; //buffer for rd ;; also used for storing  the imm for s and sb type of Instructions
    u8 size; //size of load/store
    bool usign; //unsigned
    bool done; //executed once?
    u32 rs; //rs for store
    u32 pce; // pc of exec stage
    u32 vec_write[vector_size];// vector to be written
    u32 v1[vector_size];// vector 1
    u32 v2[vector_size];// vector 2
};

struct mem_unit{
    u8 inst; //insttype
    bool isnoc;// signals for mem_map_registers
    bool issimd; //simd instruction?
    bool isstore; //signals store & 
    bool isload; // load
    bool iswrite; // buffer signal
    bool isstype; // signal for stalling (without bypassing)
    u32 addr; //address to load/store
    u8 size; //size of load/store
    u32 value; //value to store or loaded value
    u32 rd; //buffer for rd
    bool usign; //unsigned
    bool done; //executed once?
    u32 vec_write[vector_size];// vector to be written
};
struct wb_unit{
    u8 inst; //insttype
    bool issimd; //simd instruction?
    bool iswrite;//signal(write to regfile)
    u32 rd; //reg to write
    u32 imm;  // value to write
    bool done; //executed once?
    u32 vec_write[vector_size];// vector to be written
};

struct bypassreg{
    u32 res_exec; //result of execute
    u32 res_mem; //result of memory(load)
    u32 rde; // register for execute
    u32 rdm; // register fof memory
    u32 new_pc; //new pc 
    u32 vec_ex[vector_size];// vector from exec
    u32 vec_mem[vector_size];// vector from memory
};

struct pipeline{
    int cycle; //current cycle no
    d_unit *decode; //decode unit
    f_unit *fetch; //fetch unit
    exec_unit *execute; //execute unit
    mem_unit *memory; //memory unit
    wb_unit *writeback; //writeback unit
    bool isbranch; //branch taken? (by execute)
    u32 newpc_offset; //new pc offset (for execute)
    bool isjalr; //jalr taken?
    bool isbranch2; //branched in decode?
    u32 newpc_offset2; //new pc offset or new pc(for jalr) (for decode)
    struct bypassreg* bypass; //bypass register 
    bool de_stall; //stall decode?
    bool ex_stall; //stall execute?
};



// pipeline init
pipeline* pipe_init();

// reset pipeline
void pipeline_reset(pipeline *pipe);

// to controll state changes in pipeline
bool statechange(pipeline* pipe,u32* pc,bool over);
bool statechange_withoutBYPASSING(pipeline* pipe,bool over);
//Pipeline ending function
// bool pipeline_end(bool fetchdone,pipeline* pipe);
// hidden implementations

// reset bypass registers
// void bypass_reset(bypassreg *bypass);

// function to read bypass registers
// u32 bypass_read(pipeline *pipe,struct d_unit *decode,bool isrs1);

// kill / reset pipeline stages or buffers with nop (inst=0)
// void decode_reset(d_unit *decode);
// void fetch_reset(f_unit *fetch);
// void execute_reset(exec_unit *execute);
// void memory_reset(mem_unit *memory);
// void writeback_reset(wb_unit *writeback);

//to detect and kill the pipeline
// void jump(pipeline*pipe);
// void jump_withoutBYPASSING(pipeline*pipe);

// state change functions
// void changef_to_d(pipeline *pipe);
// void changed_to_ex(pipeline *pipe);
// void changeex_to_m(pipeline *pipe);
// void changem_to_wb(pipeline *pipe);

// stalling functions
// void stall_withoutBYPASSING(pipeline*pipe);
// void stall(pipeline*pipe);

// reset the branch flags
// void reset_branchflags(pipeline *pipe);

//to try state change
// void try_statechange(pipeline* pipe);

// state chnge care->order shold be =>
// updating pc
// then check for any killable process +stallable process
// changem_to_wb -> changeex_to_m -> changed_to_ex -> changef_to_d
// then only cpu will start next cycle

#endif
