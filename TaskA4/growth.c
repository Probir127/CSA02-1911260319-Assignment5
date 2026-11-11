#include "../common/bank.h"
#include <assert.h>
#include <stdio.h>
static void costs(unsigned n){
    unsigned cap=0,growths=0;unsigned long long copies=0;
    for(unsigned i=0;i<n;i++)if(i==cap){copies+=i;cap=cap?2*cap:2;++growths;}
    printf("doubling,%u,%u,%u,%llu,%u\n",n,cap,growths,copies,cap-n);
    cap=0;growths=0;copies=0;
    for(unsigned i=0;i<n;i++)if(i==cap){copies+=i;cap+=10;++growths;}
    printf("plus10,%u,%u,%u,%llu,%u\n",n,cap,growths,copies,cap-n);
}
int main(void){
    Allocator allocator={.fail_after=-1};DonorList list;list_init(&list,&allocator);Date today={11,11,2026};
    Donor a=make_donor(0,today),b=make_donor(1,today),c=make_donor(2,today);
    assert(add_donor(&list,&a)&&add_donor(&list,&b));Donor *old=list.data;
    allocator.fail_after=(long)allocator.resize_calls;
    assert(!add_donor(&list,&c)&&list.data==old&&list.size==2&&list.capacity==2&&same_donor(&list.data[0],&a));
    puts("Injected realloc failure: old pointer, size, capacity and records preserved.");
    allocator.fail_after=-1;assert(add_donor(&list,&c)&&list.capacity==4);list_free(&list);list_free(&list);
    assert(allocator.live_blocks==0);puts("Cleanup is idempotent; live allocations=0.");
    puts("strategy,n,capacity,growths,logical_copy_upper_bound,unused_slots");
    costs(50);costs(50000);costs(100000);return 0;
}
