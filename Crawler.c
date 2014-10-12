/*************************************************************************
    > File Name: Crawler.c
    > Author: Morain
    > Mail: morainchen135@gmail.com
    > Created Time: 2014年10月12日 星期日 01时35分12秒
 ************************************************************************/


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "RIO.h"
#include <stdlib.h>

//#undef _GNU_SOURCE


#define MAXLINE 8192

char * respHeader;
char * respMessage;
typedef struct sockaddr SA;
int connectServer (const char * servAddr , int port)
{
	struct sockaddr_in server;
	int connfd ;
	char ip[32];

	struct hostent *hp;
	if( (hp = gethostbyname(servAddr) ) < 0 )
	{
		perror("gethostbyname error");
		return -1;
	}

	bzero(&server , sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	//server.sin_addr.s_addr = hp->h_addr_list[0];

	//printf(" first address: %s\n",inet_ntop(hp->h_addrtype, hp->h_addr, ip, sizeof(ip)));
	inet_ntop(hp->h_addrtype, hp->h_addr, ip, sizeof(ip));
	//printf("hp->h_addr_list[2] = %s\n", hp->h_addr_list[2]);

	/*if(!(inet_aton(ip , &server.sin_addr)) )
	{
		perror("inet_aton error");
		return -1;
	}*/

	if(inet_pton(AF_INET , ip , &server.sin_addr) <= 0)
	{
		perror("inet_pton error");
		return -1;
	}
	if ( (connfd = socket(AF_INET , SOCK_STREAM , 0) ) < 0)
	{
		perror("socket error");
		return -1;
	}
	
	if(connect(connfd , (SA *)&server , sizeof(server)) < 0)
	{
		perror("connect error");
		return -1;
	}

	return connfd;
}
void sendRequestHeader(int fd , const char * host)
{
	char header[MAXLINE];
	sprintf(header , "GET / HTTP/1.1\r\n");
	sprintf(header , "%sHost: %s\r\n",header , host);
	//sprintf(header , "%sAccept-Langusge=""zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3""" , header);
	//sprintf(header , "%sAccept-Encoding=""gzip, deflate""",header);
	sprintf(header , "%s\r\n" , header);
	rio_writen(fd , header , sizeof(header));
}
void readRequestResponse(int fd )
{
	respHeader = (char *)malloc(MAXLINE * sizeof(char));
	char buffer[MAXLINE];

	rio_t  rp;
	rio_readinitb(&rp , fd);
	rio_readlineb(&rp , buffer , MAXLINE);
	sprintf(respHeader , "");
	while(strcmp(buffer , "\r\n"))
	{
		sprintf(respHeader , "%s%s" , respHeader , buffer);
		rio_readlineb(&rp , buffer , MAXLINE);
	}
	//printf("strlen(respHeader) = %d\n",strlen(respHeader));
	//printf("resp = %s\n",resp);

	rio_writen(STDOUT_FILENO , respHeader , strlen(respHeader) );
	return;
}

void readPage(int fd , char *header , int * len)
{
	int length;
	char * lengthStr;
	char * ptr = strcasestr(header , "Content-Length:");
	lengthStr = ptr + sizeof("Content-Length:");
	sscanf(lengthStr , "%d" , &length);
	printf("length = %d \n",length);

	*len = length;
	respMessage = (char *)malloc(length * sizeof (char) + 1);

	rio_readn(fd , respMessage , length);

	rio_writen(STDOUT_FILENO , respMessage , length);
	return;
}

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		printf("%s <host> <port>\n",argv[0]);
		return -1;
	}
	int portNum = atoi(argv[2]);
	int fd;
	int pageLength;
	if( (fd = connectServer(argv[1] , portNum) ) < 0)
	{
		printf("connectServer error\n");
		return -1;
	}
	sendRequestHeader(fd , argv[1]);
	readRequestResponse(fd);

	readPage(fd , respHeader , &pageLength);

	printf("\n");
	free(respHeader);
	free(respMessage);
	close(fd);
	return 0;
}
