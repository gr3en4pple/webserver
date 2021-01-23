#define maxlength 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>


typedef struct { 
  long mtype;
  char mtext[maxlength];
} msgq;


int sendMessage(char message[])
{
    //socket
    int serverq_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverq_sock < 0)
    {
       perror("socket");
       exit(1);
    } 
        

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("192.168.0.1");
    saddr.sin_port = htons(8080);

    if (connect(serverq_sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
        report("connect error", 1);

    write(serverq_sock, message, strlen(message));
    close(serverq_sock);
    return 0;
}

int main()
{
    char returnMessage[maxlength];
    key_t key = ftok("progfile", 111); 
    if (key < 0)
    {
        perror("key");
        exit(1);
    }

    int mqid = msgget(key, 0666 | IPC_CREAT); 
    if (mqid < 0)
    {
        perror("message queue");
        exit(1);
    }
        
        
    while (1)
    {
        struct msqid_ds buf;
        int rc = msgctl(qid, IPC_STAT, &buf);
        if ((uint)(buf.msg_qnum) == 0)
        {
            continue;
        }
        msgq msg;
        if (msgrcv(qid, &msg, sizeof(msg), 1, MSG_NOERROR | IPC_NOWAIT) < 0)
            perror("receive from queue");
        sendMessage(msg.mtext);
    }
    msgctl(qid, IPC_RMID, NULL);
        

    return 0;
}
