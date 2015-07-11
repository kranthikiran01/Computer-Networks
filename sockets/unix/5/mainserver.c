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

#define SOCK_PATH "./socket"
#define SOCK_PATH1 "./socket1"
#define SOCK_PATH2 "./socket2"
#define MAXFORKS 3

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

int main(void)
{
    int s,ss, s2, t, len,sfd,sf;
    struct sockaddr_un local, remote;
    char str[100];

    int i=0;
        if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCK_PATH);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(s, (struct sockaddr *)&local, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(s, 5) == -1) {
            perror("listen");
            exit(1);

        }

        if ((ss = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCK_PATH1);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(ss, (struct sockaddr *)&local, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(ss, 5) == -1) {
            perror("listen");
            exit(1);
        }

    int pid,j;
    for (i = 0; i < MAXFORKS; ++i)
    {
        if((pid=fork())==0)
        {
            execlp("./childserver.out","hello",(char *)0);
        }
    }

    int cs[MAXFORKS];


    for(i=0;i<MAXFORKS;i++)
    {
        cs[i] = accept(ss, (struct sockaddr *)&remote, &t);
    }

    int flag[MAXFORKS];
    for(i=0;i<MAXFORKS;i++)
        flag[i] = 0;

    fd_set readfds;

    struct timespec tim_dur;
    tim_dur.tv_sec = 5;   // waiting time of 1 sec for verifying set bits of FDSET
    tim_dur.tv_nsec = 0;

    int ccount=-1;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(s,&readfds);
        for(i=0;i<MAXFORKS;i++)
            FD_SET(cs[i],&readfds);

        // find max fd

        int max=0;
        for(i=0;i<MAXFORKS;i++)
            if(max<cs[i])
                max=cs[i];

        // pselect for finding if data is available
        //printf("HEllo\n");

        if(pselect(max+1,&readfds,NULL,NULL,&tim_dur,NULL)>0) // pselect is used as timeleft is not updated after each select call
        {
            if(FD_ISSET(s,&readfds))
            {
                    printf("Client came have fun mr.server :P\n");
                    int nsfd = accept(s, (struct sockaddr *)&remote, &t); // Type cast sockaddr_in to sockaddr and pass by reference and pointer of length to be passed
                    if(nsfd < 0)
                        perror("Accept error");
                    printf("%d\n",send_file_descriptor(cs[((++ccount)%MAXFORKS)],nsfd));
            }
            for(i=0;i<MAXFORKS;i++)
            {
                if(FD_ISSET(cs[i],&readfds))
                {
                    int t=recv(cs[i], str, 100, 0);
                    printf("%s from %d\n",str,i );
                }
            }
        }
    }
     
    return 0;
}
