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

typedef struct
{
    uint32_t head; // 0x55aa
    // uint32_t fr_num;
    // uint32_t fr_type;
    uint32_t length;
    uint8_t data[0];
}tcp_frame_head_t;

uint8_t *copy_file_data_to_ram(char *file_name, uint32_t size);


int main(int argc, char **argv)
{
	int					sockfd, n, ret;
	socklen_t			len;
	uint8_t				recv_buf[MAXLINE + 1];
	struct sockaddr_in	servaddr, cliaddr;
	char				file_name[128 + 1]; 
	char				ipaddr_name[15 + 1]; 
    uint32_t            send_tcp_size;

	if (argc != 4)
		err_quit("usage: a.out <IPaddress> <file> <size>");

	memset(ipaddr_name, 0, sizeof(ipaddr_name));
	strncpy(ipaddr_name,argv[1],sizeof(ipaddr_name));

	memset(file_name, 0, sizeof(file_name));
	strncpy(file_name,argv[2],sizeof(file_name));

    send_tcp_size = atoi(argv[3]);

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

    uint8_t *file_data = copy_file_data_to_ram(file_name,send_tcp_size);
    if (file_data == NULL){
        printf("Error: copy_file_data_to_ram\n");
        exit(EX_OSERR);
    }

    ret = write(sockfd,file_data,send_tcp_size);
    printf("write ret:%d\n",ret);
    free(file_data);

	// 读取服务器返回的数据，并打印出来
	while ( (n = read(sockfd, recv_buf, MAXLINE)) > 0) {
        printf("read: %s\n", recv_buf);
	}
	if (n < 0)
		err_sys("read error");

	exit(0);
}

uint8_t *copy_file_data_to_ram(char *file_name, uint32_t size)
{
    // 从文件读取数据进行发送
	FILE *fp = fopen(file_name, "rt");
	if (fp == NULL){
		printf("fopen null \n");
		return NULL;
	}

    struct stat statbuff;
	if(stat(file_name, &statbuff) < 0) {
        printf("File Not Exist:%s\n", file_name);
        fclose(fp);
		return NULL;
    }
    printf("file size: %ld Bytes\n", statbuff.st_size);
	uint32_t read_size = 0;
    if (size > statbuff.st_size){
        read_size = statbuff.st_size;
    }else{
        read_size = size;
    }

	uint8_t *file_buf = malloc(read_size);
    if (file_buf == NULL){
        fclose(fp);
        return NULL;
    }
	uint32_t actual_read = fread(file_buf, 1, read_size, fp);
	if(actual_read != read_size){
		printf("fread %s error,actual_read:%d,file_size:%d\n", file_name, actual_read, read_size);
	}
    fclose(fp);

    return file_buf;
}




