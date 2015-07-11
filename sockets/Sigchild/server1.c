#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#define derror(msg) {printf("%s\n", msg); exit(0);}

int sfd,cfd;

int main()
{
	int i;
	char buf[256];
	dup2(0,cfd);
	while(recv(cfd,buf,256,0)<0);
	for(i=0;i<strlen(buf);i++)
		buf[i]=toupper(buf[i]);
	send(cfd,buf,256,0);
	return 0;
}