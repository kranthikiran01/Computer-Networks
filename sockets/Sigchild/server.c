#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define derror(buf) {printf("%s\n", buf); exit(0);}

//int csfd,lsfd,ncsfd; // csfd holds fd of each client and noc hold no of clients 

int s2_count;
int s5_flag,s3_pid; // to store pid of s1 children

struct message // structure of message to be sent
{
	char buf[100];
	int cno;
	int length;
};


static void child_detect(int x)
{
	if(x==SIGCHLD)
	{
		int cpid,i,status;
		int s1_shm,s1_chi;
		int *s1_count,*s1_child;
		s1_shm = shmget(ftok(".",2), sizeof(int), IPC_CREAT | 0666);
		s1_count = shmat(s1_shm,NULL,0);
		s1_chi = shmget(ftok(".",4), 25*sizeof(int), IPC_CREAT | 0666);
		s1_child = shmat(s1_chi,NULL,0);
		cpid = waitpid(-1,&status,WNOHANG);
		printf("%d before signal\n",*s1_count);
		for(i=0;i<25;i++)
		{
			if(cpid==s1_child[i])
			{
				s1_child[i]=-1;
				*s1_count = *s1_count-1;
				break;
			}
		}
		printf("%d signal\n",*s1_count);
	}
	signal(SIGCHLD,child_detect);
}

void* s2_service(void *arg)
{
	int sfd,len;
	struct sockaddr_in cli_addr;
	sfd = *((int*) (arg));
	char buf[256];
	memset(buf,0,100);
	recv(sfd,&buf,256,0);
	len=strlen(buf);
	memset(buf,0,100);
	snprintf(buf,100,"%d",len);
	send(sfd,&buf,256,0);
	close(sfd);
	s2_count--;
}

void s3_function()
{
	return;
}


