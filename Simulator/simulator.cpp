#include "LogFile.hpp"

void start(RISCV_cpu *cpu,u32 INST_END){
    FILE* fp = fopen("../logs/logfile.log","a");
    if(fp==NULL){
        fprintf(stderr,"Error in opening logfile\n");
        exit(1);
    }
    fprintf(fp, "****************************************************************************\n");
    fprintf(fp, "******************************RISCV SIMULATOR*******************************\n");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(fp, "Date and Time: %s\n", asctime(tm) );
    fprintf(fp, "\n");
    bool notend=true;
    u32 new_pc=0x0;
    bool over=false;
    int extracycle=1;
    if(INST_END==0){
        extracycle=0;
        notend=false;
    }
    while(notend || extracycle--){
        cpu_writeback(cpu);
        cpu_memory(cpu);
        cpu_execute(cpu);
        cpu_decode(cpu);
        cpu_fetch(cpu,new_pc ,((cpu->pc+4 != new_pc)&&over));
        logValues(fp,cpu, cpu->__pipe);
        new_pc=cpu_pc_update(cpu);
        over=(new_pc>=INST_END);
        // notend=statechange_withoutBYPASSING(cpu->__pipe,over); // without bypassing logic
        notend=statechange(cpu->__pipe,&new_pc,over); // with bypassing logic
        if(cpu->__pipe->cycle>20){
            exit(1);
        }
    }
    fprintf(fp, "****************************RISCV SIMULATOR END*****************************\n");
    fprintf(fp, "****************************************************************************\n");
    fclose(fp);
}
void save_cache(RISCV_cpu* cpu){
    FILE* fp = fopen("../logs/log_cache.log","a");
    if(fp==NULL){
        fprintf(stderr,"Error in opening log_cache file\n");
        exit(1);
    }
    fprintf(fp, "****************************************************************************\n");
    fprintf(fp, "******************************RISCV SIMULATOR*******************************\n");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(fp, "Date and Time: %s\n", asctime(tm) );
    fprintf(fp, "\n");
    fprintf(fp, "******************************I-CACHE CONTENTS*******************************\n");
    fprintf(fp, "\n");
    cpu->__bus->i_cache_bus->instr_cache->__print__(fp);
    fprintf(fp, "******************************D-CACHE CONTENTS*******************************\n");
    fprintf(fp, "\n");
    cpu->__bus->d_cache_bus->data_cache->__print__(fp);
    fprintf(fp, "****************************RISCV SIMULATOR END*****************************\n");
    fprintf(fp, "****************************************************************************\n");
    fclose(fp);
}
u32 load_Instrucctions_in_memory(RISCV_cpu* cpu,char* path){
    FILE* fp = fopen(path,"rb");
    if(fp==NULL){
        fprintf(stderr,"Error in opening binary file\n");
        exit(1);
    }
    
    u32  INST_END= RISCV_MEM_BASE_INSTR;
    u32 INST = 0;
    while(fread(&INST,sizeof(u32),1,fp)){
        i_cache_st(cpu->__bus->i_cache_bus,INST_END, 32, INST);
        INST_END+=4;
    }
    fclose(fp);
    // To debug ld and store
    // cpu_st(cpu, 87, 32, 0xf1f2);
    return INST_END;
    
}

int main(int argc, char* argv[]){
    RISCV_cpu* mycpu = CPU_init(); 
    if(argc!=2){
        fprintf(stderr,"No input to simulator: <path to bin file missing> command line arguments no-> %d\n",argc);
        exit(1);
    }
    u32 INST_END = load_Instrucctions_in_memory(mycpu,argv[1]);
    CPU_reset(mycpu);
    mycpu->__pipe->cycle=0;
    start(mycpu,INST_END);
    save_cache(mycpu);
    return 0;
}

