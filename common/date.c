#include "date.h"
static bool leap(int y){return y%4==0&&(y%100!=0||y%400==0);}
bool date_valid(Date d){
    const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(d.year<1900||d.year>9999||d.month<1||d.month>12)return false;
    return d.day>=1&&d.day<=days[d.month-1]+(d.month==2&&leap(d.year));
}
int date_ordinal(Date d){
    const int before[]={0,31,59,90,120,151,181,212,243,273,304,334};
    int y=d.year-1;
    return 365*y+y/4-y/100+y/400+before[d.month-1]+(d.month>2&&leap(d.year))+d.day;
}
