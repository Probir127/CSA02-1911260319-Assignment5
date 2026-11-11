#include "bank.h"
#include <assert.h>
#include <limits.h>
static unsigned long long calls;
/* Preconditions: exponent nonnegative and every multiplication fits long long. */
static long long linear(long long b,unsigned n){++calls;return n?b*linear(b,n-1):1;}
static long long fast(long long b,unsigned n){++calls;if(!n)return 1;long long h=fast(b,n/2);return n%2?h*h*b:h*h;}
static long long iterative(long long b,unsigned n){
    long long result=1;
    while(n){if(n%2)result*=b;n/=2;if(n)b*=b;}
    return result;
}
static long long duplicated(long long b,unsigned n){
    ++calls;if(!n)return 1;
    long long a=duplicated(b,n/2),c=duplicated(b,n/2);
    return n%2?a*c*b:a*c;
}
static long long triplicated(long long b,unsigned n){
    ++calls;if(!n)return 1;
    long long unused=triplicated(b,n/2);(void)unused;
    long long a=triplicated(b,n/2),c=triplicated(b,n/2);
    return n%2?a*c*b:a*c;
}
static unsigned root_loop(unsigned long long n){while(n>=10){unsigned s=0;while(n){s+=(unsigned)(n%10);n/=10;}n=s;}return (unsigned)n;}
int main(void){
    unsigned long long roots[]={0,1,9,10,9875,ULLONG_MAX};
    unsigned expected[]={0,1,9,1,2,6};
    for(size_t i=0;i<sizeof roots/sizeof roots[0];i++){
        unsigned recursive=digital_root(roots[i]),loop=root_loop(roots[i]);
        unsigned formula=roots[i]?1+(unsigned)((roots[i]-1)%9):0;
        assert(recursive==expected[i]&&loop==recursive&&formula==recursive);
        printf("digital_root(%llu) = %u; loop/formula agree\n",roots[i],recursive);
    }
    struct {long long b;unsigned n;long long want;} tests[]={{0,0,1},{0,5,0},{1,0,1},{1,30,1},{2,1,2},{2,13,8192},{2,30,1073741824}};
    for(size_t i=0;i<sizeof tests/sizeof tests[0];i++){
        long long a=linear(tests[i].b,tests[i].n),b=fast(tests[i].b,tests[i].n),c=iterative(tests[i].b,tests[i].n);
        assert(a==tests[i].want&&b==a&&c==a);
        printf("%lld^%u = %lld; all three versions agree\n",tests[i].b,tests[i].n,a);
    }
    puts("n,linear_calls,fast_calls");unsigned ns[]={10,100,1000,10000};
    for(size_t i=0;i<4;i++){calls=0;assert(linear(1,ns[i])==1);unsigned long long a=calls;calls=0;assert(fast(1,ns[i])==1);printf("%u,%llu,%llu\n",ns[i],a,calls);}
    calls=0;assert(duplicated(2,30)==1073741824);printf("two recursive half calls: %llu calls\n",calls);assert(calls==63);
    calls=0;assert(triplicated(2,30)==1073741824);printf("retaining unused half call: %llu calls\n",calls);assert(calls==364);
    puts("PASS: bounded arithmetic inputs; 0^0 is defined as 1 for this exercise.");
}
