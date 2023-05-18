#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("xargs : minum amount of args is 2 !\n");
        exit(1);
    }
    if(argc > MAXARG){
        printf("xargs : maxium amount of args is %d !\n",MAXARG);
        exit(1);
    }
    char *new_argv[MAXARG];
    for(int i=1;i<argc;i++){
        new_argv[i-1] = argv[i];
    }
    char* buf=(char*)malloc(1024);
    int idx=0;
    int count=argc-1;
    int is_end=0;
    while(read(0, &buf[idx], sizeof(char))!=0){
        if(buf[idx]=='\n') is_end=1;
        if(buf[idx]==' '||buf[idx]=='\n'){
            buf[idx]=0;
            new_argv[count++]=buf;
            idx=0;
            buf=(char*)malloc(1024);
        }
        else idx++;
        if(is_end){
            is_end=0;
            new_argv[count]=0;
            if(fork()==0){
                exec(new_argv[0],new_argv);
            }
            else{
                wait(0);
                for(int i=argc-1;i<count;i++)free(new_argv[i]);
            }
            count=argc-1;
        }
        
    }
    exit(0);
}