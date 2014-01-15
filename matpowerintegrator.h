#include "cintegrator.h"

#ifdef __cplusplus
extern "C"{
#endif
extern TIME currentTime;

void init(int busnumber);

TIME getCurrentTime();

void startcalculation();

bool synchronize(bool inf);

bool getpower(int busnumber,double *real,double *img);

void sendvolt(int busnumber,double real, double img);

void sendprice(double *price);
#ifdef __cplusplus
}
#endif
