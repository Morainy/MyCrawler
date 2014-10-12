/*************************************************************************
    > File Name: RIO.h
    > Author: Morain
    > Mail: morainchen135@gmail.com
    > Created Time: 2014年10月08日 星期三 21时59分01秒
 ************************************************************************/



#ifndef RIO_H
#define RIO_H

#include <errno.h>
#include <string.h>
#include <unistd.h>
#define RIO_BUFSIZE 8192

typedef int ssize_t;
typedef unsigned size_t;
typedef struct 
{
	int rio_fd;                    	//Descriptor for this internal buf
	int rio_cnt;					//Unread bytes in internal buf
	char *rio_bufptr;				//Next unread byte in internal buf
	char rio_buf[RIO_BUFSIZE];		//Internal buffer
} rio_t;
ssize_t rio_readn(int fd , void *usrbuf , size_t n);
ssize_t rio_writen(int fd , void * usrbuf , size_t n);


void rio_readinitb(rio_t *rp , int fd);

ssize_t rio_readlineb(rio_t *rp , void *usrbuf , size_t maxlen);
ssize_t rio_readnb(rio_t * rp , void *usrbuf , size_t n);




#endif
