/************************************************************************
 * @file: Do not edit
 * @author: xxx
 * @brief: xxx
 * @version: 1.0.0
 * @LastEditTime: 2021-06-15 11:15:01
 * @attention: Do not edit
 *************************************************************************/

/************************************************************************
 * @file: Do not edit
 * @author: xxx
 * @brief: 实现一个tcp客户端的功能，和服务端建立连接并通信
 * @version: 1.0.0
 * @LastEditTime: 2021-06-15 08:22:50
 * @attention: Do not edit
 *************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sysexits.h>

#include "unp.h"

#define	IM_SRV_PORT	9991

int main(int argc, char **argv)
{
	int					sockfd, n, ret;
	socklen_t			len;
	char				buf[MAXLINE + 1];
	struct sockaddr_in	servaddr, cliaddr;
	char				file_name[128 + 1]; 
	char				ipaddr_name[15 + 1]; 
    // uint32_t            send_tcp_size;

	if (argc != 2)
		err_quit("usage: a.out <IPaddress>");

	memset(ipaddr_name, 0, sizeof(ipaddr_name));
	strncpy(ipaddr_name,argv[1],sizeof(ipaddr_name));

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(IM_SRV_PORT);
	if (inet_pton(AF_INET, ipaddr_name, &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", argv[1]);

	ret = connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	if(ret < 0){
		printf("connect fail.\n");
		close(sockfd);
		exit(1);
	}

	len = sizeof(cliaddr);
	Getsockname(sockfd, (SA *) &cliaddr, &len);
	printf("local addr: %s\n",Sock_ntop((SA *) &cliaddr, sizeof(cliaddr)));

    // 1.获取用户终端输入数据发送给服务端 send-> 
    // 2.读取服务端的数据回显到终端 recv-> 
    fd_set read_sets;
    int max_fd = -1;

    FD_ZERO(&read_sets);

    while(1) {
        FD_SET(fileno(stdin), &read_sets);
        FD_SET(sockfd, &read_sets);
        max_fd = max(fileno(stdin), sockfd) + 1;
        ret = select(max_fd, &read_sets,NULL,NULL,NULL);
        // printf("select ret:%d\n",ret);

         // 服务器发来数据了，可以读了
        if (FD_ISSET(sockfd, &read_sets)){
            n = read(sockfd, buf, MAXLINE);
            if (n > 0){
                buf[n] = '\0';
                printf("recv-> %s \n", buf);
            }
        }

        // 终端有数据了，可以读了
        if (FD_ISSET(fileno(stdin), &read_sets)) {
            if(fgets(buf,MAXLINE,stdin)== NULL){
                printf("fgets error\n");
                exit(1);
            }
            // printf("send-> %s \n", buf);
            ret = write(sockfd,buf,strlen(buf));
            // printf("write ret:%d\n",ret);
            // printf("recv-> %s \n", buf);
        }
    }
    

	exit(0);
}





