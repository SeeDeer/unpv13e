/************************************************************************
 * @file: Do not edit
 * @author: xxx
 * @brief: 即时TCP聊天工具 的服务端实现
 * @version: 1.0.0
 * @LastEditTime: 2021-06-14 17:53:59
 * @attention: Do not edit
 *************************************************************************/

#include "unp.h"

#include <time.h>
#include <sysexits.h>

#define	IM_SRV_PORT	9991

typedef enum {
	TCP_WAIT_CONNECTING = 0,
	TCP_CONNECT_SUCCESS = 1,
	TCP_DISCONNECT_SUCCESS = 2,
}TCP_STATUS_E;

static TCP_STATUS_E my_tcp_status;

int main(int argc, char **argv)
{
	int listenFd = -1;
    int ret;
    char				recv_buf[MAXLINE + 1];
	char				send_buf[MAXLINE + 1];
    
    listenFd = socket(AF_INET, SOCK_STREAM,0);
    if (listenFd < 0){
        printf("listen socket error. %d\n",listenFd);
        exit(EX_OSERR);
    }
    // 给套接字绑定一个服务器端口
    struct sockaddr_in addr;
    memset(&addr, 0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(IM_SRV_PORT);
    ret = bind(listenFd,(struct sockaddr *)&addr,sizeof(addr));
    if (ret < 0){
        printf("socket bind error. %d\n",ret);
        exit(EX_OSERR);
    }

    char	*ptr = NULL;
    int backlog;
    if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);
    else
        backlog = LISTENQ;
    printf("listen of backlog=%d\n", backlog);
    ret = listen(listenFd,backlog);
    if (ret < 0){
        printf("listen error. %d\n",listenFd);
        exit(EX_OSERR);
    }

    my_tcp_status = TCP_WAIT_CONNECTING;
	struct sockaddr_in	clientaddr;
    memset(&clientaddr, 0, sizeof(clientaddr));
	socklen_t len = sizeof(clientaddr);
    int clientFd = -1;
	char str_ipaddr[15 + 1];

    while(1)
    {
        switch (my_tcp_status){
			case TCP_WAIT_CONNECTING:{
				clientFd = accept(listenFd, (SA *) &clientaddr, &len);
				if (clientFd > 0){
                    my_tcp_status = TCP_CONNECT_SUCCESS;
                    printf("connection from %s, port %d\n",
                    Inet_ntop(AF_INET, &clientaddr.sin_addr, str_ipaddr, sizeof(str_ipaddr)),
                    ntohs(clientaddr.sin_port));
                    // pid_t pid = fork();
                    // if (pid == 0){ // in clild process
                    //     close(listenFd);
                    // }else{
                        
                    // }
				}else{
					printf("Accept return :%d\n",clientFd);
				}
			}break;
			case TCP_CONNECT_SUCCESS:{
				ret = read(clientFd, recv_buf, MAXLINE);	// 阻塞，等待数据到达
				if (ret > 0){
                    memset(send_buf,0,sizeof(send_buf));
                    memcpy(send_buf, recv_buf,ret);
					// fputs(recv_buf, stdout);
				}else if(ret == 0){
					my_tcp_status = TCP_DISCONNECT_SUCCESS;
					continue;
				}else{
					printf("read error: %d\n",ret);
					exit(0);
				}
				// 发送数据
				write(clientFd, send_buf, strlen(send_buf));
			}break;
			case TCP_DISCONNECT_SUCCESS:{
				printf("client disconnect...\n");
				close(clientFd);
				my_tcp_status = TCP_WAIT_CONNECTING;
			}break;
		}
    }
    
    exit(EX_OK);
}


