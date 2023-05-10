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
        
    struct sockaddr_in server_addr, client_addr;
    socklen_t socklen;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int x = my_bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (x < 0)
    {
        perror("");
        exit(0);
    }
    x = my_listen(sockfd, 1);

    printf("SERVER IS LISTENING AT 6000\n");
    
    while (1)
    {
        int new_sockfd = my_accept(sockfd, (struct sockaddr *)&client_addr, &socklen);
        printf("NEW CLIENT CONNECTED FROM %d\n", client_addr.sin_port);

        if (new_sockfd < 0)
            perror("ERROR: ");
            
        char recv_msg[100];
        int len = my_recv(new_sockfd, recv_msg, 100, 0);
        printf("%d\n", len);

        for (int i = 0; i < len; i++)
            printf("%c", recv_msg[i]);
            
        printf("\n");
        char buff[5000];

        for (int i = 0; i < 5000; i++)
            buff[i] = 'H';

        my_send(new_sockfd, buff, strlen(buff), 0);
        for (int i = 0; i < 20; i++)
        {
            my_recv(new_sockfd, recv_msg, 100, 0);
            printf("%s", recv_msg);
        }

        printf("Done with client\n");
        my_close(new_sockfd);
    }

    return 0;
}