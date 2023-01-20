/* THE UDP CLIENT PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <poll.h>

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    char buffer[1000];          // Buffer for recieving data from server
    int n;
    socklen_t len;

    struct pollfd fds;

	// Opening a socket and exiting if failed
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(EXIT_FAILURE);
    }

    // Initializing the server and client addresses with 0
    memset(&servaddr, 0, sizeof(servaddr));

	// Specifying the server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    len = sizeof(servaddr);

    // Initialize the pollfd structure
    fds.fd = sockfd;
    fds.events = POLLIN;

    int i;              // Loop var for trying 5 times
    for(i=0;i<5;i++)
    {
        // printf("%d",i);
        // Sending a message to server to ask for their local time
        strcpy(buffer, "Whats the time???");

        if ((n = sendto(sockfd, buffer, sizeof(buffer) + 1, 0, (struct sockaddr *)&servaddr, len)) < 0)
        {
            perror("Data Sending Failure");
            exit(EXIT_FAILURE);
        }

        // Wait for a message from server, using Poll
        // poll with timeout of 3000 milliseconds
        int p = poll(&fds, 1, 3000);

        if(p<0)
        {
            perror("Polling Failure");  
            exit(EXIT_FAILURE);
        }
        if(p>0)
        {
            if ((n = recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr *)&servaddr, &len)) < 0)
            {
                perror("Data Receiving Failure");
                exit(EXIT_FAILURE);
            }

            printf("%s\n", buffer);

            break;
        }
    }

    // Error message when all attempts are exhausted
    if (i == 5)
    {
        printf("Timeout exceeded.\n");
    }

    // Close the socket
    close(sockfd);

    return 0;
}