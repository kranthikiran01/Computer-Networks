#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define derror(msg) {printf("%s\n", msg); exit(0);}

struct message
{
	char buf[100];
	int cno;
	int length;
};


int sfd,cno;
struct sockaddr_in ser_addr,cli_addr;
int portno,ser_len;


int main(int argc,char* args[])
{
	pthread_t sender,receiver;
	char buf[256];
	int ch,portno;
	int cli_len;
	cli_len = sizeof(struct sockaddr_in);
	
	
	if(sfd < 0)
		derror("Socket create");

	memset(&ser_addr,0,sizeof(struct sockaddr_in)); // Initialize to 0

	ser_addr.sin_family = AF_INET;
	// ser_addr.sin_port = htons(portno); // converts int to 16 bit integer in network byte order
	ser_addr.sin_addr.s_addr = INADDR_ANY; // to get IP address of machine
	ser_len = sizeof(struct sockaddr_in);

	//read(0,ch,2); // ch[0] will hold '1' if client wants connection oriented server '2' if connection less
	memset(buf,0,256);
	
	ch = atoi(args[1]); // get choice from command line argument
	if(ch==1) // Connection oriented service
	{
		sfd = socket(AF_INET,SOCK_STREAM,0); // socket create... AF_INET for IPv4 domain
		// and SOCK_STREAM for connection oriented systems
		portno = atoi(args[2]); // get port no
		ser_addr.sin_port = htons(portno);
		if(connect(sfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in)) < 0)
			derror("Connect error"); // connect to server
		
		read(0,buf,256); // read input from keyboard
		send(sfd,buf,256,0);
		memset(buf,0,256);
		
		while(recv(sfd,buf,256,0)<=0); // receive capitalised version
		write(1,buf,strlen(buf));
	}
	else
	{
		portno = atoi(args[2]);
		sfd = socket(AF_INET,SOCK_DGRAM,0); // socket create... AF_INET for IPv4 domain
		ser_addr.sin_port = htons(portno);
		read(0,buf,256); // read input from keyboard
		sendto(sfd,buf,strlen(buf),0,(struct sockaddr*)&ser_addr,cli_len);
		memset(buf,0,256);
		while(recvfrom(sfd,buf,256,0,(struct sockaddr*)&cli_addr,&cli_len)<=0); // receive  version
		write(1,buf,strlen(buf));
		sleep(20);	
	}
	return 0;
}