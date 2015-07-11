#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/select.h>

int csfd[10];
int ccount=0;

#define SOCK_PATH "./socket11"

void* service (void* param)
{
    fd_set readfds;
    int i=0;
    char msg[100];
    int max;
    struct timespec tim_dur;
    tim_dur.tv_sec = 5;   // waiting time of 1 sec for verifying set bits of FDSET
    tim_dur.tv_nsec = 0;
   while(1)
    {
        FD_ZERO(&readfds);
        for(i=0;i<10;i++)
            FD_SET(csfd[i],&readfds);

        // find max fd

        max=0;
        for(i=0;i<10;i++)
            if(max<csfd[i])
                max=csfd[i];

        // pselect for finding if data is available
        printf("HEllo %d\n",ccount);
        if(pselect(max+1,&readfds,NULL,NULL,&tim_dur,NULL)>0); // pselect is used as timeleft is not updated after each select call
        {
            for(i=0;i<10;i++)
            {
                if(FD_ISSET(csfd[i],&readfds))
                {
                    int np = recv(csfd[i],&msg,sizeof(msg),0);
                    send(csfd[i],&msg,np,0);
                }
            }
        }
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

     if((res = recvmsg(socket, &message, 0)) <= 0)
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
       csfd[ccount++] = sd;
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
    pthread_t receivefd,clientser;
    int *param = &s;
    pthread_create(&receivefd,NULL,recv_file_descriptor,(void*)param);
    while(ccount<=0);
    pthread_create(&clientser,NULL,service,NULL);

    pthread_join(receivefd,NULL);
    return 0;
}

