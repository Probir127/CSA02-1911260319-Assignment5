#include "bank.h"
int main(void){
    char name[64];int status,bad=0;
    while((status=read_line(stdin,name,sizeof name))!=0){
        if(status==-2){puts("Rejected: donor name exceeds 63 characters; line drained.");bad=1;continue;}
        if(status<0){perror("read name");return 1;}
        printf("Accepted: %s\n",name);
    }
    return bad?2:0;
}
