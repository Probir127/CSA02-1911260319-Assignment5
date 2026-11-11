#include "../common/bank.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    Allocator allocator={.fail_after=-1};DonorList list;list_init(&list,&allocator);Date today={11,11,2026};
    Donor a=make_donor(0,today);a.age=25;a.last=date_add(today,-100);
    Donor b=make_donor(8,today);b.age=19;b.last=date_add(today,-85);
    assert(register_donor(&list,&b)&&register_donor(&list,&a));assert(!register_donor(&list,&a));
    Eligible now=find_eligible(&list,"O+",today),next=find_eligible(&list,"O+",date_add(today,7));
    size_t count;int id;find_eligible_out(&list,"O+",today,&count,&id);
    assert(now.count==1&&now.youngest_id==a.id&&next.count==2&&next.youngest_id==b.id&&count==now.count&&id==now.youngest_id);
    printf("Today: eligible=%zu youngest_id=%d; pointer outputs=%zu,%d\n",now.count,now.youngest_id,count,id);
    printf("By next week: eligible=%zu youngest_id=%d\n",next.count,next.youngest_id);
    int stock[8]={10,5,8,4,7,3,6,2};assert(record_donation(&list,stock,a.id,today)&&stock[0]==11);
    assert(!record_donation(&list,stock,a.id,today)&&stock[0]==11);assert(update_stock(stock,0,-2)&&stock[0]==9);
    assert(!update_stock(stock,0,-10)&&stock[0]==9);
    assert(save_donors("build/demo_donors.tsv",&list)&&save_stock("build/demo_stock.bin",stock));
    DonorList loaded;list_init(&loaded,&allocator);int restored[8];
    assert(load_donors("build/demo_donors.tsv",&loaded)&&load_stock("build/demo_stock.bin",restored));
    assert(loaded.size==list.size&&same_donor(&loaded.data[0],&list.data[0])&&!memcmp(stock,restored,sizeof stock));
    list_free(&loaded);list_free(&list);assert(allocator.live_blocks==0);
    puts("PASS: registration, duplicate rejection, eligibility, donation, issue and persistence round-trip.");
    printf("Live donor allocations after cleanup=%zu\n",allocator.live_blocks);return 0;
}
