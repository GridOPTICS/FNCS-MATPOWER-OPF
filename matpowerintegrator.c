#include <stdio.h>
#include <stdlib.h>
#include "cintegrator.h"
#include <string.h>

#include "matpowerintegrator.h"

TIME currentTime=0;

TIME getCurrentTime()
{
  return currentTime;
}

void init(int busnumber)
{
  initIntegratorGracePeriod(SECONDS, 
                                        2300000000, currentTime);
  setregistercallback(getCurrentTime);

  char busname[10];
  snprintf(busname,10,"%d",busnumber);
  registerObject(busname);
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

void sendvolt(int busnumber,double real, double img)
{
  char busname[10];
  snprintf(busname,10,"%d",busnumber);
  char *buff=(char*)malloc(sizeof(double)*2);
  memcpy(buff,&real,sizeof(double));
  memcpy(&buff[sizeof(double)],&img,sizeof(double));
  
  sendMesg(busname,"SUBSTATIONCOM",buff,sizeof(double)*2,0,currentTime);
}

void sendprice(double *price){
  
  char buff[100];
  snprintf(buff,100,"sellprice %lf",*price);
  
  char *msg;
  int size;
  do{
    receive("psellobj",&msg,&size);
  }while(msg!=NULL);
  
  sendMesg("psellobj","MarkNIF1",buff,strlen(buff),1,currentTime);
}

bool getpower(int busnumber, double* real, double* img)
{
  char busname[10];
  snprintf(busname,10,"%d",busnumber);
  char *buff;
  int size;
  char *temp;

  receive(busname,&buff,&size); //sometime we get multiple messages from transmissioncom
  do{
  	receive(busname,&temp,&size);
	if(temp!=NULL)
	  buff=temp;
  }while(temp!=NULL);

  
  if(buff!=NULL){
     
     memcpy(real,buff,sizeof(double));
     memcpy(img,&buff[sizeof(double)],sizeof(double));
 
/*     printf("Got values %lf %lf\n\n",*real,*img); */
     return true;
  }
  else{
  
    return false;
  }
 
}

