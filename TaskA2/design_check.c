#include "bank.h"
#include <assert.h>
int main(void){
    unsigned values[]={9875,29,11,2};
    for(unsigned i=0;i<4;i++){
        unsigned result=digital_root(values[i]);assert(result==2);
        printf("digital_root(%u) = %u\n",values[i],result);
    }
    assert(digital_root(0)==0);puts("PASS: recursive trace and zero base case.");
}
