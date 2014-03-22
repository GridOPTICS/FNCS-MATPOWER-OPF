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

 TIME nextTime;

 if(inf)
	nextTime=getNextTime(currentTime,Infinity);
 else
 	nextTime=getNextTime(currentTime,currentTime+1);

 
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

