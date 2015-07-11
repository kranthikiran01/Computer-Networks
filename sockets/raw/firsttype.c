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

int sendrsfd,recvrsfd;

void* my_send(void* pa)
{

	
	struct sockaddr_in daddr;
	char packet[50];
	char msg[10];
	/* point the iphdr to the beginning of the packet */
	struct iphdr *ip = (struct iphdr *)packet;  

	daddr.sin_family = AF_INET;
	daddr.sin_port = 0; /* not needed in SOCK_RAW */
	inet_pton(AF_INET, DEST, (struct in_addr *)&daddr.sin_addr.s_addr);
	memset(daddr.sin_zero, 0, sizeof(daddr.sin_zero));
	memset(packet,0, sizeof(packet));   /* payload will be all As */
	
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
		/* 16 byte value */
	ip->frag_off = 0;		/* no fragment */
	ip->ttl = 64;			/* default value */
	ip->protocol = 144;	/* protocol at L4 */
	ip->check = 0;			/* not needed in iphdr */
	ip->saddr = daddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;

	while(1) 
	{ 
		memset(msg,0, sizeof(msg));
		scanf("%s",msg);
		// scanf("%s",packet+sizeof(struct iphdr));
		strcpy(packet+sizeof(struct iphdr),msg);
		ip->tot_len = htons(sizeof(struct iphdr)+sizeof(msg));

		if (sendto(sendrsfd, (char *)packet, sizeof(packet), 0, 
			(struct sockaddr *)&daddr, (socklen_t)sizeof(daddr)) < 0)
			perror("packet send error:");
	}
}

void* my_receive(void* pa)
{
	struct sockaddr_in saddr;
	char packet[50];

	saddr.sin_family = AF_INET;
	saddr.sin_port = 0; /* not needed in SOCK_RAW */
	inet_pton(AF_INET, SRC, (struct in_addr *)&saddr.sin_addr.s_addr);
	memset(saddr.sin_zero, 0, sizeof(saddr.sin_zero));
	memset(packet, 0, sizeof(packet));
	socklen_t *len = (socklen_t *)sizeof(saddr);
	int fromlen = sizeof(saddr);

	int n;
	while(1) {
		if ((n=recvfrom(recvrsfd, (char *)&packet, sizeof(packet), 0,
			(struct sockaddr *)&saddr, &fromlen) )< 0)
			perror("packet receive error:");

		int i = sizeof(struct iphdr);	/* print the payload */
		while (i < n && packet[i]!=0) 
		{
			fprintf(stderr, "%c", packet[i]);
			i++;
		}
		printf(" %d \n",i);
	}
}
int main()
{
	

	if ((sendrsfd = socket(AF_INET, SOCK_RAW, 144)) < 0) 
	{
		perror("error:");
		exit(EXIT_FAILURE);
	}

	int zero=1;
	const int *val=&zero;

	setsockopt(sendrsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(zero));

	if ((recvrsfd = socket(AF_INET, SOCK_RAW, 144)) < 0) 
	{
		perror("error:");
		exit(EXIT_FAILURE);
	}

	pthread_t sender,receiver;
	pthread_create(&sender,NULL,my_send,NULL);
	pthread_create(&receiver,NULL,my_receive,NULL);
	pthread_join(sender,NULL);

	return 0;
}
