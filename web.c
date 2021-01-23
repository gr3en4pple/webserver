#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>
#include <fcntl.h>

char imageheader[] = "HTTP/1.1 200 Ok\r\n"
                     "Content-Type: image/jpeg\r\n\r\n";

char indexpage[] = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                   "<!DOCTYPE html>\r\n"
                   "<html><head><title>Test Index Page</title>"
                   "<style>body { background-color: #111 }"
                   "h1 { font-size:4cm; text-align: center; color: black;"
                   " text-shadow: 0 0 2mm red}</style></head>"
                   "<body><h1>Hello world</h1><br>\r\n"
                   "<img src=\"test.jpg\"><center></body></html>\r\n";

char lienhe[] = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                "<!DOCTYPE html>\r\n"
                "<html><head><title style=\"color:blue\">Contacts</title></head>"
                "<body><h1>CONTACT US</h1>"
                "<p style=\"font-size:30px; font-weight:bold; color:blueviolet\">1712405 - Nguyen Truong Giang<br />1712408 - Nguyen Son Hai<br />1712409 - Pham Thanh Hai</p>"
                "</body></html>\r\n";

char error[] = "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html; charset=UTF-8\r\n\r\n"
               "<!DOCTYPE html>\r\n"
               "<html><head><title>404 Error</title></head>"
               "<body><h1>404 Not Found</h1>"
               "</body></html>\r\n";

int main()
{
    struct sockaddr_in sv_addr, cli_addr;
    socklen_t sin_len = sizeof(cli_addr);
    char buffer[2048], method[50], path[256];
    int fd_sv, fd_cli, fd_jpg, one = 1;

    fd_sv = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_sv < 0)
    {
        perror("Cannot create socket:");
        exit(1);
    }

    setsockopt(fd_sv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = htons(8080);

    if (bind(fd_sv, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) == -1)
    {
        perror("Error on binding:");
        close(fd_sv);
        exit(1);
    }

    if (listen(fd_sv, 100) == -1)
    {
        perror("Failed to listen");
        close(fd_sv);
        exit(1);
    }

    while (1)
    {
        fd_cli = accept(fd_sv, (struct sockaddr *)&cli_addr, &sin_len);
        printf("New connection %d accepted\n", fd_sv);
        if (fd_cli < 0)
        {
            perror("Create socket error:");
            continue;
        }
        if (fork() == 0)
        {
            memset(buffer, 0, 2048);
            read(fd_cli, buffer, 2047);
            sscanf(buffer, "%s %s", method, path);

            if (strcmp(path, "/lienhe.html") == 0)
            {
                write(fd_cli, lienhe, sizeof(lienhe) - 1);
                // close(fd_cli);
            }
            else if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)
            {
                if (strncmp(buffer, "GET /test.jpg", 13) == 0)
                {
                    write(fd_cli, imageheader, sizeof(imageheader) - 1);
                    fd_jpg = open("test.jpg", O_RDONLY);
                    sendfile(fd_cli, fd_jpg, NULL, 0x5F5E100);
                    close(fd_jpg);
                }
                write(fd_cli, indexpage, sizeof(indexpage) - 1);
                // close(fd_cli);
            }
            else
            {
                write(fd_cli, error, sizeof(error) - 1);
                // close(fd_cli);
            }
            exit(0);
        }
        close(fd_cli);
    }
    close(fd_sv);

    return 0;
}
