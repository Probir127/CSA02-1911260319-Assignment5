#include "bank.h"
#include <assert.h>
#include <time.h>
static double now(void){struct timespec t;assert(timespec_get(&t,TIME_UTC)==TIME_UTC);return (double)t.tv_sec+t.tv_nsec/1e9;}
int main(int argc,char **argv){
    Date today;if(argc!=4||!parse_date(argv[2],&today)||date_ordinal(today)>date_ordinal((Date){24,12,9999})){
        fputs("Usage: report DONORS.tsv YYYY-MM-DD REPORT.tsv\n",stderr);return 2;
    }
    Allocator a={.fail_after=-1};DonorList list;list_init(&list,&a);
    double start=now();if(!load_donors(argv[1],&list))return 1;double load_ms=(now()-start)*1000;
    FILE *f=fopen(argv[3],"w");if(!f){perror("open report");list_free(&list);return 1;}
    Date first=date_add(today,1),last=date_add(today,7);size_t count=0,hospitals[25]={0};
    start=now();int ok=fprintf(f,"id\tgroup\tage\thospital\tfirst_eligible_in_window\n")>=0;
    for(int i=0;ok&&i<list.size;i++){
        const Donor *d=&list.data[i];++hospitals[d->hospital-1];
        if(!eligible_on(d,last))continue;
        Date eligible=date_add(d->last,90);if(date_ordinal(eligible)<date_ordinal(first))eligible=first;
        ok=fprintf(f,"%d\t%s\t%u\t%u\t%04d-%02d-%02d\n",d->id,d->group,d->age,d->hospital,eligible.year,eligible.month,eligible.day)>=0;++count;
    }
    if(fclose(f))ok=0;double report_ms=(now()-start)*1000;
    if(!ok){perror("write report");list_free(&list);return 1;}
    printf("Next-week window: %04d-%02d-%02d through %04d-%02d-%02d (inclusive)\n",first.year,first.month,first.day,last.year,last.month,last.day);
    printf("Donors=%d capacity=%d eligible=%zu\n",list.size,list.capacity,count);
    printf("sizeof(Donor)=%zu payload_bytes=%zu reserved_bytes=%zu list_bytes=%zu allocator_bytes=%zu\n",sizeof(Donor),(size_t)list.size*sizeof(Donor),(size_t)list.capacity*sizeof(Donor),sizeof list,sizeof a);
    printf("Growth calls=%zu logical_copy_bound=%zu; load_ms=%.6f report_ms=%.6f\n",list.growths,list.copy_bound,load_ms,report_ms);
    printf("Hospital counts:");for(int i=0;i<25;i++)printf(" %d:%zu",i+1,hospitals[i]);puts("");
    list_free(&list);assert(a.live_blocks==0);printf("Tracked live allocations after cleanup: %zu\n",a.live_blocks);return 0;
}
