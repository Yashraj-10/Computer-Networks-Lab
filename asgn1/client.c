/* THE CLIENT PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//Function to dynamically input the string
void GetStr(char *d, char **f)
{
    printf("%s", d);

    for (int i = 0; 1; i++)
    {
        if (i)
            *f = (char *)realloc((*f), i + 1);
        else
            *f = (char *)malloc(i + 1);
        (*f)[i] = getchar();
        if ((*f)[i] == '\n')
        {
            (*f)[i] = '\0';
            break;
        }
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    int i;        // Loop var
    char buf[20]; // Buffer for recieving data from server

    while (1)
    {
        // Opening a socket and exiting if failed
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Unable to create socket\n");
            exit(0);
        }

        // Specifying the server address
        serv_addr.sin_family = AF_INET;
        inet_aton("127.0.0.1", &serv_addr.sin_addr);    
        serv_addr.sin_port = htons(20000);

        // Trying to connect with the server and exiting if failed
        if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
        {
            perror("Unable to connect to server\n");
            exit(0);
        }
        char *s = NULL;
        GetStr("Enter the expression: ", &s);

        if (s[0] == '-' && s[1] == '1' && s[2] == '\0')
        {
            printf("Terminating the connection\n");
            break;
        }

        char buf[100000];
        send(sockfd, s, strlen(s) + 1, 0); // Sending the expression to the server
        recv(sockfd, buf, 100000, 0);      // Recieving the result from the server
        printf("Result: %s\n", buf);

        free(s);
    }

    close(sockfd); // Closing the socket
    return 0;
}
