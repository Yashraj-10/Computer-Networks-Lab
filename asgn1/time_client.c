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
	serv_addr.sin_port	= htons(20000);

	// Trying to connect with the server and exiting if failed
    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

    for(i=0; i < 20; i++) buf[i] = '\0';

	recv(sockfd, buf, 20, 0);		// Recieving data from server
	printf("%s\n", buf);			// Printing the recieved data

	close(sockfd);					// Closing the socket
	return 0;
}

