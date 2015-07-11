#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
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
    int s, s2, t, len,sfd,sf;
    struct sockaddr_un local, remote;
    char str[100];

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


    if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH1);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sfd, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
    t = sizeof(remote);
    int sfd1 =  accept(sfd, (struct sockaddr *)&remote, &t);


    if ((sf = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH2);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sf, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sf, 5) == -1) {
        perror("listen");
        exit(1);
    }
    t = sizeof(remote);
    int sfd2 =  accept(sf, (struct sockaddr *)&remote, &t);

    printf("All connections Established %d\n",sfd1);

    for(;;) 
    {
        int done, n;
        printf("Waiting for a connection...\n");
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) 
        {
            perror("accept");
            exit(1);
        }
        
        printf("Connected.\n");
        char choice[2];
        char msg[11];
        while(recv(s2,&choice,sizeof(choice),0)<=0);
        strcpy(msg,"Redirected");
        send(s2,&msg,sizeof(msg),0);
        int ch = atoi(choice);
        if(ch==1)
        {
            printf("%d\n",send_file_descriptor(sfd1,s2));//!=-1)
                printf("FD SENT\n");
            
        }
        else
        {
            printf("%d\n",send_file_descriptor(sfd2,s2));//!=-1)
                printf("FD SENT\n");
           
        }
    }     
    return 0;
}
