#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SOCK_PATH "mysocket"
int main(int argc, char const *argv[])
{
	int sfd,cfd;

	struct sockaddr_un my_addr, peer_addr;
	socklen_t sock_size;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0 );
	if(sfd==-1)
		perror("socket error");

	memset(&my_addr,0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path,SOCK_PATH, sizeof(my_addr.sun_path)-1);
	unlink(my_addr.sun_path);
	if (bind(sfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr_un))==-1)
	{
		perror("Bind error");
	}
	if(listen(sfd,50)==-1)
		perror("Listen error");
	
	sock_size = sizeof(struct sockaddr_un);

	cfd = accept(sfd, (struct sockaddr *)&peer_addr,&sock_size);
	
	if(cfd==-1)
		perror("Accept error");

	char buffer[256];
	
	read(cfd,buffer,255);
	buffer[0]='k';
	write(cfd,buffer,strlen(buffer));

	return 0;
}
