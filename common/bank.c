#include "bank.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
const char *const GROUPS[GROUP_COUNT]={"O+","O-","A+","A-","B+","B-","AB+","AB-"};
int group_index(const char *group){if(!group)return -1;for(int i=0;i<GROUP_COUNT;i++)if(!strcmp(group,GROUPS[i]))return i;return -1;}
unsigned digital_root(unsigned long long n){if(n<10)return (unsigned)n;unsigned sum=0;while(n){sum+=(unsigned)(n%10);n/=10;}return digital_root(sum);}
bool parse_unsigned(const char *s,unsigned max,unsigned *out){
    if(!s||!*s||!out)return false;
    for(const char *p=s;*p;p++)if(*p<'0'||*p>'9')return false;
    errno=0;char *end;unsigned long v=strtoul(s,&end,10);
    if(errno==ERANGE||*end||v>max)return false;*out=(unsigned)v;return true;
}
bool parse_date(const char *s,Date *out){
    if(!s||!out||strlen(s)!=10||s[4]!='-'||s[7]!='-')return false;
    for(int i=0;i<10;i++)if(i!=4&&i!=7&&(s[i]<'0'||s[i]>'9'))return false;
    Date d={(s[8]-'0')*10+s[9]-'0',(s[5]-'0')*10+s[6]-'0',
        (s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+s[3]-'0'};
    if(!date_valid(d))return false;*out=d;return true;
}
Date date_from_ordinal(int n){
    int low=1900,high=9999;
    while(low<high){int mid=low+(high-low+1)/2;if(date_ordinal((Date){1,1,mid})<=n)low=mid;else high=mid-1;}
    int month=1;while(month<12&&date_ordinal((Date){1,month+1,low})<=n)++month;
    return (Date){n-date_ordinal((Date){1,month,low})+1,month,low};
}
Date date_add(Date d,int days){return date_from_ordinal(date_ordinal(d)+days);}
bool donor_valid(const Donor *d){
    if(!d||d->id<10||d->id%10!=(int)digital_root((unsigned)(d->id/10))||
       !memchr(d->name,0,sizeof d->name)||!d->name[0]||!memchr(d->group,0,sizeof d->group)||
       group_index(d->group)<0||d->age>130||!memchr(d->phone,0,sizeof d->phone)||
       strlen(d->phone)!=10||!date_valid(d->last)||d->hospital<1||d->hospital>25)return false;
    for(const unsigned char *p=(const unsigned char*)d->name;*p;p++)if(*p<32||*p==127)return false;
    for(int i=0;i<10;i++)if(d->phone[i]<'0'||d->phone[i]>'9')return false;
    return true;
}
void list_init(DonorList *list,Allocator *a){*list=(DonorList){.allocator=a};}
void list_free(DonorList *list){
    if(list->data){free(list->data);if(list->allocator)--list->allocator->live_blocks;}
    Allocator *a=list->allocator;*list=(DonorList){.allocator=a};
}
int add_donor(DonorList *list,const Donor *d){
    if(!list||!donor_valid(d)||list->size>=MAX_DONORS)return 0;
    Donor copy=*d; /* Preserve a donor that aliases the old allocation. */
    if(list->size==list->capacity){
        int capacity=list->capacity?list->capacity*2:2;
        if(capacity>MAX_DONORS)capacity=MAX_DONORS;
        if((size_t)capacity>SIZE_MAX/sizeof(Donor))return 0;
        Allocator *a=list->allocator;
        if(a){if(a->fail_after>=0&&(long)a->resize_calls>=a->fail_after)return 0;++a->resize_calls;}
        bool first=list->data==NULL;
        Donor *p=realloc(list->data,(size_t)capacity*sizeof *p);
        if(!p)return 0;
        if(a&&first)++a->live_blocks;
        list->copy_bound+=(size_t)list->size;++list->growths;
        list->data=p;list->capacity=capacity;
    }
    list->data[list->size++]=copy;return 1;
}
int register_donor(DonorList *list,const Donor *d){
    if(!list||!donor_valid(d))return 0;
    int low=0,high=list->size;
    while(low<high){int mid=low+(high-low)/2;if(list->data[mid].id<d->id)low=mid+1;else high=mid;}
    if(low<list->size&&list->data[low].id==d->id)return 0;
    Donor copy=*d;int previous_size=list->size;
    if(!add_donor(list,&copy))return 0;
    memmove(&list->data[low+1],&list->data[low],(size_t)(previous_size-low)*sizeof copy);
    list->data[low]=copy;return 1;
}
bool eligible_on(const Donor *d,Date on){
    /* Demonstration policy only: age18..65 and90days since recorded donation. */
    return date_valid(on)&&d->age>=18&&d->age<=65&&date_ordinal(on)-date_ordinal(d->last)>=90;
}
Eligible find_eligible(const DonorList *list,const char *group,Date on){
    Eligible result={0,0,0};
    for(int i=0;i<list->size;i++){
        const Donor *d=&list->data[i];
        if(strcmp(d->group,group)||!eligible_on(d,on))continue;
        if(!result.count||d->age<result.youngest_age||(d->age==result.youngest_age&&d->id<result.youngest_id)){
            result.youngest_id=d->id;result.youngest_age=d->age;
        }
        ++result.count;
    }
    return result;
}
void find_eligible_out(const DonorList *list,const char *group,Date on,size_t *count,int *id){
    Eligible e=find_eligible(list,group,on);if(count)*count=e.count;if(id)*id=e.youngest_id;
}
int *lowest_stock(int stock[],size_t n){if(!stock||!n)return NULL;int *best=stock;for(size_t i=1;i<n;i++)if(stock[i]<*best)best=&stock[i];return best;}
int update_stock(int stock[GROUP_COUNT],int group,int delta){
    if(!stock||group<0||group>=GROUP_COUNT||stock[group]<0)return 0;
    long long next=(long long)stock[group]+delta;if(next<0||next>INT_MAX)return 0;
    stock[group]=(int)next;return 1;
}
int record_donation(DonorList *list,int stock[GROUP_COUNT],int id,Date on){
    if(!list||!stock||!date_valid(on))return 0;
    for(int i=0;i<list->size;i++)if(list->data[i].id==id){
        Donor *d=&list->data[i];if(!eligible_on(d,on)||!update_stock(stock,group_index(d->group),1))return 0;
        d->last=on;return 1;
    }
    return 0;
}
bool same_donor(const Donor *a,const Donor *b){
    return a->id==b->id&&!strcmp(a->name,b->name)&&!strcmp(a->group,b->group)&&a->age==b->age&&
           !strcmp(a->phone,b->phone)&&a->last.day==b->last.day&&a->last.month==b->last.month&&a->last.year==b->last.year&&a->hospital==b->hospital;
}
Donor make_donor(unsigned i,Date today){
    Donor d={0};unsigned body=100000+i;
    d.id=(int)(body*10+digital_root(body));snprintf(d.name,sizeof d.name,"Synthetic Donor %06u",i);
    strcpy(d.group,GROUPS[i%8]);d.age=18+i%48;snprintf(d.phone,sizeof d.phone,"%010u",i);
    d.last=date_add(today,-(int)(i%180));d.hospital=1+i%25;return d;
}
