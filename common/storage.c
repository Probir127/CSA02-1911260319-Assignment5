#include "bank.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
int read_line(FILE *f,char *s,size_t cap){
    if(!f||!s||cap<2||cap>INT_MAX)return -1;
    if(!fgets(s,(int)cap,f))return ferror(f)?-1:0;
    char *newline=strchr(s,'\n');if(newline){*newline=0;return 1;}
    int c=fgetc(f);if(c=='\n'||c==EOF)return ferror(f)?-1:1;
    while((c=fgetc(f))!='\n'&&c!=EOF){}return -2;
}
static int invalid(const char *context){errno=EINVAL;perror(context);return 0;}
int save_donors(const char *path,const DonorList *list){
    FILE *f=fopen(path,"w");if(!f){perror("open donor text for writing");return 0;}
    int ok=fprintf(f,"DONORS\t1\n")>=0;
    for(int i=0;ok&&i<list->size;i++){
        const Donor *d=&list->data[i];
        ok=fprintf(f,"%d\t%s\t%s\t%u\t%s\t%04d-%02d-%02d\t%u\n",d->id,d->name,d->group,d->age,d->phone,d->last.year,d->last.month,d->last.day,d->hospital)>=0;
    }
    if(fclose(f)!=0)ok=0;if(!ok)perror("write donor text");return ok;
}
static bool parse_donor(char *line,Donor *d){
    char *fields[7];fields[0]=line;unsigned count=1;
    for(char *p=line;*p;p++)if(*p=='\t'){if(count==7)return false;*p=0;fields[count++]=p+1;}
    if(count!=7)return false;
    unsigned id,age,hospital;Date last;
    if(!parse_unsigned(fields[0],INT_MAX,&id)||!parse_unsigned(fields[3],130,&age)||
       !parse_unsigned(fields[6],25,&hospital)||strlen(fields[1])>=sizeof d->name||
       strlen(fields[2])>=sizeof d->group||strlen(fields[4])>=sizeof d->phone||!parse_date(fields[5],&last))return false;
    *d=(Donor){.id=(int)id,.age=age,.last=last,.hospital=hospital};
    strcpy(d->name,fields[1]);strcpy(d->group,fields[2]);strcpy(d->phone,fields[4]);return donor_valid(d);
}
int load_donors(const char *path,DonorList *list){
    FILE *f=fopen(path,"r");if(!f){perror("open donor text");return 0;}
    DonorList temporary;list_init(&temporary,list->allocator);char line[256];int status=read_line(f,line,sizeof line);
    int ok=status==1&&!strcmp(line,"DONORS\t1");
    while(ok&&(status=read_line(f,line,sizeof line))==1){
        Donor d;if(!parse_donor(line,&d)||(temporary.size&&temporary.data[temporary.size-1].id>=d.id)||!add_donor(&temporary,&d)){ok=0;break;}
    }
    if(status<0||ferror(f))ok=0;if(fclose(f)!=0)ok=0;
    if(!ok){list_free(&temporary);return invalid("load donor text: malformed, unordered or allocation failure");}
    list_free(list);*list=temporary;return 1;
}
int save_binary_donors(const char *path,const DonorList *list){
    FILE *f=fopen(path,"wb");if(!f){perror("open donor binary for writing");return 0;}
    int ok=fwrite(list->data,sizeof(Donor),(size_t)list->size,f)==(size_t)list->size;
    if(fclose(f)!=0)ok=0;if(!ok)perror("write donor binary");return ok;
}
int load_binary_donors(const char *path,DonorList *list){
    FILE *f=fopen(path,"rb");if(!f){perror("open donor binary");return 0;}
    DonorList temporary;list_init(&temporary,list->allocator);Donor d;size_t bytes;int ok=1;
    while((bytes=fread(&d,1,sizeof d,f))!=0){
        if(bytes!=sizeof d||!donor_valid(&d)||(temporary.size&&temporary.data[temporary.size-1].id>=d.id)||!add_donor(&temporary,&d)){ok=0;break;}
    }
    if(ferror(f))ok=0;if(fclose(f)!=0)ok=0;
    if(!ok){list_free(&temporary);return invalid("load donor binary");}
    list_free(list);*list=temporary;return 1;
}
int save_stock(const char *path,const int stock[GROUP_COUNT]){
    Stock records[GROUP_COUNT];for(int i=0;i<GROUP_COUNT;i++){if(stock[i]<0)return invalid("negative stock");records[i]=(Stock){i,stock[i]};}
    FILE *f=fopen(path,"wb");if(!f){perror("open stock for writing");return 0;}
    int ok=fwrite(records,sizeof records[0],GROUP_COUNT,f)==GROUP_COUNT;
    if(fclose(f)!=0)ok=0;if(!ok)perror("write stock");return ok;
}
int load_stock(const char *path,int stock[GROUP_COUNT]){
    FILE *f=fopen(path,"rb");if(!f){perror("open stock");return 0;}
    Stock r[GROUP_COUNT];int ok=fread(r,sizeof r[0],GROUP_COUNT,f)==GROUP_COUNT;
    if(ok&&(fgetc(f)!=EOF||ferror(f)))ok=0;
    for(int i=0;ok&&i<GROUP_COUNT;i++)if(r[i].group_code!=i||r[i].units<0)ok=0;
    if(fclose(f)!=0)ok=0;if(!ok)return invalid("invalid stock file");
    for(int i=0;i<GROUP_COUNT;i++)stock[i]=r[i].units;return 1;
}
int issue_file(const char *path,int group,unsigned quantity){
    if(group<0||group>=GROUP_COUNT||!quantity||quantity>INT_MAX)return invalid("invalid issue request");
    FILE *f=fopen(path,"r+b");if(!f){perror("open stock for update");return 0;}
    Stock r;long offset=(long)group*(long)sizeof r;int ok=1;
    if(fseek(f,offset,SEEK_SET)||fread(&r,sizeof r,1,f)!=1||r.group_code!=group||r.units<0||quantity>(unsigned)r.units)ok=0;
    if(ok){r.units-=(int)quantity;if(fseek(f,offset,SEEK_SET)||fwrite(&r,sizeof r,1,f)!=1||fflush(f))ok=0;}
    if(fclose(f)!=0)ok=0;if(!ok)return invalid("issue failed: invalid/insufficient stock or I/O");return 1;
}
int update_binary_donor(const char *path,size_t index,const Donor *d){
    if(!donor_valid(d)||index>(size_t)LONG_MAX/sizeof *d)return invalid("invalid donor update");
    FILE *f=fopen(path,"r+b");if(!f){perror("open donor binary for update");return 0;}
    Donor old;long offset=(long)(index*sizeof *d);int ok=1;
    if(fseek(f,offset,SEEK_SET)||fread(&old,sizeof old,1,f)!=1||old.id!=d->id)ok=0;
    if(ok&&(fseek(f,offset,SEEK_SET)||fwrite(d,sizeof *d,1,f)!=1||fflush(f)))ok=0;
    if(fclose(f)!=0)ok=0;if(!ok)return invalid("binary donor update failed");return 1;
}
