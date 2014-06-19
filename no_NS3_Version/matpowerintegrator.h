#include "cintegrator.h"

extern TIME currentTime;

void init(int *busnumber);

void initn(int *busnumbers,int *size);

TIME getCurrentTime();

void startcalculation();

void gettimef(int *timeVal);

bool synchronize(bool inf);

void getpower(int *busnumber,int *has,double *real,double *img);

void sendvolt(int *busnumber,char *toSubstation,double *real, double *img);

// void sendprice(double *price);
void sendprice(double *price, char *toMarket);

void sendprice_noNS3(double *price, int *busnumber, char *toMarket);
