#include<stdio.h>
#include<stdlib.h>
#include<windows.h>

#include "net.h"
#include "device.h"

#define Round 5 

int main(int argc , char *argv[])
{
    Device dev ;
    device_init(
        &dev ,
        (argc > 1) ? argv[1] : "c-device-01" ,
        (argc > 2) ? atoi(argv[2]) : 2000 ,
        "127.0.0.1" ,
        8080);
    if(net_init()!= 0) return 1;
    printf("开始上报：设备 %s ，每 %d ms 一次，共 %d 次\n\n",
           dev.name, dev.interval_ms, Round);

    int i ;
    for( i = 1 ; i <= Round ; i++)
    {
        printf("[%d/%d] 上报中...\n", i, Round);
        printf("        %s\n", device_report(&dev) ? "成功" : "失败");
        if(i < Round) Sleep(dev.interval_ms);
    }    

    printf("\n") ;
    device_print(&dev) ;

    net_cleanup() ;
    return 0 ;
}
