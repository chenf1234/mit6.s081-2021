#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc > 1){
        fprintf(2, "Usage: pingpong\n");
        exit(1);
    }

    int p[2];
    pipe(p);
    if(fork()!=0){
        char c='c';
        write(p[1],&c,1);
        if(read(p[0],&c,1) == 1)
            printf("%d: received pong\n", getpid());
    }
    else{
        char c;
        if(read(p[0],&c,1) == 1)
            printf("%d: received ping\n", getpid());
        write(p[1],&c,1);
    }
    close(p[0]);
    close(p[1]);
    exit(0);
}