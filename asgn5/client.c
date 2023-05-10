#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "mysocket.h"

int main()
{
    int sockfd = my_socket(AF_INET, 0, 0);
    if (sockfd < 0)
        perror("ERROR: ");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    if ((my_connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        perror("ERROR: ");
        exit(0);
    }

    char buff[100] = "HELLO SERVER!!";
    my_send(sockfd, buff, strlen(buff), 0);
    char recv_msg[5000];
    int len = my_recv(sockfd, recv_msg, 5000, 0);
    printf("%d\n", len);

    for (int i = 0; i < len; i++)
        printf("%c", recv_msg[i]);
        
    char buff2[100]="TEST\n";
    for (int i = 0; i < 20; i++)
        my_send(sockfd, buff2, strlen(buff2)+1, 0);
        
    my_close(sockfd);
    printf("\nDONE\n");

    return 0;
}