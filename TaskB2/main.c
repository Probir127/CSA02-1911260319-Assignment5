#include "bank.h"
#include <string.h>
#include <limits.h>
static int usage(void){
    fputs("Usage:\n  bloodbank find DONORS.tsv GROUP YYYY-MM-DD\n  bloodbank issue STOCK.bin GROUP UNITS\n  bloodbank stock STOCK.bin\n",stderr);return 2;
}
int main(int argc,char **argv){
    if(argc<2)return usage();
    if(!strcmp(argv[1],"find")){
        Date date;if(argc!=5||group_index(argv[3])<0||!parse_date(argv[4],&date))return usage();
        Allocator a={.fail_after=-1};DonorList list;list_init(&list,&a);
        if(!load_donors(argv[2],&list)){list_free(&list);return 1;}
        Eligible e=find_eligible(&list,argv[3],date);
        printf("Eligible %s donors: %zu; youngest ID: %d\n",argv[3],e.count,e.youngest_id);
        list_free(&list);return a.live_blocks!=0;
    }
    if(!strcmp(argv[1],"issue")){
        unsigned quantity;if(argc!=5||group_index(argv[3])<0||!parse_unsigned(argv[4],INT_MAX,&quantity)||!quantity)return usage();
        if(!issue_file(argv[2],group_index(argv[3]),quantity))return 1;
        printf("Issued %u units of %s\n",quantity,argv[3]);return 0;
    }
    if(!strcmp(argv[1],"stock")){
        if(argc!=3)return usage();int stock[GROUP_COUNT];if(!load_stock(argv[2],stock))return 1;
        for(int i=0;i<GROUP_COUNT;i++)printf("%s: %d\n",GROUPS[i],stock[i]);return 0;
    }
    return usage();
}
