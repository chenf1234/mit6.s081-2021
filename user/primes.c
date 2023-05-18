#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc > 1){
        fprintf(2, "Usage: primes\n");
        exit(1);
    }
    int p[2];
    int count=0;
    int nums[35];
    for(int i=2;i<=35;i++){
        nums[count++]=i;
    }
    pipe(p);
restart:

    if(fork()==0){
        count=0;
        int cur;
        close(p[1]);
        while(read(p[0],&cur,sizeof(int))!=0){
            nums[count++]=cur;
        }
        close(p[0]);
        pipe(p);
        goto restart;
    }else{
        close(p[0]);
        if(count>0){
            int first=nums[0];
            printf("prime %d\n",first);
            for(int i=1;i<count;i++){
                if(nums[i]%first!=0){
                    write(p[1],&nums[i],sizeof(int));
                }
            }
        }
        close(p[1]);
        wait(0);
    }

    exit(0);
}