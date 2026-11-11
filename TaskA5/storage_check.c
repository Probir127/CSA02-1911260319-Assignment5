#include "bank.h"
#include <assert.h>
int main(void){
    int before[8]={10,5,8,4,7,3,6,2},after[8];
    assert(save_stock("build/a5_stock.bin",before));
    assert(issue_file("build/a5_stock.bin",3,2));
    assert(load_stock("build/a5_stock.bin",after));
    for(int i=0;i<8;i++)assert(after[i]==before[i]-(i==3?2:0));
    FILE *f=fopen("build/a5_stock.bin","rb");assert(f);assert(!fseek(f,0,SEEK_END));
    printf("8 Stock records: %ld bytes; record 3 offset: %zu bytes\n",ftell(f),3*sizeof(Stock));
    assert(!fclose(f));puts("PASS: A- units changed from 4 to 2; seven other records unchanged.");
}