int main(int argc,char* args[])
{
	struct sockaddr_in ser_addr[6],cli_addr;
	int portno,cli_len,i,j,max,nsfd;  // max holds max fd value + 1
	i = getppid();
	printf("%d %d\n", i,getpid());
	int S[6];
	char buf[256];
	int s1_shm,s1_chi;
	int *s1_count,*s1_child;
	s1_shm = shmget(ftok(".",2), sizeof(int), IPC_CREAT | 0666);
	s1_count = shmat(s1_shm,NULL,0);
	s1_chi = shmget(ftok(".",4), 25*sizeof(int), IPC_CREAT | 0666);
	s1_child = shmat(s1_chi,NULL,0);
	for(i=0;i<25;i++)
		s1_child[i]=-1;
	signal(SIGCHLD,child_detect);
	*s1_count=s2_count=0;
	fd_set readfds;

	cli_len = sizeof(struct sockaddr_in);
	

	for(i=1;i<=5;i++)
	memset(&ser_addr[i],0,sizeof(struct sockaddr_in)); // Initialize to 0

	// address of connection oriented socket
	for(i=1;i<=5;i++)
	{
		ser_addr[i].sin_family = AF_INET;
		portno = atoi(args[i]);
		ser_addr[i].sin_port = htons(portno); // converts int to 16 bit integer in network byte order
		ser_addr[i].sin_addr.s_addr = INADDR_ANY; // to get IP address of machine on which server is running
	}

	S[1] = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0); // socket create... AF_INET for IPv4 domain and SOCK_STREAM for connection oriented systems
	S[2] = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
	S[3] = socket(AF_INET,SOCK_DGRAM,0);
	S[4] = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
	S[5] = socket(AF_INET,SOCK_DGRAM,0);

	for(i=1;i<=5;i++)
	if(bind(S[i],(struct sockaddr*)&ser_addr[i],sizeof(struct sockaddr_in))<0)
		derror("Bind error");
	
	if(listen(S[1],5)<0) // No. of clients that server can service
		derror("Listen error");

	if(listen(S[2],5)<0) // No. of clients that server can service
		derror("Listen error");

	if(listen(S[4],5)<0) // No. of clients that server can service
		derror("Listen error");

	cli_len = sizeof(struct sockaddr_in);

	struct timespec tim_dur;
	tim_dur.tv_sec = 2;   // waiting time of 1 sec for verifying set bits of FDSET
	tim_dur.tv_nsec = 0;

	max=S[1];
	for(i=2;i<=5;i++)
	{
		if(max<S[i])
			max=S[i];
	}

	while(1)
	{
		FD_ZERO(&readfds);
		// printf("%d %d\n",*s1_count,s2_count);

		
		if(*s1_count<25)
		FD_SET(S[1],&readfds);
		if(s2_count<15)
		FD_SET(S[2],&readfds);
		for(i=3;i<=5;i++)
		{
			//if(i!=1&&i!=2)
			FD_SET(S[i],&readfds); // Set the connection oriented sfd
			// if(i==1&&s1_count<2)
			// FD_SET(S[i],&readfds);
			// if(i==2&&s2_count<2)
			// FD_SET(S[i],&readfds);
		}

		// pselect for finding if data is available
		//printf("HEllo\n");
		if(pselect(max+1,&readfds,NULL,NULL,&tim_dur,NULL)>0); // pselect is used as timeleft is not updated after each select call
		{
			//printf("Select truee\n");
			if(FD_ISSET(S[1],&readfds))
			{
				printf("S1\n");
				// if(s1_count<3)
				// {
					nsfd = accept(S[1],(struct sockaddr*)&cli_addr,&cli_len);
					if(nsfd>=0)
					{
						*s1_count=*s1_count+1;
						int c = fork();
						if(c>0)
						{
							for(i=0;i<25;i++)
							{
								if(s1_child[i]==-1)
								{
									s1_child[i]=c;
									break;
								}
							}
							close(nsfd);
							printf("%d %d\n",*s1_count,s2_count);
						}
						else
						{
							close(S[1]);
							dup2(nsfd,0);
							execlp("./s1","s1",(char*) 0); // capitalise the buffer
							exit(0);
						}
					}
					else
					{;
					//	printf("Something fishy in s1!\n");
					}
				// }
				// else
				// {
				// 	printf("queue for s1\n");
				// }
			}

			if(FD_ISSET(S[2],&readfds))
			{
				printf("S2\n");
				// if(s2_count<2)
				// {
					nsfd = accept(S[2],(struct sockaddr*)&cli_addr,&cli_len);
				if(nsfd>=0)
				{
					pthread_t s2_thread;
					s2_count++;
					pthread_create(&s2_thread,NULL,s2_service,(void*)&nsfd);
					printf("%d %d\n",*s1_count,s2_count);
				}
				else
				{;
				//	printf("Something fishy on s2\n");
				}
				// }
				// else
				// {
				// 	printf("queue for s2\n");
				// }
			}

			if(FD_ISSET(S[4],&readfds))
			{
				printf("S4\n");
				nsfd = accept(S[4],(struct sockaddr*)&cli_addr,&cli_len);
				if(nsfd>=0)
				{
					int c = fork();
					if(c>0)
					{
						close(nsfd);
					}
					else
					{
						close(S[4]);
						dup2(nsfd,0);
						execlp("./s5","s5",(char*) 0); // capitalise the buffer
						exit(0);
					}
				}
				else
				{
					printf("Something fishy on s4\n");
				}
			}

			if(FD_ISSET(S[3],&readfds))
			{
				printf("S3\n");
				s3_function();
			}

			if(FD_ISSET(S[5],&readfds))
			{
				printf("S5\n");
				if(s5_flag)
				{
					int c = fork();
					if(c>0)
					{
						;
					}
					else
					{
						dup2(S[5],0);
						execlp("./s5","s5",(char*) 0); // capitalise the buffer
						exit(0);
					}
				}
				else
				{;
					//printf("s5 full so waiting\n");
				}
			}
		}
		//printf("Some testing\n");
	}

	return 0;
}