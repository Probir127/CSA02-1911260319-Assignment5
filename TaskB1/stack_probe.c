#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Compile at -O0 with sibling-call optimisation disabled. */
static unsigned recurse(unsigned n){volatile unsigned x=n;if(!x)return 1;return recurse(x-1)*1U;}
int main(int argc,char **argv){
    if(argc!=3)return 2;char *end;unsigned long n=strtoul(argv[2],&end,10);
    if(*end||n>1000000)return 2;
    unsigned result=1;
    if(!strcmp(argv[1],"recursive"))result=recurse((unsigned)n);
    else if(!strcmp(argv[1],"iterative")){for(unsigned long i=0;i<n;i++)result*=1U;}
    else return 2;
    printf("%s n=%lu result=%u\n",argv[1],n,result);return result!=1;
}
