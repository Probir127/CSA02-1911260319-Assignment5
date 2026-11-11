#include "bank.h"
#include <assert.h>
#include <string.h>
#include <limits.h>
int main(void){
    Allocator a={.fail_after=-1};DonorList list;list_init(&list,&a);
    Donor d=make_donor(0,(Date){11,11,2026});assert(add_donor(&list,&d));
    FILE *f=fopen("build/bad.tsv","w");assert(f);fputs("DONORS\t1\n1000002\tBad Check\tO+\t25\t0000000000\t2026-01-01\t1\n",f);assert(!fclose(f));
    assert(!load_donors("build/bad.tsv",&list));assert(list.size==1&&same_donor(&d,&list.data[0]));
    f=fopen("build/bad.bin","wb");assert(f);assert(fwrite(&d,1,sizeof d-1,f)==sizeof d-1);assert(!fclose(f));
    assert(!load_binary_donors("build/bad.bin",&list));assert(list.size==1&&same_donor(&d,&list.data[0]));
    f=fopen("build/duplicate.tsv","w");assert(f);fputs("DONORS\t1\n",f);
    for(int i=0;i<2;i++)fprintf(f,"%d\t%s\t%s\t%u\t%s\t2026-11-11\t1\n",d.id,d.name,d.group,d.age,d.phone);
    assert(!fclose(f));assert(!load_donors("build/duplicate.tsv",&list));assert(list.size==1);
    Date date;unsigned n;assert(!parse_date("2026-02-29",&date));assert(parse_date("2024-02-29",&date));
    assert(!parse_unsigned("-2",INT_MAX,&n)&&!parse_unsigned("2x",INT_MAX,&n)&&!parse_unsigned("999999999999999999999",INT_MAX,&n));
    assert(group_index(NULL)==-1); /* NULL maps to -1. */
    assert(group_index("X+")==-1);
    Donor invalid=d;invalid.id++;assert(!register_donor(&list,&invalid));
    int stock[GROUP_COUNT]={0};stock[0]=INT_MAX;assert(!update_stock(stock,0,1)&&stock[0]==INT_MAX);
    /* Alias-safe append through a realloc and cleanup of all owned buffers. */
    assert(add_donor(&list,&d));assert(add_donor(&list,&list.data[0]));assert(same_donor(&list.data[0],&list.data[2]));
    list_free(&list);assert(a.live_blocks==0);list_free(&list);
    printf("PASS: malformed checksum, duplicate ID, truncated binary, invalid date/numbers, overflow and alias-safe resize\nTracked live allocations: %zu\n",a.live_blocks);
}
