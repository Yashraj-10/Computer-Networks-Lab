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

// Function to merge the packets recieved from the server
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
                buf[i+index]='\0';
                halt = 1;
                break;
            }
            buf[i+index] = temp[i];
                
        }
        index += x;
    }
}

int main()
{
    int sockfd ;
	struct sockaddr_in serv_addr;

	int i;						// Loop var
	char buf[100];				// Buffer for recieving data from server

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
	inet_aton("192.168.50.177", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	// Trying to connect with the server and exiting if failed
    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

    recv(sockfd, buf, 50, 0);		// Recieving the message from the server
    printf("%s ", buf);				// Printing the message "LOGIN:"

    char username[25];				// Buffer for storing the username
    for(i=0; i<25; i++)				// Initializing the username buffer
        username[i] = '\0';

    scanf("%s", username);			// Scanning the username from the user
    getchar();                        // To remove the newline character from the input buffer
    send(sockfd, username, strlen(username)+1, 0);	// Sending the username to the server

    recv(sockfd, buf, 50, 0);		// Recieving the message from the server

    if(buf[0]=='F')
    {
        // printf("%s", buf);			// Printing the message "FOUND"
        while(1)
        {
            // printf("Enter command: ");
            char *command;		// Buffer for storing the command
            GetStr("-> ", &command);	// Scanning the command from the user

            if(command[0]=='e' && command[1]=='x' && command[2]=='i' && command[3]=='t' && command[4]=='\0')
            {
                printf("Bye!!!\n");
                close(sockfd);
                return 0;
            }

            send(sockfd, command, strlen(command)+1, 0);	// Sending the command to the server

            message_packet_merging(sockfd, buf);		                // Recieving the message from the server

            if(buf[0]=='$')
            {
                printf("%s\n", invalidCommand);			    // Printing the message "Invalid command"
            }
            else if(buf[0]=='#')
            {
                printf("%s\n", errorInRunningCommand);	    // Printing the message "Error in running command"
            }
            else
            {
                printf("%s\n", buf);			                // Printing the message recieved from the server
            }
            
        }
    }
    else
    {
        printf("%s\n", invalidUsername);			            // Printing the message "Invalid username"
    }

	close(sockfd);					                        // Closing the socket
	return 0;
}

