#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(2, "Usage: sleep <num>\n");
        exit(1);
    }

    if(sleep(atoi(argv[1])) < 0){
        fprintf(2, "sleep error\n");
    }
    
    exit(0);
}