/* THE SERVER PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));
    
    if (argc <= 1)
    {
        perror("No port number provided\n");
        exit(0);
    }

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int i;
    char buf[100];

    // Opening a socket and exiting if failed
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // Specifying the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Trying to bind the socket with the server address and exiting if failed
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    // Listening for connections and specifying maximum number of pending connections to be 5
    listen(sockfd, 5);

    // Implementing an iterative server
    while (1)
    {
        clilen = sizeof(cli_addr);                                         // Specifying the size of the client address
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // Accepting a connection

        // Exiting if accept failed
        if (newsockfd < 0)
        {
            perror("Accept error\n");
            exit(0);
        }

        for (i = 0; i < 100; i++)
            buf[i] = '\0'; // Clearing the buffer

        recv(newsockfd, buf, 100, 0); // Receiving the message from the client

        if (buf[5] == 'L')
        {

            int num = rand() % 100 + 1;     // Generating a random number between 1 and 100
            printf("Load sent: %d\n", num); // Printing the random number

            char numstr[10];
            sprintf(numstr, "%d", num);                     // Converting the random number to a string
            send(newsockfd, numstr, strlen(numstr) + 1, 0); // Sending the random number to the client

            close(newsockfd); // Closing the socket
        }
        else
        {
            time_t t; // Variable to store the time using a non primitive data type
            time(&t); // Storing the time in t

            char timeNdate[20];                                          // Variable to store the time and date in a string format
            strftime(timeNdate, 20, "%H:%M:%S %d/%m/%Y", localtime(&t)); // Storing the time and date in a string format

            send(newsockfd, timeNdate, strlen(timeNdate) + 1, 0); // Sending the time and date to the client
            close(newsockfd);                                     // Closing the socket

            printf("Time sent\n");
        }
    }
    return 0;
}
