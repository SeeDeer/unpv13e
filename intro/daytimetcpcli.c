/*****************************************************************
 * @File: main.c
 * @Author: LinusZhao
 * @Date: 2019-10-22 23:52:31
 * @LastEditors: LinusZhao
 * @LastEditTime: 2019-10-24 08:08:00
 * @Description: 实现一个tcp客户端的功能，和服务端建立连接并通信
 *****************************************************************/
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

#include "unp.h"

#define SERV_USER_PORT 	7777

typedef struct
{
    uint32_t head; // 0x55aa
    // uint32_t fr_num;
    // uint32_t fr_type;
    uint32_t length;
    uint8_t data[0];
}tcp_frame_head_t;

int main(int argc, char **argv)
{
	int					sockfd, n, ret;
	socklen_t			len;
	uint8_t				recv_buf[MAXLINE + 1];
	struct sockaddr_in	servaddr, cliaddr;
	char				file_name[128 + 1]; 
	char				ipaddr_name[15 + 1]; 

	if (argc != 3)
		err_quit("usage: a.out <IPaddress>");

	memset(ipaddr_name, 0, sizeof(ipaddr_name));
	strncpy(ipaddr_name,argv[1],sizeof(ipaddr_name));

	memset(file_name, 0, sizeof(file_name));
	strncpy(file_name,argv[2],sizeof(file_name));

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(SERV_USER_PORT);
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

	// 从文件读取数据进行发送
	FILE *fp = fopen(file_name, "rt");
	if (fp == NULL){
		printf("fopen null \n");
		exit(1);
	}

    struct stat statbuff;
	if(stat(file_name, &statbuff) < 0) {
        printf("File Not Exist:%s\n", file_name);
        fclose(fp);
		exit(1);
    }
	uint32_t file_size = statbuff.st_size;

	uint8_t *file_buf = malloc(file_size);
	uint32_t actual_read = fread(file_buf, 1, file_size, fp);
	if(actual_read != file_size){
		printf("fread %s error,actual_read:%d,file_size:%d\n", file_name, actual_read, file_size);
	}
    fclose(fp);	

	#define SEND_FILE_PACKET_SIZE	(1024 * 100)
	uint32_t packet_num = actual_read / SEND_FILE_PACKET_SIZE;
	uint8_t *buf_ptr = file_buf;
	uint32_t data_length = SEND_FILE_PACKET_SIZE - sizeof(tcp_frame_head_t);
	for(uint32_t i = 0; i < packet_num; i++){
		buf_ptr[0] = 0x55;
		buf_ptr[1] = 0x56;
		buf_ptr[2] = 0x57;
		buf_ptr[3] = 0x58;
		// 长度
		buf_ptr[7] = data_length >> 24;
		buf_ptr[6] = (data_length >> 16) & 0xff;
		buf_ptr[5] = (data_length >> 8) & 0xff;;
		buf_ptr[4] = (data_length >> 0) & 0xff;;
		ret = write(sockfd,buf_ptr,SEND_FILE_PACKET_SIZE);
		if (ret != SEND_FILE_PACKET_SIZE){
			printf("write failed: %d \n",ret);
		}
		buf_ptr += ret;
		sleep(10);
	}
	free(file_buf);

	// 读取服务器返回的数据，并打印出来
	while ( (n = read(sockfd, recv_buf, MAXLINE)) > 0) {
		recv_buf[n] = 0;
		if (fputs(recv_buf, stdout) == EOF)
			err_sys("fputs error");
	}
	if (n < 0)
		err_sys("read error");

	exit(0);
}

