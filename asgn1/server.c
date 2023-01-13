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

// Funtion to remove spaces from the recieved expression
void removeSpaces(char *str)
{
    int count = 0;
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];
    str[count] = '\0';
}

// FUnction to evaluate the expression after removing spaces
float evaluate(char *s)
{
    removeSpaces(s);
    // Evaluating the expression
    int i = 0;
    float ans = 0;      //variable to store ans
    char op = '+';
    while (s[i] != '\0')
    {
        if (s[i] == '+')
        {
            op = s[i];
            i++;
        }
        else if (s[i] == '-')
        {
            op = s[i];
            i++;
        }
        else if (s[i] == '/')
        {
            op = s[i];
            i++;
        }
        else if (s[i] == '*')
        {
            op = s[i];
            i++;
        }
        else if (s[i] == '(')               ///handling brackets
        {
            char *substr = NULL;
            i++;
            int j;
            for (j = 0; 1; j++, i++)
            {
                if (j)
                    substr = (char *)realloc(substr, j + 1);
                else
                    substr = (char *)malloc(j + 1);
                substr[j] = s[i];
                if(substr[j]==')')
                {
                    substr[j]='\0';
                    break;
                }
            }
            float bracketnum = evaluate(substr);            // Calculating the expression inside bracket
            if (op == '+')
                ans += bracketnum;
            else if (op == '-')
                ans -= bracketnum;
            else if (op == '/')
                ans /= bracketnum;
            else if (op == '*')
                ans *= bracketnum;

            i++;
        }
        else if (s[i] >= '0' && s[i] <= '9')
        {
            float num = 0;
            char buff[100];
            for (int i = 0; i < 100; i++)
            {
                buff[i] = '\0';
            }
            int x = 0;
            while ((s[i] >= '0' && s[i] <= '9') || s[i] == '.')
            {
                buff[x] = s[i];
                x++;
                i++;
            }
            num = atof(buff);
            if (op == '+')
                ans += num;
            else if (op == '-')
                ans -= num;
            else if (op == '/')
                ans /= num;
            else if (op == '*')
                ans *= num;
        }
        else            // case for invalid expression
        {
            printf("Invalid Expression\n");
            return 0;
        }
    }
}

int main()
{
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
        // printf("loop\n");
        clilen = sizeof(cli_addr);                                         // Specifying the size of the client address
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // Accepting a connection

        // Exiting if accept failed
        if (newsockfd < 0)
        {
            perror("Accept error\n");
            exit(0);
        }

        // time_t t;           													// Variable to store the time using a non primitive data type
        // time(&t);           													// Storing the time in t
        // char timeNdate[20]; 													// Variable to store the time and date in a string format
        // strftime(timeNdate, 20, "%H:%M:%S %d/%m/%Y", localtime(&t));    		// Storing the time and date in a string format

        // send(newsockfd, timeNdate, strlen(timeNdate) + 1, 0);   				// Sending the time and date to the client

        // printf("client found");

        char buf[1000000];
        recv(newsockfd, buf, 1000000, 0);       // Recieving the expression
        printf("buf = %s\n", buf);

        float n = evaluate(buf);                //Evaluating the expression
        char ans[100];
        for (int i = 0; i < 100; i++)
        {
            ans[i] = '\0';
        }
        sprintf(ans, "%f", n);                  // Converting the float result to string
        send(newsockfd, ans, 1000, 0);          // Sending the result of expression
        close(newsockfd); // Closing the socket
    }
    return 0;
}
