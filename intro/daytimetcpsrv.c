/************************************************************************
 * @file: Do not edit
 * @author: xxx
 * @brief: xxx
 * @version: 1.0.0
 * @LastEditTime: 2021-06-14 17:28:49
 * @attention: Do not edit
 *************************************************************************/
#include	"unp.h"
#include	<time.h>

#define	USER_PORT	7777

typedef enum {
	TCP_WAIT_CONNECTING = 0,
	TCP_CONNECT_SUCCESS = 1,
	TCP_DISCONNECT_SUCCESS = 2,
}TCP_STATUS_E;

static TCP_STATUS_E my_tcp_status;

int main(int argc, char **argv)
{
	int					listenfd, connfd;
	int					n;
	struct sockaddr_in	servaddr;
	struct sockaddr_in	clientaddr;
	char				recv_buf[MAXLINE + 1];
	char				send_buf[MAXLINE + 1];
	char				str_ipaddr[15 + 1];
	time_t				ticks;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&clientaddr, sizeof(clientaddr));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(USER_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	my_tcp_status = TCP_WAIT_CONNECTING;

	socklen_t len = sizeof(clientaddr);

	for ( ; ; ) {
		
		switch (my_tcp_status){
			case TCP_WAIT_CONNECTING:{
				connfd = Accept(listenfd, (SA *) &clientaddr, &len);
				if (connfd > 0){
					my_tcp_status = TCP_CONNECT_SUCCESS;
					printf("connection from %s, port %d\n",
					Inet_ntop(AF_INET, &clientaddr.sin_addr, str_ipaddr, sizeof(str_ipaddr)),
					ntohs(clientaddr.sin_port));
				}else{
					printf("Accept return :%d\n",connfd);
				}
			}break;
			case TCP_CONNECT_SUCCESS:{
				n = read(connfd, recv_buf, MAXLINE);	// 阻塞，等待数据到达
				if (n > 0){
					recv_buf[n] = '\0';	/* null terminate */
					fputs(recv_buf, stdout);
				}else if(n == 0){
					my_tcp_status = TCP_DISCONNECT_SUCCESS;
					continue;
				}else{
					printf("read error: %d\n",n);
					exit(0);
				}
				// 发送数据
				ticks = time(NULL);
				snprintf(send_buf, sizeof(send_buf), "%.24s\r\n", ctime(&ticks));
				Write(connfd, send_buf, strlen(send_buf));
			}break;
			case TCP_DISCONNECT_SUCCESS:{
				printf("client disconnect...\n");
				Close(connfd);
				my_tcp_status = TCP_WAIT_CONNECTING;
			}break;
		}
	}
	Close(connfd);
}
