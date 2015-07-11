#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SRC "127.0.0.1"
#define DEST "127.0.0.1"

int sendrsfd,recvrsfd[3];

int man[3][10],rec[3][5][6];
int refv=-1;
void* verify(void *pa)
{
	int j,k,f=1;
	while(refv==-1);
	if(refv>-1)
	{
		for(j=0;j<5&&f==1;j++)
		{
			f=0;
			for(k=0;k<6;k++)
			{
				if(rec[refv][j][k]>0)
				{
					f=1;
					break;
				}
			}
		}
	}
	if(f==0)
		printf("%d won the game\n",refv);
	else
		printf("Fake announcement by %d REPLAY\n",refv);
	exit(0);
}
void* manager(void* pa)
{

	
	struct sockaddr_in daddr;
	char packet[50];
	char msg[10];
	struct iphdr *ip = (struct iphdr *)packet;  

	daddr.sin_family = AF_INET;
	daddr.sin_port = 0;
	inet_pton(AF_INET, DEST, (struct in_addr *)&daddr.sin_addr.s_addr);
	memset(daddr.sin_zero, 0, sizeof(daddr.sin_zero));
	memset(packet,0, sizeof(packet));
	
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->frag_off = 0;
	ip->ttl = 64;
	ip->protocol = 144;
	ip->check = 0;
	ip->saddr = daddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;

	while(1) 
	{ 
		memset(msg,0, sizeof(msg));
		scanf("%s",msg);
		strcpy(packet+sizeof(struct iphdr),msg);
		ip->tot_len = htons(sizeof(struct iphdr)+sizeof(msg));
		int y=0;
		int l=strlen(msg),i;
		for(i=0;i<l;i++)
			y=(y*10)+msg[i]-48;
		printf("%d \n",y);
		if (sendto(sendrsfd, (char *)packet, sizeof(packet), 0, 
			(struct sockaddr *)&daddr, (socklen_t)sizeof(daddr)) < 0)
			perror("packet send error:");
	}
}

void* contestant(void* pa)
{
	int x=(int)(int *) pa;
	printf("%d\n",x);
	struct sockaddr_in saddr;
	char packet[50];

	saddr.sin_family = AF_INET;
	saddr.sin_port = 0;
	inet_pton(AF_INET, SRC, (struct in_addr *)&saddr.sin_addr.s_addr);
	memset(saddr.sin_zero, 0, sizeof(saddr.sin_zero));
	memset(packet, 0, sizeof(packet));
	socklen_t *len = (socklen_t *)sizeof(saddr);
	int fromlen = sizeof(saddr);

	int n;
	while(1) {
		if ((n=recvfrom(recvrsfd[x], (char *)&packet, sizeof(packet), 0,
			(struct sockaddr *)&saddr, &fromlen) )< 0)
			perror("packet receive error:");

		int i = sizeof(struct iphdr);
		int y=0;
		while (i < n && packet[i]!=0) 
		{
			y=(y*10)+packet[i]-48;
			i++;
		}
	int j=0,k=0;
	for(;j<5;j++)
		for(;k<6;k++)
			if(rec[x][j][k]==y)
				rec[x][j][k]*=-1;
	for(j=0;j<5;j++)
	{
		int f=0;
		for(k=0;k<6;k++)
		{
			if(rec[x][j][k]>0)
			{
				f=1;
				break;
			}
		}
		if(f==0)
		{
			refv=x;

			//exit(0);
		}
	}
		printf("\n");
	}
}
int main()
{
	int i,j,k;

	for(i=0;i<3;i++)
		for(j=0;j<10;j++)
			man[i][j]=(i*10)+j+1;

	for(i=0;i<3;i++)
		for(j=0;j<5;j++)
			for(k=0;k<6;k++)
				rec[i][j][k]=(6*j)+k+1;

	if ((sendrsfd = socket(AF_INET, SOCK_RAW, 144)) < 0) 
	{
		perror("error:");
		exit(EXIT_FAILURE);
	}

	int zero=1;
	const int *val=&zero;
	setsockopt(sendrsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(zero));

	for(i=0;i<3;i++)
		if ((recvrsfd[i] = socket(AF_INET, SOCK_RAW, 144)) < 0) 
		{
			perror("error:");
			exit(EXIT_FAILURE);
		}
		
	pthread_t sender,receiver[3],check;

	pthread_create(&sender,NULL,manager,NULL);
	pthread_create(&check,NULL,verify,NULL);
	for(i=0;i<3;i++)
		pthread_create(&receiver[i],NULL,contestant,(void *)i);
	pthread_join(check,NULL);
	return 0;
}