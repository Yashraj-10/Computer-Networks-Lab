/* THE CLIENT PROCESS*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void message_packet_merging(int sockfd, char *buf){

    int index = 0;
    char temp[50];
    int halt = 0;
    
    while(!halt){

        int x = recv(sockfd, temp, 10, 0);
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

int main(int argc, char *argv[])
{
    if(argc<=1)
    {
        perror("No port number provided\n");
        exit(0);
    }
    
    int sockfd ;
	struct sockaddr_in serv_addr;

	int i;						// Loop var
	char buf[20];				// Buffer for recieving data from server

	// Opening a socket and exiting if failed
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Unable to create socket\n");
		exit(0);
	}

	// Specifying the server address
    serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(atoi(argv[1]));

	// Trying to connect with the server and exiting if failed
    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

    for(i=0; i < 20; i++) buf[i] = '\0';

    message_packet_merging(sockfd, buf);

	// recv(sockfd, buf, 20, 0);		// Recieving data from server
	printf("%s\n", buf);			// Printing the recieved data

	close(sockfd);					// Closing the socket
	return 0;
}

