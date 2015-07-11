#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define SOCK_PATH "mysocket"

int main(int argc, char const *argv[])
{
	int sfd,len,n;
	struct sockaddr_un remote;
	char buffer[256];
	if((sfd=socket(AF_UNIX,SOCK_STREAM,0))==-1){
		perror("Socket error");
	}

	printf("Trying to connect ....\n");

	remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if(connect(sfd,(struct sockaddr *)&remote,len)==-1)
	{
		perror("Connect error");
		exit(1);
	}

	printf("Connection established. OK!\n");

	while(printf(">"), fgets(buffer,256,stdin),!feof(stdin)){
		if(send(sfd,buffer,strlen(buffer),0)==-1)
		{	
			perror("Send error");
			exit(1);
		}
		if((n=recv(sfd,buffer,256,0))>0){
			buffer[n]='\0';
			printf("echo> %s\n",buffer );
		}
		else{
			if(n<0)perror("Reading error");
			else{
				printf("Server closed connection!\n");
				exit(1);
			}
		}
	}

	close(sfd);

	return 0;
}