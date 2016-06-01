#include <stdio.h>
#include <pthread.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int fd);
int construct_requesthdrs(rio_t *rp, char *buf, char *filename);
void parse_uri(char *uri, char *host, char *port, char *filename);

void *doit_thread(void *vargp);

int main(int argc, char *argv[])
{
    int listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
	int *connfdp = Malloc(sizeof(int));
	clientlen = sizeof(clientaddr);
	*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	Pthread_create(&tid, NULL, doit_thread, connfdp);
    }
    return 0;
}

void *doit_thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int fd)
{
    int clientfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE];
    char filename[MAXLINE];
    size_t buflen;
    rio_t c_rio, s_rio;


    /* Read request line and headers */
    Rio_readinitb(&c_rio, fd);
    if (!Rio_readlineb(&c_rio, buf, MAXLINE))
	return;
    printf("%s", buf);

    sscanf(buf, "%s %s", method, uri);
    sprintf(buf, "%s %s %s", method, uri, "HTTP/1.0\r\n");

    if (strcasecmp(method, "GET")) {
	return;
    }

    clientfd = construct_requesthdrs(&c_rio, buf, filename);

    Rio_readinitb(&s_rio, clientfd);
    Rio_writen(clientfd, buf, strlen(buf));

    while ((buflen = Rio_readlineb(&s_rio, buf, MAXLINE)) != 0) {
	Rio_writen(fd, buf, buflen);
    }

    Close(clientfd);
    exit(0);
}


/* 단순히 header를 읽고 적는 함수 */
int construct_requesthdrs(rio_t *rp, char *buf, char *filename)
{
    char temp[MAXLINE], temp2[MAXLINE];
    char uri[MAXLINE];
    char host[MAXLINE], port[MAXLINE];
    int clientfd;
    int flag = 0;

    sscanf(buf, "%s %s", temp, uri);
    /* parsing the usi */
    parse_uri(uri, host, port, filename);

    /* read header */
    Rio_readlineb(rp, temp, MAXLINE);
    sscanf(temp, "%s", temp2);

    if (strcmp(temp2, "Host:"))
	sprintf(buf, "%sHost: %s\r\n", buf, host);
    else
	strcat(buf, temp);

    strcat(buf, user_agent_hdr);

    if (strcmp(temp2, "User-Agent:") && strcmp(temp2, "Connection:"))
	strcat(buf, temp);

    while (strcmp(temp, "\r\n")) {
	Rio_readlineb(rp, temp, MAXLINE);
	printf("%s", temp);
	sscanf(temp, "%s", temp2);
	
	if (!strcmp(temp2, "User-Agent:"))
	    continue;

	if (strcmp(temp2, "Connection:") && strcmp(temp2, "Proxy-Connection:")) {
	    strcat(buf, temp);
	} else if (flag == 0) {
	    sprintf(buf, "%sConnection: close\r\n", buf);
	    sprintf(buf, "%sProxy-Connection: close\r\n", buf);
	    flag = 1;
	}
	
	if (!strcmp(temp, "\r\n") && flag == 0) {
	    sprintf(buf, "%sConnection: close\r\n", buf);
	    sprintf(buf, "%sProxy-Connection: close\r\n", buf);
	    strcat(buf, temp);
	}
    }

    printf("%s", buf);

    clientfd = Open_clientfd(host, port);
    return clientfd;
}

void parse_uri(char *uri, char *host, char *port, char *filename)
{
    char *ptr;
    char temp_uri[MAXLINE];

    strcpy(temp_uri, uri);

    ptr = temp_uri + 7;
    while (*ptr != '/' && *ptr != ':') {
	*host = *ptr;
	ptr++;
	host++;
    }
    *host = '\0';

    /* port번호 존재 한다면 가져옴 */
    if (*ptr == ':') {
	ptr++;
	while (*ptr != '/') {
	    *port = *ptr;
	    ptr++;
	    port++;
	}
	*port = '\0';
    } else {
	port = "80";
    }
    
    /* filename 가져옴 */
    sscanf(ptr + 1, "%s", filename);
}
