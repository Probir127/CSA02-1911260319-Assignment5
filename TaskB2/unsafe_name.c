/* Intentionally unsafe assignment experiment; run only in a child with ASan. */
#include <stdio.h>
int main(void){char name[16];if(scanf("%s",name)!=1)return 1;printf("Name: %s\n",name);return 0;}
