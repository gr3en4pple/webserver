#define maxlength 512

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

typedef struct { 
  long mtype;
  char mtext[maxlength];
} msgq;

int main()
{
    
    key_t key = ftok("progfile",111 );
    if (key < 0)
    {
        perror("key");
        exit(1);
    }

  int qid = msgget(key, 0666 | IPC_CREAT);
    if (qid < 0)
   {
       perror("queue id");
       exit(1);
   }

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    {
    perror("socket");
    exit(1);
    }

  //bind
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr("192.168.0.1");
  saddr.sin_port = htons(8080);

  if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
   {
       perror("bind");
       exit(1);
   }

  if (listen(fd, 20) < 0)
    {
        perror("listen");
        exit(1);
    }

  while (1)
  {
    struct sockaddr_in cli_addr;
    int cli_len = sizeof(cli_addr);

    int client_fd = accept(fd, (struct sockaddr *)&cli_addr, &cli_len);
    if (client_fd < 0)
    {
    perror("accept error");
    continue;
    }

    char buffer[maxlength + 1];
    memset(buffer, '\0', sizeof(buffer));
    int count = read(client_fd, buffer, sizeof(buffer));
    if (count > 0)
    {
      msgq msg;
      msg.mtype = 1;
      strcpy(msg.mtext, buffer);
      /* send the message */
      msgsnd(qid, &msg, sizeof(msg), IPC_NOWAIT);
      printf("%s wrote to message queue\n", msg.mtext);
    }
    close(client_fd);
  }
  return 0;
}