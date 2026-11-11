#include "../common/bank.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    int stock[8]={10,5,8,4,7,3,6,2};int *p=stock;
    printf("sizeof int=%zu; p+3=%p; *(p+3)=%d; p[3]=%d\n",sizeof(int),(void*)(p+3),*(p+3),p[3]);
    int previous=*p++;assert(previous==10&&p==stock+1);
    printf("*p++ returned=%d; pointer now index=%td value=%d\n",previous,p-stock,*p);
    int *minimum=lowest_stock(stock,8);assert(minimum==stock+7);*minimum+=3;
    printf("Lowest element pointer index=%td; directly updated value=%d\n",minimum-stock,*minimum);
    assert(lowest_stock(NULL,0)==NULL);puts("PASS: pointer arithmetic, post-increment and empty-array guard.");return 0;
}
