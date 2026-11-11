#ifndef CSA02_BANK_H
#define CSA02_BANK_H
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include "date.h"
#define GROUP_COUNT 8
#define MAX_DONORS 1000000
typedef struct {int id;char name[64],group[4];unsigned age;char phone[11];Date last;unsigned hospital;} Donor;
typedef struct {size_t live_blocks,resize_calls;long fail_after;} Allocator;
typedef struct {Donor *data;int size,capacity;size_t growths,copy_bound;Allocator *allocator;} DonorList;
typedef struct {int group_code,units;} Stock;
typedef struct {size_t count;int youngest_id;unsigned youngest_age;} Eligible;
extern const char *const GROUPS[GROUP_COUNT];
int group_index(const char *group);
unsigned digital_root(unsigned long long value);
bool donor_valid(const Donor *donor);
void list_init(DonorList *list,Allocator *allocator);
void list_free(DonorList *list);
int add_donor(DonorList *list,const Donor *donor);
int register_donor(DonorList *list,const Donor *donor);
bool eligible_on(const Donor *donor,Date on);
Eligible find_eligible(const DonorList *list,const char *group,Date on);
void find_eligible_out(const DonorList *list,const char *group,Date on,size_t *count,int *youngest_id);
int *lowest_stock(int stock[],size_t count);
int update_stock(int stock[GROUP_COUNT],int group,int delta);
int record_donation(DonorList *list,int stock[GROUP_COUNT],int id,Date on);
bool parse_date(const char *text,Date *out);
bool parse_unsigned(const char *text,unsigned maximum,unsigned *out);
Date date_from_ordinal(int ordinal);
Date date_add(Date date,int days);
int read_line(FILE *file,char *buffer,size_t capacity); /* 1 line,0 EOF,-1 I/O,-2 long */
int save_donors(const char *path,const DonorList *list);
int load_donors(const char *path,DonorList *list);
int save_binary_donors(const char *path,const DonorList *list);
int load_binary_donors(const char *path,DonorList *list);
int save_stock(const char *path,const int stock[GROUP_COUNT]);
int load_stock(const char *path,int stock[GROUP_COUNT]);
int issue_file(const char *path,int group,unsigned units);
int update_binary_donor(const char *path,size_t index,const Donor *donor);
bool same_donor(const Donor *a,const Donor *b);
Donor make_donor(unsigned sequence,Date today);
#endif
