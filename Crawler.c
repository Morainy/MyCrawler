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
#include <zlib.h>

//#undef _GNU_SOURCE


#define MAXLINE 8192
#define BUFFERSIZE 1024

char * respHeader;   	//save response header
unsigned int respHeaderLen;
char * respMessage;		//save response message
unsigned int respMessageLen;
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
void sendRequestHeader(int fd , const char * host , const char * URI)
{
	char header[MAXLINE];
	sprintf(header , "GET %s HTTP/1.1\r\n" , URI);
	sprintf(header , "%sHost: %s\r\n",header , host);
	//sprintf(header , "%sAccept-Langusge=""zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3""" , header);
	//sprintf(header , "%sAccept-Encoding=""gzip""",header);
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
	respHeaderLen = strlen(respHeader);
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

	return;
}


void unzip()
{
	if(!strcasestr(respHeader , "Content-Encoding: gzip"))
		return ;
	printf("respMessage = \n");
	
	rio_writen(STDOUT_FILENO , respMessage , respMessageLen);
	char * buf = (char *)malloc( 2 * respMessageLen * sizeof(char));
	bzero(buf , 2 * respMessageLen);
	unsigned long bufLen = 2 * respMessageLen;
	int rtn = uncompress(buf , &bufLen , respMessage , respMessageLen);
	if(rtn == Z_OK)
	{
		free(respMessage);
		respMessageLen = bufLen;
		respMessage = buf;
		printf("buf = \n");
		rio_writen(STDOUT_FILENO , buf , bufLen);
		printf("update respMessage\n");
		rio_writen(STDOUT_FILENO , respMessage , respMessageLen);
	}
	else if(rtn == Z_MEM_ERROR)
	{
		printf("Memory is not enough\n");
		return;
	}
	else if(rtn == Z_BUF_ERROR)
	{
		printf("The buffer is too small\n");
	}
	else if(rtn == Z_DATA_ERROR)
	{
		printf("Source data has been damaged\n");
	}
	return;
}


int crawlePage(const char * host , const char * URI)
{
	int sockfd ;
	if( (sockfd = connectServer(host , 80) ) < 0)
	{
		printf("connectServer error\n");
		return -1;
	}
	sendRequestHeader(sockfd , host , URI);
	readRequestResponse(sockfd);

	readPage(sockfd , respHeader , &respMessageLen);
	unzip();
	rio_writen(STDOUT_FILENO , respMessage , strlen(respMessage));
	printf("\n");
	close(sockfd);
	return 0;
}

list <string> URLlist;
void parsePage(const char * page) 
{
	char * ptr = page;
	char * start , * end;
	char chTemp[256];
	while(ptr = strstr(ptr + 1, "href"))
	{
		start = index(ptr , '"') + 1;
		end = index(start , '"');
		//cout<<"*end = "<<*end<<endl;
		memcpy(chTemp , start , end - start);
		chTemp[end-start] = 0;
		//cout<<"chTemp = "<<chTemp<<endl;
		string URLTemp(chTemp);
		URLlist.push_back(URLTemp);
	}
	/*list <string>::iterator it;
	for(it = URLlist.begin() ; it != URLlist.end() ; it++)
	{
		cout<<*it<<endl;
	}*/
}


char host[BUFFERSIZE] , URI[BUFFERSIZE];
void parseURL(char * URL )
{
	char * ptr;
	char * start;
	if(strstr(URL , "http://"))
		start = URL + strlen("http://");
	else
		start = URL;


	if( !(ptr = index(start , '/') ) )
	{
		strcpy(host , start);
		return;
	}

	strncpy(host , start , ptr - start);
	strcpy(URI , ptr);
	return;
}




void Crawler(const char * host)  //Crawl a website
{
	crawlePage(host , "/");
	parsePage(respMessage);
	list<string>::iterator it;
	for(it = URLlist.begin() ; it != URLlist.end() ; it++)
	{
		parseURL(*it);
		crawlePage(host , URI);
	}
}

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		printf("%s <host> \n",argv[0]);
		return -1;
	}
	Crawler(argv[1]);

	free(respHeader);
	free(respMessage);
	return 0;
}
