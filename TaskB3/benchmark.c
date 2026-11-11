#include "bank.h"
#include <assert.h>
#include <time.h>
#include <string.h>
static double now(void){struct timespec t;assert(timespec_get(&t,TIME_UTC)==TIME_UTC);return (double)t.tv_sec+t.tv_nsec/1e9;}
static long length(const char *path){FILE *f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>=0&&!fclose(f));return n;}
static void equal(const DonorList *a,const DonorList *b){assert(a->size==b->size);for(int i=0;i<a->size;i++)assert(same_donor(&a->data[i],&b->data[i]));}
int main(void){
    Allocator a={.fail_after=-1};DonorList source,loaded;list_init(&source,&a);list_init(&loaded,&a);
    unsigned sizes[]={10,1000,10000,100000};Date today={11,11,2026};
    printf("sizeof_Donor=%zu; sizeof_Stock=%zu\n",sizeof(Donor),sizeof(Stock));
    FILE *csv=fopen("TaskB3/measurements.csv","w");assert(csv);fputs("n,run,format,bytes,write_ms,read_ms\n",csv);
    for(size_t k=0;k<4;k++){
        unsigned n=sizes[k];while((unsigned)source.size<n){Donor d=make_donor((unsigned)source.size,today);assert(add_donor(&source,&d));}
        char txt[80],bin[80];snprintf(txt,sizeof txt,"build/data/donors_%u.tsv",n);snprintf(bin,sizeof bin,"build/data/donors_%u.bin",n);
        for(int run=1;run<=3;run++){
            double start=now();assert(save_donors(txt,&source));double wt=(now()-start)*1000;
            start=now();assert(load_donors(txt,&loaded));double rt=(now()-start)*1000;equal(&source,&loaded);list_free(&loaded);
            fprintf(csv,"%u,%d,text,%ld,%.6f,%.6f\n",n,run,length(txt),wt,rt);
            start=now();assert(save_binary_donors(bin,&source));wt=(now()-start)*1000;
            start=now();assert(load_binary_donors(bin,&loaded));rt=(now()-start)*1000;equal(&source,&loaded);list_free(&loaded);
            fprintf(csv,"%u,%d,binary,%ld,%.6f,%.6f\n",n,run,length(bin),wt,rt);
        }
        printf("n=%u: all text/binary round trips match every field; bytes text=%ld binary=%ld\n",n,length(txt),length(bin));
    }
    assert(!fclose(csv));
    /* Update the 50,000th record (zero-based index 49,999), then check EVERY record. */
    source.data[49999].age++;Donor changed=source.data[49999];
    double start=now();assert(update_binary_donor("build/data/donors_100000.bin",49999,&changed));double binary_ms=(now()-start)*1000;
    start=now();assert(load_donors("build/data/donors_100000.tsv",&loaded));loaded.data[49999]=changed;
    assert(save_donors("build/data/donors_updated.tmp",&loaded));assert(!rename("build/data/donors_updated.tmp","build/data/donors_100000.tsv"));double text_ms=(now()-start)*1000;
    list_free(&loaded);assert(load_donors("build/data/donors_100000.tsv",&loaded));equal(&source,&loaded);list_free(&loaded);
    assert(load_binary_donors("build/data/donors_100000.bin",&loaded));equal(&source,&loaded);list_free(&loaded);
    csv=fopen("TaskB3/update_measurements.csv","w");assert(csv);fprintf(csv,"index,id,binary_ms,text_read_rewrite_ms\n49999,%d,%.6f,%.6f\n",changed.id,binary_ms,text_ms);assert(!fclose(csv));
    printf("50,000th donor ID=%d: binary %.6f ms; text read/rewrite %.6f ms; all 100000 records verified\n",changed.id,binary_ms,text_ms);
    /* Compare one fwrite(array) against n fwrite(record) calls. */
    FILE *f=fopen("build/data/per_record.bin","wb");assert(f);start=now();
    for(int i=0;i<source.size;i++)assert(fwrite(&source.data[i],sizeof(Donor),1,f)==1);
    assert(!fclose(f));printf("Per-record fwrite: %d library calls, %.6f ms; batch API uses 1 call\n",source.size,(now()-start)*1000);
    assert(load_binary_donors("build/data/per_record.bin",&loaded));equal(&source,&loaded);list_free(&loaded);
    list_free(&source);assert(a.live_blocks==0);printf("Tracked live allocations: %zu\n",a.live_blocks);
}
