/*
网络层
负责发送请求如post,但是不管是mqtt还是http
*/
#ifndef NET_H
#define NET_H

int net_init(void) ;
void net_cleanup(void) ;
int http_post(const char *host , int port , const char *path) ;

#endif

