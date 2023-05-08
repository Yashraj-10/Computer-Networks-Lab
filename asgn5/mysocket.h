#include <stddef.h>
#include <unistd.h>

#define SOCK_MyTCP 0
#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

struct message_table
{
    int rear, front;
    char **message_list;
    int message_size[10];
};
struct message_table *Send_Message, *Received_Message;

void *handle_receive(void *);
void *handle_send(void *);
int my_socket(int , int, int);  
int my_bind(int, const struct sockaddr *, socklen_t);
int my_listen(int,int);
int my_accept(int, struct sockaddr *, socklen_t *);
int my_connect(int, const struct sockaddr *, socklen_t);
ssize_t my_recv(int, void *, size_t , int);
ssize_t my_send(int, void *, ssize_t ,int);
void my_close(int);