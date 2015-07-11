#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#define derror(msg) {printf("%s\n", msg); exit(0);}
#define SOCK_PATH "./temp_socket"

int nos,usfd,sfd;
struct sockaddr_in ser_addr,cli_addr;
struct sockaddr_un ser_uaddr,cli_uaddr;

int portno,cli_len,cli_ulen,i,j,count; // max holds max fd value + 1

static int send_file_descriptor(
 	int socket, /* Socket through which the file descriptor is passed */
 	int fd_to_send) /* File descriptor to be passed, could be another socket */
{
 	struct msghdr message;
 	struct iovec iov[1];
 	struct cmsghdr *control_message = NULL;
 	char ctrl_buf[CMSG_SPACE(sizeof(int))];
 	char data[2];

 	memset(&message, 0, sizeof(struct msghdr));
 	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

 	/* We are passing at least one byte of data so that recvmsg() will not return 0 */
 	data[0] = ' ';
 	data[1] = '\0';
 	iov[0].iov_base = data;
 	iov[0].iov_len = sizeof(data);

 	message.msg_name = NULL;
 	message.msg_namelen = 0;
 	message.msg_iov = iov;
 	message.msg_iovlen = 1;
 	message.msg_controllen =  CMSG_SPACE(sizeof(int));
 	message.msg_control = ctrl_buf;

 	control_message = CMSG_FIRSTHDR(&message);
 	control_message->cmsg_level = SOL_SOCKET;
 	control_message->cmsg_type = SCM_RIGHTS;
 	control_message->cmsg_len = CMSG_LEN(sizeof(int));

 	*((int *) CMSG_DATA(control_message)) = fd_to_send;

 	return sendmsg(socket, &message, 0);
}


int recv_file_descriptor(void* param) /* Socket from which the file descriptor is read */
{

	int socket = *((int*)param);

 	int sent_fd;
 	struct msghdr message;
 	struct iovec iov[1];
 	struct cmsghdr *control_message = NULL;
 	char ctrl_buf[CMSG_SPACE(sizeof(int))];
 	char data[2];
 	int res;

 	printf("Waiting to receive fd %d\n",socket);
    
    memset(&message, 0, sizeof(struct msghdr));
    memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

   	/* For the dummy data */
    iov[0].iov_base = data;
    iov[0].iov_len = sizeof(data);

    message.msg_name = NULL;
    message.msg_namelen = 0;
    message.msg_control = ctrl_buf;
    message.msg_controllen = CMSG_SPACE(sizeof(int));
    message.msg_iov = iov;
    message.msg_iovlen = 1;

    if((res = recvmsg(socket, &message, 0)) < 0)
    printf("Does not receive\n");

     /* Iterate through header to find if there is a file descriptor */
    for(control_message = CMSG_FIRSTHDR(&message);
       	control_message != NULL;
       	control_message = CMSG_NXTHDR(&message,
                                      control_message))
    {
     	if( (control_message->cmsg_level == SOL_SOCKET) &&
       	  (control_message->cmsg_type == SCM_RIGHTS) )
     	{
       		int sd  =  *((int *) CMSG_DATA(control_message));
       		sfd = sd;
       		printf("Recieved\n");
       		return 1;
      	}
    }
    return 0;
}


void* service_thread(void *arg)
{
	int no = *((int *)arg);
	while((count)%(nos+1)!=no);
	int nsfd = accept(sfd, (struct sockaddr *)&cli_addr, &cli_len);
	count++;
	while(1)
	{
		char buf[256];
		memset(buf,0,256);
		recv(nsfd,buf,256,0);
		printf("%s",buf);
		send(nsfd,buf,256,0);
	}
}

void* main_thread(void *arg)
{
	int no = *((int *)arg);
	while(1)
	{
		while((count)%(nos+1)!=no);
		count++;
		int nsfd = accept(usfd, (struct sockaddr *)&cli_uaddr, &cli_ulen);
		send_file_descriptor(nsfd,sfd);
	}
}


int main(int argc,char* args[])
{
	
	fd_set readfds;

	nos = 2;


	cli_len = sizeof(struct sockaddr_in);

	usfd = socket(AF_UNIX,SOCK_STREAM,0); // socket create... for unix sockets
	
	if(usfd < 0)
		derror("Socket create");


	memset(&ser_uaddr,0,sizeof(struct sockaddr_un)); // Initialize to 0
	memset(&cli_uaddr,0,sizeof(struct sockaddr_un));

	ser_uaddr.sun_family = AF_UNIX;
    strcpy(ser_uaddr.sun_path, SOCK_PATH);
    /*unlink(ser_uaddr.sun_path);

	if(bind(usfd,(struct sockaddr*)&ser_uaddr,sizeof(ser_uaddr))<0)
		derror("Bind error");
	
	if(listen(usfd,10)<0) // No. of clients that server can service
		derror("Listen error");*/

	cli_ulen = sizeof(struct sockaddr_un);

	if(connect(usfd, (struct sockaddr *)&ser_uaddr, cli_ulen) == -1)
		derror("connect erroe\n");

	count=-1;

	pthread_t st[2],mt;
	int temp[2];

	for(i=0;i<nos;i++)
	{
		temp[i]=i;
		pthread_create(&st[i],NULL,service_thread,(void*)&temp[i]);
	}


	int lol = nos;
	pthread_create(&mt,NULL,main_thread,(void*)&lol);
	recv_file_descriptor(&usfd);
	count++;
	pthread_join(mt,NULL);

	return 0;
}