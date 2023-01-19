/* THE CLIENT PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int sockfd ;
	struct sockaddr_in serv_addr;

	int i;						// Loop var
	char buf[50];				// Buffer for recieving data from server

    char *invalidUsername = "Invalid username";
    char *invalidCommand = "Invalid command";
    char *errorInRunningCommand = "Error in running command";

	// Opening a socket and exiting if failed
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Unable to create socket\n");
		exit(0);
	}

	// Specifying the server address
    serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	// Trying to connect with the server and exiting if failed
    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

    recv(sockfd, buf, 50, 0);		// Recieving the message from the server
    printf("%s", buf);				// Printing the message "LOGIN:"

    char username[25];				// Buffer for storing the username
    for(i=0; i<25; i++)				// Initializing the username buffer
        username[i] = '\0';

    scanf("%s", username);			// Scanning the username from the user
    send(sockfd, username, strlen(username)+1, 0);	// Sending the username to the server

    recv(sockfd, buf, 50, 0);		// Recieving the message from the server

    if(buf[0]=='F')
    {
        printf("%s", buf);			// Printing the message "FOUND"
        while(1)
        {
            // printf("Enter command: ");
            char command[50];		// Buffer for storing the command
            for(i=0; i<50; i++)		// Initializing the command buffer
                command[i] = '\0';

            scanf("%s", command);	                        // Scanning the command from the user

            send(sockfd, command, strlen(command)+1, 0);	// Sending the command to the server

            recv(sockfd, buf, 50, 0);		                // Recieving the message from the server

            if(buf[0]=='$')
            {
                printf("%s", invalidCommand);			    // Printing the message "Invalid command"
            }
            else if(buf[0]=='#')
            {
                printf("%s", errorInRunningCommand);	    // Printing the message "Error in running command"
            }
            else
            {
                printf("%s", buf);			                // Printing the message recieved from the server
            }
            
        }
    }
    else
    {
        printf("%s", invalidUsername);			            // Printing the message "Invalid username"
    }

	close(sockfd);					                        // Closing the socket
	return 0;
}

