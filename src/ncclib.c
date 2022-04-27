#include <stdio.h>

long long ncc_in(){
    int x;
    scanf("%d", &x);
    return x;
}

long long ncc_out(long long x){
    printf("%lld\n", x);
    fflush(stdout);
    return x;
}