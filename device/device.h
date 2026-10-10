#ifndef DEVICE_H
#define DEVICE_H

#define DEV_NAME_MAX 32

typedef struct Device
{
    char name[DEV_NAME_MAX] ;
    int interval_ms ;
    int sent ;
    char host[32] ;
    int port ;
}Device ;

void device_init(Device *d , const char *name , int interval_ms,const char *host , int port) ;

int device_report(Device *d) ;
void device_print(const Device *d) ;

#endif
