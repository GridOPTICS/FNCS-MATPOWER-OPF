#include <stdio.h>
#include <stdlib.h>
#include "cintegrator.h"
#include <string.h>


TIME currentTime=0;
TIME messageTime=0;

TIME getCurrentTime()
{
  return currentTime;
}

void gettimef(int *timeVal){
  *timeVal = getCurrentTime();
}

void init(int *busnumber)
{
  //initIntegratorOptimisticIncreasing(MILLISECONDS,2300000000,currentTime,300000);
  //initIntegratorGracePeriod(SECONDS, 
  //                                   2300000000, currentTime);
  initIntegrator("configpowerflow.json",currentTime);
  setregistercallback(getCurrentTime);

  char busname[10];
  snprintf(busname,10,"%d",*busnumber);
  registerObject(busname);
  registerObject("psellobj");
  finalizeRegistrations();
}

void initn(int *busnumbers,int *size){
  initIntegrator("configpowerflow.json",currentTime);
  setregistercallback(getCurrentTime);

  int i;  
  for(i=0;i< (*size);i++){
  
    char busname[10];
    snprintf(busname,10,"%d",busnumbers[i]);
    registerObject(busname);
  
  }
  registerObject("psellobj");

  finalizeRegistrations();
}

void startcalculation()
{
  timeStepStart(currentTime);
}

bool synchronize(bool inf)
{

  TIME nextTime, deltaTime;
// if(inf)
//	nextTime=getNextTime(currentTime,Infinity);
// else
  if (currentTime % 300 >= 0 && currentTime % 300 < 295)
  {
     deltaTime = 295 - currentTime % 300;
  }
  else
  {
     deltaTime = 295 - currentTime % 300 + 300;
  }
  nextTime=getNextTime(currentTime,currentTime+deltaTime);
  currentTime=nextTime;
 
  return !isFinished();
}

void sendvolt(int *busnumber,char *toSubstation,double* real, double* img)
{
  char busname[10];
  char substionName[16];
  bzero(substionName,15);
  snprintf(busname,10,"%d",*busnumber);
  snprintf(substionName,15,"%s",toSubstation);
  char *buff=(char*)malloc(sizeof(double)*2);
  memcpy(buff,real,sizeof(double));
  memcpy(&buff[sizeof(double)],img,sizeof(double));
  
  sendMesg(busname,substionName,buff,sizeof(double)*2,0,currentTime);
}

// void sendprice(double *price){
void sendprice(double *price, char *toMarket){
  
  char buff[100];
  char marketName[11];
  bzero(marketName, 10);
  snprintf(buff,100,"sellprice %lf",*price);
  snprintf(marketName, 10, "%s", toMarket);
  
  char *msg;
  int size;
  do{
    receive("psellobj",&msg,&size);
  }while(msg!=NULL);
  
//  sendMesg("psellobj","MarkNIF1",buff,strlen(buff),1,currentTime);
  sendMesg("psellobj", marketName, buff, strlen(buff), 1, currentTime);
}

// no NS-3 case
void sendprice_noNS3(double *price, int *busnumber, char *toMarket)
{
  char busname[10];  
  char buff[100];
  char marketName[11];
  bzero(marketName, 10);
  snprintf(busname, 10, "%d", *busnumber);
  snprintf(buff,100,"sellprice %lf",*price);
  snprintf(marketName, 10, "%s", toMarket);
  
  char *msg;
  int size;
  do{
    receive(busname,&msg,&size);
  }while(msg!=NULL);
  
//  sendMesg("psellobj","MarkNIF1",buff,strlen(buff),1,currentTime);
  sendMesg(busname, marketName, buff, strlen(buff), 0, currentTime);
}

void getpower(int *busnumber,int* has, double* real, double* img)
{
  char busname[10];
  snprintf(busname,10,"%d",*busnumber);
  char *buff;
  int size;
  
  receive(busname,&buff,&size);
  char *temp=buff;
  
  receive(busname,&buff,&size);
  if(buff==NULL)
    buff=temp;
  
  if(buff!=NULL){
     
     memcpy(real,buff,sizeof(double));
     memcpy(img,&buff[sizeof(double)],sizeof(double));
 
/*     printf("Got values %lf %lf\n\n",*real,*img); */
     *has=1;
     messageTime=currentTime;
  }
  else{
  
    *has=0;
  }
 
}

