/* THE UDP SERVER PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

int main()
{

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    char buffer[1000];
    int n;
    socklen_t len;

    // Opening a UDP socket and exiting if failed
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // Initializing the server and client addresses with 0
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Specifying the server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(20000);

    // Trying to bind the socket with the server address and exiting if failed
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind Failure");
        exit(EXIT_FAILURE);
    }

    // Implementing the UDP server
    len = sizeof(cliaddr);

    // Recieve client's request to get time
    n = recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr *)&cliaddr, &len);

    printf("CLIENT: %s\n", buffer);

    time_t t; // Variable to store the time using a non primitive data type
    time(&t); // Storing the time in t

    char timeNdate[20];                                          // Variable to store the time and date in a string format
    strftime(timeNdate, 20, "%H:%M:%S %d/%m/%Y", localtime(&t)); // Storing the time and date in a string format

    // Sending the time and date to the client
    sendto(sockfd, timeNdate, strlen(timeNdate) + 1, 0, (struct sockaddr *)&cliaddr, len);

    // Closing the socket
    close(sockfd);

    return 0;
}