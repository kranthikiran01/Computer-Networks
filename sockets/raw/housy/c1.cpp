#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<linux/ip.h>
#include <netdb.h>
#include<arpa/inet.h>
#define DEST "127.0.0.1"
using namespace std;
int rsfd1;
struct sockaddr_in local,foreign;
int flag=0;
int nums[3][4];
void* sender(void *ptr)
{
        char packet[50];char buf[30];
	/* point the iphdr to the beginning of the packet */
	struct iphdr *ip = (struct iphdr *)packet;  
        ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = htons(40);	/* 16 byte value */
	ip->frag_off = 0;		/* no fragment */
	ip->ttl = 64;			/* default value */
	ip->protocol = 144;	/* protocol at L4 */
	ip->check = 0;			/* not needed in iphdr */
	ip->saddr =local.sin_addr.s_addr;
	ip->daddr = local.sin_addr.s_addr;

	while(1) {
		sleep(1);
                if(flag){flag=0;
                strcpy(buf,"c1...won");
                for(int i=0;i<strlen(buf);i++)
                *(packet+sizeof(struct iphdr)+i)=buf[i];
                ip->tot_len=htons(sizeof(struct iphdr)+strlen(buf));
		if (sendto(rsfd1, (char *)packet,sizeof(struct iphdr)+strlen(buf), 0, 
			(struct sockaddr *)&local, (socklen_t)sizeof(local)) < 0)
			perror("packet send error:");
	}}
}

void *receiver(void *ptr)
{
  int x = *((int*)(ptr));
  int rsfd2;
  if((rsfd2=socket(AF_INET,SOCK_RAW,145))<0)
 {
   perror("rsfd2 error\n");
 }
  sleep(1);
  char message[20];
  char packet[200];
  struct iphdr *ip;
  //ip=new iphdr();
  socklen_t len=sizeof(struct sockaddr_in);
  while(1)
  {
    
    int n=recvfrom(rsfd2,(char*)&packet,sizeof(packet),0,(struct sockaddr *)&foreign,(socklen_t *)&len);
    if(n>0)
    {
       ip=(struct iphdr*)packet;
      /* cout<<ip->id<<endl;
       //cout<<ip->ihl<<endl;
       cout<<ntohs(ip->tot_len)<<endl;
       cout<<n<<endl;
       cout<<sizeof(struct iphdr)<<endl;*/
       /*for(int i=sizeof(struct iphdr);i<n;i++)
       message[i-sizeof(struct iphdr)]=packet[i];
       message[n-sizeof(struct iphdr)]='\0';*/
      char c[10] = {'\0'};
      int k1=0;
      int k = sizeof(struct iphdr);  /* print the payload */
      while (k < sizeof(packet)) 
      {
        c[k1++] = packet[k];
        k++;
      }
       int rnum = atoi(c);
       // if(c=='1' || c=='2' ||c=='3')
       // flag=1;
       for(int i=0;i<4;i++)
        if(nums[x][i] == rnum)
        {
          nums[x][i] = 0;
          break;
        }
        int j=0;
          for(int pl = 0;pl<4;pl++)
            cout<<nums[x][pl]<<" ";
          cout<<endl;
        for(j=0;j<4;j++)
        if(nums[x][j] != 0)
        {
          break;
        }
        if(j>3)
          flag = 1;
      // cout<<endl;
    }
  }
}

int main()
{
  int timep = 1;
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<4;j++)
      nums[i][j] = timep++;
  }

 if((rsfd1=socket(AF_INET,SOCK_RAW,145))<0)
 {
   perror("rsfd1 error\n");
 }
 //cout<<rsfd1<<endl;
 int optval=1;
 const int *val=&optval;
 if(setsockopt(rsfd1,IPPROTO_IP, IP_HDRINCL, val, sizeof(optval))<0)
 {
   perror("setsockopt error\n");
   exit(0);
 }
 /*if(setsockopt(rsfd2,IPPROTO_IP, IP_HDRINCL, val, sizeof(optval))<0)
 {
   printf("setsockopt error\n");
   exit(0);
 }*/
 
 memset(&local,0,sizeof(struct sockaddr_in));
 memset(&foreign,0,sizeof(struct sockaddr_in));
 local.sin_family=AF_INET;
 inet_pton(AF_INET, DEST, (struct in_addr *)&local.sin_addr.s_addr);
 local.sin_port=htons(0);
 /*if(bind(rsfd1,(struct sockaddr*)&local,sizeof(struct sockaddr_in))<0)
 {
      printf("bind error\n");exit(0);
 }
 foreign.sin_family=AF_INET;
 foreign.sin_addr.s_addr=INADDR_ANY;
 /*if(connect(rsfd2,(struct sockaddr*)&foreign,sizeof(struct sockaddr_in))<0)
 {
     printf("connect error\n");exit(0);
 }*/
 pthread_t tid1,tid2;
 pthread_create(&tid1,NULL,sender,NULL);
 int p1 = 0, p2 = 1, p3 = 2;
 pthread_create(&tid2,NULL,receiver,(void*)(&p1));
 pthread_create(&tid2,NULL,receiver,(void*)(&p2));
 pthread_create(&tid2,NULL,receiver,(void*)(&p3));
 pthread_join(tid1,NULL);
 return 0;
}
