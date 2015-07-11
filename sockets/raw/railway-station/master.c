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
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

#define SOCK_PATH1 "./socket1"


int xid;
int yid;
int *x;
int *y;
int sem;

int ucss[3];
int flag[3];
int sq[10];
int count = 0;
int pids[3];

union semun
{
    int val;
    struct semid_ds *bf;
    unsigned short int  *array;
};
struct sembuf sbuf;

static int send_file_descriptor(
  int socket, /* Socket through which the file descriptor is passed */
  int fd_to_send) /* File descriptor to be passed, could be another socket */
{
 struct msghdr message;
 struct iovec iov[1];
 struct cmsghdr *control_message = NULL;
 char ctrl_buf[CMSG_SPACE(sizeof(int))];
 char data[1];

 memset(&message, 0, sizeof(struct msghdr));
 memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

 /* We are passing at least one byte of data so that recvmsg() will not return 0 */
 data[0] = ' ';
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

static void sighandler(int p)
{
    if(p == SIGUSR1)
    {
        signal(SIGUSR1,sighandler);
        int cm = *x;
        printf("Signalled %d\n",cm);
        if(cm==0)
        {
            if(count>=1)
            send_file_descriptor(ucss[cm],sq[(--count)]);
            else
                flag[cm] = 0;
        }
        else if(cm==1)
        {
            if(count>=1)
            send_file_descriptor(ucss[cm],sq[(--count)]);
            else
                flag[cm] = 0;
        }
        else if(cm==2)
        {
            if(count>=1)
            send_file_descriptor(ucss[cm],sq[(--count)]);
            else
                flag[cm] = 0;
        }
        sbuf.sem_op = 1;
        semop(sem,&sbuf,1);
    }
}



int main(void)
{
    int uss, bsfd[3], thop, len,sfd,sf;
    struct sockaddr_un local, remote;
    struct sockaddr_in ser_addr,cli_addr;
    char str[100];

    signal(SIGUSR1,sighandler);

    sem = semget(134,1,IPC_CREAT|0666);
    semctl(sem,0,SETVAL,1);
    xid = shmget(123,4,IPC_CREAT|0666);
    x =  shmat(xid,NULL,0);

    int i=0;


    for(i=0;i<3;i++)
    {
        bsfd[i] = socket(AF_INET,SOCK_STREAM,0); // socket create... AF_INET for IPv4 domain and SOCK_STREAM for connection oriented systems
        if(bsfd[i] < 0)
        perror("Socket create");
    }

    memset(&ser_addr,0,sizeof(struct sockaddr_in)); // Initialize to 0
    memset(&cli_addr,0,sizeof(struct sockaddr_in));

    ser_addr.sin_family = AF_INET;
    int portno = 27234;
    ser_addr.sin_port = htons(portno); // converts int to 16 bit integer in network byte order
    ser_addr.sin_addr.s_addr = INADDR_ANY; // to get IP address of machine on which server is running

    for(i=0;i<3;i++)
    {
        if(bind(bsfd[i],(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
            perror("Bind error");
        portno++;
        ser_addr.sin_port = htons(portno);
    }
    for(i=0;i<3;i++)
        printf("%d\n",bsfd[i] );
    
    for(i=0;i<3;i++)
        if(listen(bsfd[i],10)<0) // No. of clients that server can service
            perror("Listen error");


/*UNIX SOCKETS*/
        if ((uss = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        local.sun_family = AF_UNIX;
       strcpy(local.sun_path, SOCK_PATH1);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(uss, (struct sockaddr *)&local, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(uss, 5) == -1) {
            perror("listen");
            exit(1);
        }



    // int css[3];
    int selfpid = getpid();
    for(i=0;i<3;i++)
    {
        char pid[10] = {'\0'};

        ucss[i] = accept(uss, (struct sockaddr *)&remote, &thop);
        printf(" FD %d\n",ucss[i]);

        while(recv(ucss[i],pid,sizeof(pid),0)<=0);//printf("H");
        printf("Received %s\n",pid );
        pids[i] = atoi(pid);
        //itoa(selfpid,pid,10);
        snprintf(pid,10,"%d\n",selfpid);
        send(ucss[i],pid,sizeof(pid),0);
        printf("%d",pids[i]);
    }

    printf("PIDS EXCHANGED\n");

    
    for(i=0;i<3;i++)
    {
        printf("%d\n",ucss[i] );
        flag[i] = 0;
        count = 0;
    }

    fd_set readfds;

    


    struct timespec tim_dur;
    tim_dur.tv_sec = 5;   // waiting time of 1 sec for verifying set bits of FDSET
    tim_dur.tv_nsec = 0;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(bsfd[0],&readfds);
        FD_SET(bsfd[1],&readfds);
        FD_SET(bsfd[2],&readfds);
        int max = bsfd[0];


        for(i=0;i<3;i++)
            if(max<bsfd[i])
                max=bsfd[i];

        if(pselect(max+1,&readfds,NULL,NULL,&tim_dur,NULL)>0) // pselect is used as timeleft is not updated after each select call
        {
            for(i=0;i<3;i++)
            {
                if(FD_ISSET(bsfd[i],&readfds))
                {
                        printf("Client came have fun mr.server :P\n");
                        int nsfd = accept(bsfd[i], (struct sockaddr *)&remote, &thop); // Type cast sockaddr_in to sockaddr and pass by reference and pointer of length to be passed
                        if(nsfd < 0)
                        perror("Accept error");

                        int cm = i;
                        printf("Selected %d\n",cm );
                        int j=0;
                        for(j=0;j<3;j++)
                            printf("%d\n",flag[j] );
                        printf("\n");
                        for(j=0;j<3;j++)
                            if(flag[j]==0)
                                break;

                            printf("%d\n",j );
                        if(j<3)
                        {
                            printf("CSS %d nsfd %d\n",ucss[j],nsfd );
                            printf("%d\n",send_file_descriptor(ucss[j],nsfd));
                            flag[j]=1;
                        }
                        else
                        {
                            char sry[10];
                            strcpy(sry,"Wait!!");
                            send(nsfd,sry,sizeof(sry),0);
                            sq[count++] = nsfd;
                        }

                }
            }
                
        }
    }
           
    return 0;
}
