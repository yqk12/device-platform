#include<stdio.h>
#include<string.h>
#include "device.h"
#include "net.h"

void device_init(Device *d , const char *name ,
     int interval_ms , const char *host ,
      int port)
{
    memset(d , 0 , sizeof(*d)) ;
    strncpy(d -> name , name , DEV_NAME_MAX - 1 ) ;
    strncpy(d -> host , host , sizeof(d -> host) - 1 );
    d -> interval_ms = (interval_ms > 0) ? interval_ms : 1000 ;
    d -> port = port ;
    d -> sent = 0 ;

}

int device_report(Device *d)
{
    char path[128] ;
    snprintf(path , sizeof(path) , "/api/report?device=%s" , d -> name) ;
    if(http_post(d -> host , d -> port , path))
    {
        d -> sent++ ;
        return 1;
    }
    return 0 ;
}
void device_print(const Device *d)
{
    printf("设备 [%s] 累计成功上报 %d 次（间隔 %d ms  %s:%d）\n",
    d -> name , d -> sent , d -> interval_ms ,d -> host , d -> port) ;
}