#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
int main(int argc, char const *argv[])
{
	struct hostent *server;
	struct sockaddr_in serv_addr;
	int portno=atoi(argv[2]);
	int nsfd;
	char buffer[256];
	server=gethostbyname(argv[1]);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(atoi(argv[2]));
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	int sfd=socket(AF_INET,SOCK_DGRAM,0);
	while(1){
	n=read(0,buffer,100);
	if(n<=0)return;
	sendto(sfd,buffer,strlen(buffer),0,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	bzero(buffer,strlen(buffer));
	recvfrom(sfd,buffer,255,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	write(1,buffer,strlen(buffer));
	}
	return 0;
}
