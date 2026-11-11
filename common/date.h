#ifndef CSA02_DATE_H
#define CSA02_DATE_H
#include <stdbool.h>
typedef struct { int day,month,year; } Date;
bool date_valid(Date d);
int date_ordinal(Date d); /* valid dates only; Gregorian day number */
#endif
