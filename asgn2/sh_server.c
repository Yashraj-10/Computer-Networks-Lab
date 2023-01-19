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

#include <dirent.h>

int main()
{
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int i;

    char *login = "LOGIN:";
    char *found = "FOUND";
    char *nfound = "NOT-FOUND";
    char *invalidCommand = "$$$$";
    char *errorInRunningCommand = "####";

    // Opening a socket and exiting if failed
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // Specifying the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

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

        if (fork == 0)
        {
            char username[25];       // Buffer for storing the username
            for (i = 0; i < 25; i++) // Initializing the username buffer
                username[i] = '\0';

            send(newsockfd, login, strlen(login) + 1, 0); // Sending the message "LOGIN:" to the client
            recv(newsockfd, username, 25, 0);             // Recieving the username from the client

            // Opening the file and reading it
            FILE *fp;
            fp = fopen("usernames.txt", "r");
            char line[25];
            int flag = 0;
            while (fgets(line, 25, fp) != NULL)
            {
                if (strcmp(line, username) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            fclose(fp);

            if (flag == 1)
                send(newsockfd, found, strlen(found) + 1, 0); // Sending the message "FOUND" to the client
            else
                send(newsockfd, nfound, strlen(nfound) + 1, 0); // Sending the message "NOT-FOUND" to the client

            while (1)
            {
                char command[50];        // Buffer for storing the command
                for (i = 0; i < 50; i++) // Initializing the command buffer
                    command[i] = '\0';

                recv(newsockfd, command, 50, 0); // Recieving the command from the client

                char commandType1[2];
                strncpy(commandType1, command, 2);
                char commandType2[3];
                strncpy(commandType2, command, 3);

                char command1[2] = "ch";
                char command2[3] = "dir";
                char command3[3] = "pwd";

                if (strcmp(command1, commandType1) == 0)
                {
                    char *path = command + 3;
                    if (chdir(path) == 0)
                    {
                        send(newsockfd, "Changed directory", strlen("Changed directory") + 1, 0);
                    }
                    else
                    {
                        send(newsockfd, errorInRunningCommand, strlen(errorInRunningCommand) + 1, 0);
                    }
                }
                else if (strcmp(command2, commandType2) == 0)
                {
                    char *path = command + 4;
                    DIR *dir;
                    struct dirent *ent;
                    if ((dir = opendir(path)) != NULL)
                    {
                        while ((ent = readdir(dir)) != NULL)
                        {
                            send(newsockfd, ent->d_name, strlen(ent->d_name) + 1, 0);
                        }
                        closedir(dir);
                    }
                    else
                    {
                        send(newsockfd, errorInRunningCommand, strlen(errorInRunningCommand) + 1, 0);
                    }
                }
                else if (strcmp(command3, commandType2) == 0)
                {
                    long size;
                    char *buf;
                    char *path;

                    size = pathconf(".", _PC_PATH_MAX);

                    if ((buf = (char *)malloc((size_t)size)) != NULL)
                        path = getcwd(buf, (size_t)size);

                    send(newsockfd, path, strlen(path) + 1, 0);
                }
                else
                {
                    send(newsockfd, invalidCommand, strlen(invalidCommand) + 1, 0);
                }
            }

            close(newsockfd); // Closing the socket
            exit(0);
        }

        close(newsockfd); // Closing the socket

        // time_t t; // Variable to store the time using a non primitive data type
        // time(&t); // Storing the time in t

        // char timeNdate[20];                                          // Variable to store the time and date in a string format
        // strftime(timeNdate, 20, "%H:%M:%S %d/%m/%Y", localtime(&t)); // Storing the time and date in a string format

        // send(newsockfd, timeNdate, strlen(timeNdate) + 1, 0); // Sending the time and date to the client
        // close(newsockfd);                                     // Closing the socket
    }
    return 0;
}
