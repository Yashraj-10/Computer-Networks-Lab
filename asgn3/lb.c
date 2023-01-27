/* THE LOAD BALANCER PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        perror("No port number provided\n");
        exit(0);
    }
    /*
    argv[1] = port number of load balancer
    argv[2] = port number of server 1
    argv[3] = port number of server 2
    */

    int serverPort1 = atoi(argv[2]);
    int serverPort2 = atoi(argv[3]);
    char loadSTR1[10];
    char loadSTR2[10];
    int load1 = 0;
    int load2 = 0;

    char *sendTime = "Send Time";
    char *sendLoad = "Send Load";

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int i;
    // Opening a socket and exiting if failed
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // Specifying the load balancer address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Trying to bind the socket with the load balancer address and exiting if failed
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    // Listening for connections and specifying maximum number of pending connections to be 5
    listen(sockfd, 5);

    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN;

    // Implementing an iterative server
    while (1)
    {
        int ret = poll(&fds, 1, 5000); // Waiting for 5 secs to ask the server for load
        if (ret == 0)
        {
            int sockfd1_load, sockfd2_load;
            struct sockaddr_in serv_addr1_load, serv_addr2_load; // Specifying the server address

            //----------Server 1----------------
            if ((sockfd1_load = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket\n");
                exit(0);
            }

            serv_addr1_load.sin_family = AF_INET;
            inet_aton("127.0.0.1", &serv_addr1_load.sin_addr);
            serv_addr1_load.sin_port = htons(serverPort1);

            if (connect(sockfd1_load, (struct sockaddr *)&serv_addr1_load, sizeof(serv_addr1_load)) < 0)
            {
                perror("Unable to connect to server 1\n");
                exit(0);
            }

            send(sockfd1_load, sendLoad, strlen(sendLoad) + 1, 0);
            recv(sockfd1_load, &loadSTR1, strlen(loadSTR1), 0);
            close(sockfd1_load);

            load1 = atoi(loadSTR1);

            //----------Server 2----------------
            if ((sockfd2_load = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket\n");
                exit(0);
            }

            serv_addr2_load.sin_family = AF_INET;
            inet_aton("127.0.0.1", &serv_addr2_load.sin_addr);
            serv_addr2_load.sin_port = htons(serverPort2);

            if (connect(sockfd2_load, (struct sockaddr *)&serv_addr2_load, sizeof(serv_addr2_load)) < 0)
            {
                perror("Unable to connect to server 2\n");
                exit(0);
            }

            send(sockfd2_load, sendLoad, strlen(sendLoad) + 1, 0);
            recv(sockfd2_load, &loadSTR2, strlen(loadSTR2), 0);
            close(sockfd2_load);

            load2 = atoi(loadSTR2);
        }
        else
        {

            clilen = sizeof(cli_addr);                                         // Specifying the size of the client address
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // Accepting a connection

            // Exiting if accept failed
            if (newsockfd < 0)
            {
                perror("Accept error\n");
                exit(0);
            }

            if (fork() == 0)
            {
                close(sockfd);

                if (load1 < load2)
                {
                    int sockfd1;
                    struct sockaddr_in serv_addr1;

                    // Opening a socket and exiting if failed
                    if ((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        perror("Unable to create socket\n");
                        exit(0);
                    }

                    // Specifying the server address
                    serv_addr1.sin_family = AF_INET;
                    inet_aton("127.0.0.1", &serv_addr1.sin_addr);
                    serv_addr1.sin_port = htons(serverPort1);

                    int serLength = sizeof(serv_addr1);

                    // Trying to connect to the server and exiting if failed
                    if (connect(sockfd1, (struct sockaddr *)&serv_addr1, serLength) < 0)
                    {
                        perror("Unable to connect to server 1\n");
                        exit(0);
                    }

                    // Sending the request to the server
                    send(sockfd1, sendTime, strlen(sendTime) + 1, 0);

                    // Receiving the time from the server and closing the connection
                    char buffer[20];
                    recv(sockfd1, buffer, 20, 0);
                    close(sockfd1);

                    // Sending the time back to the client
                    send(newsockfd, buffer, strlen(buffer) + 1, 0);
                    close(newsockfd);
                }
                else
                {
                    int sockfd2;
                    struct sockaddr_in serv_addr2;

                    // Opening a socket and exiting if failed
                    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        perror("Unable to create socket\n");
                        exit(0);
                    }

                    // Specifying the server address
                    serv_addr2.sin_family = AF_INET;
                    inet_aton("127.0.0.1", &serv_addr2.sin_addr);
                    serv_addr2.sin_port = htons(serverPort2);

                    // Trying to connect to the server and exiting if failed
                    if (connect(sockfd2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0)
                    {
                        perror("Unable to connect to server 2\n");
                        exit(0);
                    }

                    // Sending the request to the server
                    send(sockfd2, sendTime, strlen(sendTime) + 1, 0);

                    // Receiving the time from the server and closing the connection
                    char buffer[20];
                    recv(sockfd2, buffer, 20, 0);
                    close(sockfd2);

                    // Sending the time back to the client
                    send(newsockfd, buffer, strlen(buffer) + 1, 0);
                    close(newsockfd);
                }

                close(newsockfd); // Closing the socket
                exit(0);
            }//fork end

            close(newsockfd); // Closing the socket
        }//poll end
    }//while end
}//main end
