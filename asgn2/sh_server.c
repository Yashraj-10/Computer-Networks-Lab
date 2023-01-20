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

//--------------------ALTERNATE WAY TO BIND OUTPUT OF dir COMMAND TO A STRING------------------
// void filesBinding(char **files, char *toAdd, int i)
// {
//     printf("-%s-\n", toAdd);
//     if (i)
//     {
//         *files = (char *)realloc((*files), strlen(*files) + 1 + strlen(toAdd) + 1);
//         *files[strlen(*files)] = ' ';
//         int j;
//         for(j=strlen(*files)+1;j<strlen(*files) + 1 + strlen(toAdd);j++)
//         {
//             *files[j] = toAdd[j-strlen(*files)-1];
//         }
//         *files[j] = '\0';
//     }
//     else
//     {
//         *files = (char *)malloc(strlen(toAdd) + 1);
//         strcpy(*files, toAdd);
//         *files[strlen(*files)] = '\0';
//     }
//     printf("-%s-\n", *files);
// }

// Function to merge the packets recieved from the client
void message_packet_merging(int sockfd, char *buf){

    int index = 0;
    char temp[50];
    int halt = 0;
    
    while(!halt){

        int x = recv(sockfd, temp, 50, 0);
        if(x < 0){
            perror("Unable to get data\n");
            exit(0);
        }
        
        for (int i = 0; i < x; i++)
        {      
            if(temp[i] == '\0'){
                halt = 1;
                buf[i+index]='\0';
                break;
            }
            buf[i+index] = temp[i];
                
        }
        index += x;
    }
}

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

        if (fork() == 0)
        {
            close(sockfd);

            printf("!\n");
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
                line[strlen(line) - 1] = '\0';
                // printf("%s-\n", line);
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
                char command[1000];        // Buffer for storing the command
                for (i = 0; i < 50; i++) // Initializing the command buffer
                    command[i] = '\0';

                message_packet_merging(newsockfd, command); // Recieving the command from the client

                // printf("%s-\n", command);                    -----
                //                                                  |
                // char commandType1[2];                            |
                // strncpy(commandType1, command, 3);               |
                // printf("%s-1\n", commandType1);                  |
                // char commandType2[3];                            |
                // strncpy(commandType2, command, 4);               |-----> Alternate way to compare commands
                // printf("%s-2\n", commandType2);                  |
                //                                                  |
                // char command1[3] = "cd\0";                       |
                // char command2[4] = "dir\0";                      |
                // char command3[4] = "pwd\0";                  -----

//--------------CD--------------------------------------------------------------------------------------------------------------------------------
                if (command[0] == 'c' && command[1] == 'd' && (command[2] == '\0' || command[2] == ' '))
                {
                    // printf("cd--\n");
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
//--------------DIR-------------------------------------------------------------------------------------------------------------------------------
                else if (command[0] == 'd' && command[1] == 'i' && command[2] == 'r' && (command[3] == '\0' || command[3] == ' '))
                {
                    // printf("dir--\n");
                    char *path = command + 4;
                    char *path2 = ".";
                    printf("%s-\n", path);
                    DIR *dir;
                    struct dirent *ent;
                    char *files = (char *)malloc(1000 * sizeof(char));

                    if ((dir = opendir(path)) != NULL && strlen(command)>4)
                    {
                        while ((ent = readdir(dir)) != NULL)
                        {
                            // filesBinding(&files, ent->d_name, i);
                            strcat(files, ent->d_name);
                            strcat(files, "\n");
                            // send(newsockfd, ent->d_name, strlen(ent->d_name) + 1, 0);
                        }
                        send(newsockfd, files, strlen(files) + 1, 0);
                        printf("%s-f\n", files);
                        // free(files);
                        closedir(dir);
                    }
                    else if ((dir = opendir(path2)) != NULL && strlen(command)<=4)
                    {   
                        while ((ent = readdir(dir)) != NULL)
                        {
                            // filesBinding(&files, ent->d_name, i);
                            strcat(files, ent->d_name);
                            strcat(files, "\n");
                            // send(newsockfd, ent->d_name, strlen(ent->d_name) + 1, 0);
                        }
                        send(newsockfd, files, strlen(files) + 1, 0);
                        printf("%s-f\n", files);
                        // free(files);
                        closedir(dir);
                    }
                    else
                    {
                        send(newsockfd, errorInRunningCommand, strlen(errorInRunningCommand) + 1, 0);
                    }
                }
//--------------PWD-------------------------------------------------------------------------------------------------------------------------------
                else if (command[0] == 'p' && command[1] == 'w' && command[2] == 'd' && command[3] == '\0')
                {
                    printf("pwd--\n");
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
    }
    return 0;
}
