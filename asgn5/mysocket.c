#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "mysocket.h"


int recv_sock_fd = -1, send_sock_fd = -1;
pthread_t R, S;
pthread_attr_t attr;

// Thread R
void *handle_receive(void *arg)
{
    char buf[5004];
    
    while (1)
    {
        if (recv_sock_fd == -1) // if the recv_sock_fd is not initialised then wait
            continue;

        int recv_len = 0, total_received = 0;

        // receive the length of message first(FIRST FOUR BYTES)
        while (recv_len < 4)
        {
            char temp_buff[4];
            recv_len = recv(recv_sock_fd, temp_buff, 4, 0);
            if (recv_len <= 0)
                continue;
            for (int i = 0; i < recv_len; i++)
                buf[i + total_received] = temp_buff[i];
            total_received += recv_len;
        }

        char len[5] = "\0\0\0\0\0";

        for (int i = 0; i < 4; i++)
            len[i] = buf[i];

        int message_len = atoi(len);

        if (DEBUG_TEST)
            printf("Going to receive a message of length: %d\n", message_len);

        // receives the rest of message
        while (total_received < message_len + 4)
        {
            char temp_buff[message_len];
            recv_len = recv(recv_sock_fd, temp_buff, message_len, 0);

            for (int i = 0; i < recv_len; i++)
                buf[i + total_received] = temp_buff[i];
                
            total_received += recv_len;
        }

        // if the received message table is full wait for a free space
        while ((Received_Message->front + 1) % 10 == Received_Message->rear)
            continue;

        for (int i = 4; i < total_received; i++)
            Received_Message->message_list[Received_Message->front][i - 4] = buf[i];

        Received_Message->message_size[Received_Message->front] = message_len;
        Received_Message->front = (Received_Message->front + 1) % 10;

        if (DEBUG_TEST)
        {

            printf("Message have been successfully added to the receive table, Following are the details: \n");
            printf("Message-length: %d, Message: %s", Received_Message->message_size[Received_Message->front], Received_Message->message_list[Received_Message->front]);
        }
    }
}

void *handle_send(void *arg)
{
    while (1)
    {
        if (send_sock_fd == -1) // wait till send_sock_fd is initialised
            continue;

        // if send_message queue is empty then sleep and wait
        if (Send_Message->rear == Send_Message->front)
        {
            // sleep(5);
            continue;
        }

        if (DEBUG_TEST)
        {
            printf("FOUND A MESSAGE TO BE SENT IN Send_Message table\n");
            printf("DETAILS\n message_len: %d message: %s", Send_Message->message_size[Send_Message->rear], Send_Message->message_list[Send_Message->rear]);
        }

        char buff[1000];
        char temp[5], len[5] = "0000\0";
        snprintf(temp, 5, "%d", Send_Message->message_size[Send_Message->rear]);
        int start_idx = 4 - strlen(temp);

        for (int i = 0; i < strlen(temp); i++)
            len[start_idx++] = temp[i];

        send(send_sock_fd, len, 4, 0);
        int j = 0;

        for (int i = 0; i < Send_Message->message_size[Send_Message->rear]; i++)
        {
            // printf("%d\n", i);
            if (i % 1000 == 0 && i >= 1000)
            {
                send(send_sock_fd, buff, 1000, 0);
                j = 0;
            }
            buff[j++] = Send_Message->message_list[Send_Message->rear][i];
        }

        send(send_sock_fd, buff, j, 0);
        Send_Message->rear = (Send_Message->rear + 1) % 10;

        if (DEBUG_TEST)
            printf("SUCCESSFULLY SENT MESSAGE\n");
    }
}

int my_socket(int domain, int type, int protocol)
{
    // if type is is not zero throw an error and return
    if (type != 0)
    {
        errno = EINVAL;
        return -1;
    }

    // create the socket
    int sock_fd;
    if ((sock_fd = socket(domain, SOCK_STREAM, 0)) < 0)
        return -1;

    // initialise and allocate tables
    Send_Message = (struct message_table *)malloc(sizeof(struct message_table));
    Received_Message = (struct message_table *)malloc(sizeof(struct message_table));

    if (Send_Message == NULL || Received_Message == NULL)
        return -1;

    Send_Message->message_list = (char **)malloc(10 * sizeof(char *));
    Received_Message->message_list = (char **)malloc(10 * sizeof(char *));

    if (Send_Message->message_list == NULL || Received_Message->message_list == NULL)
        return -1;

    for (int i = 0; i < 10; i++)
    {
        Send_Message->message_list[i] = (char *)malloc(5000 * sizeof(char));
        Received_Message->message_list[i] = (char *)malloc(5000 * sizeof(char));

        if (Send_Message->message_list[i] == NULL || Received_Message->message_list[i] == NULL)
            return -1;
    }

    Received_Message->front = 0;
    Received_Message->rear = 0;
    Send_Message->rear = 0;
    Send_Message->front = 0;

    // initialise and create the threads
    pthread_attr_init(&attr);
    pthread_create(&R, &attr, handle_receive, NULL);
    pthread_create(&S, &attr, handle_send, NULL);
    return sock_fd;
}

int my_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    return bind(fd, addr, len);
}

int my_listen(int fd, int n)
{
    return listen(fd, n);
}

int my_accept(int fd, struct sockaddr *addr, socklen_t *len)
{
    return accept(fd, addr, len);
}

int my_connect(int fd, const struct sockaddr *addr, socklen_t len)
{
    return connect(fd, addr, len);
}

ssize_t my_recv(int fd, void *buf, size_t n, int flags)
{
    recv_sock_fd = fd;
    while (Received_Message->front == Received_Message->rear)
        continue;

    ssize_t len = 0;

    char *buffer = (char *)malloc(n * sizeof(char));
    if (!buffer)
        return -1;

    for (int i = 0; i < n; i++)
    {
        if (i >= Received_Message->message_size[Received_Message->rear])
            break;

        buffer[i] = Received_Message->message_list[Received_Message->rear][i];
        *(char *)(buf + i) = buffer[i];
        len++;
    }

    if (DEBUG_TEST)
        printf("SUCCESSFULLY READ A MESSAGE OF LENGTH: %ld", len);

    Received_Message->rear = (Received_Message->rear + 1) % 10;

    return len;
}

ssize_t my_send(int fd, void *buf, ssize_t n, int flags)
{
    send_sock_fd = fd;

    while ((Send_Message->front + 1) % 10 == Send_Message->rear)
        continue;
        
    char *buff = buf;

    for (int i = 0; i < n; i++)
        Send_Message->message_list[Send_Message->front][i] = buff[i];
        
    Send_Message->message_size[Send_Message->front] = n;
    Send_Message->front = (Send_Message->front + 1) % 10;

    if (DEBUG_TEST)
        printf("MESSAGE of length: %ld ADDED TO SEND_MESSAGE TABLE\n", n);

    return n;
}

void my_close(int sockfd)
{
    // close the connection
    sleep(5);
    close(sockfd);
    // destroy the tables

    for (int i = 0; i < 10; i++)
    {
        free(Send_Message->message_list[i]);
        free(Received_Message->message_list[i]);
    }

    free(Send_Message->message_list);
    free(Received_Message->message_list);
    // kill the threads

    pthread_attr_destroy(&attr);
    pthread_cancel(R);
    pthread_cancel(S);
}