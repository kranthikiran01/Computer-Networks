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
    int s[3],ss[3], s2, t, len,sfd,sf;
    struct sockaddr_un local, remote;
    char str[100];

    int i=0;
    for(i=0;i<3;i++)
    {
        if ((s[i] = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        local.sun_family = AF_UNIX;

        char sock[11] = {'\0'};
        strcpy(sock,SOCK_PATH);
        char ch[2] = {'\0'};
        ch[0] = (char)(((int)'0')+i);
        strcat(sock,ch);

        strcpy(local.sun_path, sock);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(s[i], (struct sockaddr *)&local, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(s[i], 5) == -1) {
            perror("listen");
            exit(1);
        }

    }
    for(i=0;i<3;i++)
    {
        if ((ss[i] = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        local.sun_family = AF_UNIX;

        char socks[12] = {'\0'};
        strcpy(socks,SOCK_PATH1);
        char chs[2] = {'\0'};
        chs[0] = (char)(((int)'0')+i);
        strcat(socks,chs);

        strcpy(local.sun_path, socks);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(ss[i], (struct sockaddr *)&local, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(ss[i], 5) == -1) {
            perror("listen");
            exit(1);
        }

    }

    int flag[3];
    for(i=0;i<3;i++)
        flag[i] = 0;

    fd_set readfds;

    struct timespec tim_dur;
    tim_dur.tv_sec = 5;   // waiting time of 1 sec for verifying set bits of FDSET
    tim_dur.tv_nsec = 0;

    while(1)
    {
        FD_ZERO(&readfds);
        //FD_SET(sfd,&readfds);
        for(i=0;i<3;i++)
            FD_SET(s[i],&readfds);

        // find max fd

        int max=0;
        for(i=0;i<3;i++)
            if(max<s[i])
                max=s[i];

        // pselect for finding if data is available
        //printf("HEllo\n");

        if(pselect(max+1,&readfds,NULL,NULL,&tim_dur,NULL)>0) // pselect is used as timeleft is not updated after each select call
        {
            /*if(FD_ISSET(sfd,&readfds))
            {
                    printf("Client came have fun mr.server :P\n");
                    csfd[3++] = accept(sfd,(struct sockaddr*)&cli_addr,&cli_len); // Type cast sockaddr_in to sockaddr and pass by reference and pointer of length to be passed
                    if(csfd < 0)
                    derror("Accept error");
            }*/
            for(i=0;i<3;i++)
            {
                if(FD_ISSET(s[i],&readfds))
                {
                    printf("Client came have fun mr.server %d :P\n",i);
                    if(flag[i]==0)
                    {
                        int c = 0;
                        c = fork();
                        if(c>0)
                        {
                            flag[i]= 1;
                            int ssfd =  accept(ss[i], (struct sockaddr *)&remote, &t);
                            int sfd =  accept(s[i], (struct sockaddr *)&remote, &t);
                            printf("%d\n",send_file_descriptor(ssfd,sfd));
                           
                        }
                        else
                        {
                            int j=0;
                            for(j;j<3;j++)
                                if(j!=i)
                                    close(s[j]);
                            //dup2(sfd[i],0);
                            //dup2(sfd[i],1);
                            if(i==0)
                            {
                               execlp("./serviceserver.out","hello",(char *)0);
                            }
                            else if(i==1)
                            {
                                execlp("./serviceserver1.out","hello",(char *)0);
                            }
                            else if(i==2)
                            {
                                execlp("./serviceserver2.out","hello",(char *)0);
                            }
                        }

                    }
                }
            }
        }
    }

         
    return 0;
}
