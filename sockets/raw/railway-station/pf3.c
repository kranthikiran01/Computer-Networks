#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/select.h>
#include <netinet/ip.h>

#define SOCK_PATH "./socket1"
#define SRC "127.0.0.1"

int csfd;
int ccount=0;
int recvrsfd;

int xid;
int yid;
int *x;
int *y;
int sem;
int mainser;

int s1q[10],s2q[10],s3q[10];
int count[3] = {-1};
int pids[3];

union semun
{
    int val;
    struct semid_ds *bf;
    unsigned short int  *array;
};
struct sembuf sbuf;



void* service (void* param)
{
    fd_set readfds;
    int i=0;
    char msg1[100];
    strcpy(msg1,"Resource Allocated\n");
    send(csfd,msg1,sizeof(msg1),0);
    int n;
    while(1)
    {
        strcpy(msg1,"\nEnter the msg please: ");
        send(csfd,msg1,sizeof(msg1),0);
        char msg[10] = {'\0'};
        while((n = recv(csfd,msg,10,0))<=0);
       // printf("OTHER THAN EXIT %s\n",msg );
        if(strcmp(msg,"exit")==0)
        {
           // printf("IN EXIT\n");
            close(csfd);
            *x = 2;
            sbuf.sem_op = -1;
            semop(sem,&sbuf,1);
            kill(mainser,SIGUSR1);
        }
        send(csfd,msg,n,0);
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

        int i = sizeof(struct iphdr);   /* print the payload */
        fprintf(stderr, "ADVERTISEMENT : ");
        while (i < n && packet[i]!=0) 
        {
            fprintf(stderr, "%c", packet[i]);
            i++;
        }
        printf("\n",i);
    }
}

void* recv_file_descriptor(
  void* param) /* Socket from which the file descriptor is read */
{

 int socket = *((int*)param);

 int sent_fd;
 struct msghdr message;
 struct iovec iov[1];
 struct cmsghdr *control_message = NULL;
 char ctrl_buf[CMSG_SPACE(sizeof(int))];
 char data[1];
 int res;

 printf("Waiting to receive fd %d\n",socket);
while(1)
{
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

     // if((res = recvmsg(socket, &message, 0)) <= 0)
         // printf("Does not receive\n");
    recvmsg(socket, &message, 0);
  
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
       csfd = sd;
       ccount++;
       printf("Recieved\n");
      }
     }
 }
}

int main(void)
{
    int s, t, len;
    struct sockaddr_un remote;
    char str[100];

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }
    printf("Connected!! %d\n",s);

    sem = semget(134,1,IPC_CREAT|0666);
  //  semctl(sem,0,SETVAL,1);
    xid = shmget(123,4,IPC_CREAT|0666);
    x =  shmat(xid,NULL,0);

    int pid = getpid();
    char buf[10]={'\0'};
    //itoa(pid,buf,10);
    snprintf(buf,10,"%d\n",pid);
    send(s,buf,sizeof(buf),0);
    while(recv(s,buf,10,0)<=0);
    mainser = atoi(buf);

    printf("PIDS EXCHANGED\n");

    pthread_t receivefd,clientser;
    int *param = &s;

    if ((recvrsfd = socket(AF_INET, SOCK_RAW, 144)) < 0) 
    {
        perror("error:");
        exit(EXIT_FAILURE);
    }

    pthread_t sender,receiver;
    pthread_create(&receiver,NULL,my_receive,NULL);

    pthread_create(&receivefd,NULL,recv_file_descriptor,(void*)param);
    while(ccount<=0);
    pthread_create(&clientser,NULL,service,NULL);

    pthread_join(receivefd,NULL);
    return 0;
}

