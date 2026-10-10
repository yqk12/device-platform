#include<stdio.h>
#include<string.h>
#include<winsock2.h>
#include<windows.h>
#include "net.h"

/*
初始化
*/
int net_init(void)
{
    WSADATA wsa ;
    if(WSAStartup(MAKEWORD(2,2) , &wsa) != 0)
    {
        printf("[net] WSAStartup 失败\n") ;
        return -1 ;
    }
    return 0 ;
}

/*
清理
*/
void net_cleanup(void)
{
    WSACleanup() ;
}



int http_post(const char *host , int port , const char *path)
{
    //create socket to use
    SOCKET sock = socket(AF_INET , SOCK_STREAM , 0) ;
    // jugment error or ringht 
    if(sock == INVALID_SOCKET)
    {
        printf("[net] 建 socket 失败（错误码 %d)\n" , WSAGetLastError()) ;
        return 0 ;
    }

    struct sockaddr_in addr ;

    //【memset 清零】局部变量的内容是栈上的垃圾值，必须清零再填字段。
    memset(&addr , 0 , sizeof(addr)) ;
    addr.sin_family = AF_INET ;
    //网络只识别32位不转不会识别
    addr.sin_port = htons((unsigned short)port) ;

    addr.sin_addr.s_addr = inet_addr(host) ;

    //连接服务器
    if(connect(sock , (struct sockaddr*)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
        printf("[net] 连接失败（错误码 %d" , WSAGetLastError()) ;
        closesocket(sock) ;
        return 0 ;
    }

    //http请求
    char req[1024] ;
    int n = snprintf(req , sizeof(req) ,
        "POST %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n" ,
        path , host , port
    ) ;

    if(n < 0 || n>= (int)sizeof(req))
    {
        printf("[net] 报文太长被拦截 \n") ;
        closesocket(sock) ;
        return 0 ;
    }

    send(sock , req , n , 0) ;

    /*response*/
    char buf[2048] ;
    int r , ok = 0 , total = 0 ;

    while((r = recv(sock , buf , sizeof(buf) - 1 , 0)) > 0)
    {
        buf[r] = 0 ;
        if(total == 0) printf("[net] 回应: %.16s...\n", buf);
        if(strstr(buf ,"200 ok")!= NULL)  ok = 1 ;
        total += r ;
        closesocket(sock);
        return ok ;
    }
    return ok ;
}
