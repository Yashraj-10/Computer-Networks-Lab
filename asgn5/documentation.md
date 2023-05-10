## Message table structure (Circular queue)
struct message_table
{
    int rear, front; //consumer will read data from the rear, and producer will write data to the front
    char **message_list; //dynamic message list
    int message_size[10]; //size of individual message
};
struct message_table *Send_Message, *Received_Message; //table for send and received messages respectively

## Functions

void *handle_receive(void *arg) -> this function wait on recv. If data is received, it properly converts it into a complete message and writes it to the receive table
void *handle_send(void *arg) -> this function sees if there is a message in send message table. If found, send the message using send call
int my_socket(int domain, int type, int protocol) -> create a socket and spawn the send and receive threads
int my_bind -> bind the socket to the given address
int my_listen -> listen on the socket for incoming connection
int my_accept -> accept the incoming connection
int my_connect -> connect to the given address
ssize_t my_recv -> read the message from receive table, or if the receive table is empty, then block
ssize_t my_send -> write the message to send table, or if the send table is full, then block
void my_close -> wait for 5 seconds(due to race between receive or send thread, might not be handled but sir recommended in mail due to shortage of time) close the socket 
Furthermore, clean up the tables. It also kills the thread.

## Mutual exclusion of threads
We have designed our data structure so there will not be race conditions.
In the receive table receive thread is writing data, and the main thread is reading data. They are not 
modifying any common variable. The receive thread first writes a message to the reception table
and then increments the front. Whereas the main thread is waiting till the receive thread has not finished.
so there will not be any race condition
The same goes for the send_table

### What happens when my_send is called
If the send table is full, then it becomes blocking, and wait till it is empty
without loss of generality; let us assume that send table does not contain any entry(if it does, thread S will send it eventually)
my_send adds the message, its size, and increment front to the send table.
So now we have one message in send table.
As send thread(S) is checking if there is a message in the send table; when it finds the message,
it reads it and increments the rear pointer.
it inserts the message length (Fixed size 4 bytes) at the beginning and sends this message to the client
after that it starts checking for new messages

### What happens when my_recv is called
my_recv checks if there is a message in the receive table? If there is, it reads it and returns its length.
If there is, no message my_recv waits till a  message has been added.
Meanwhile, thread R is waiting on recv() if there is data to be received.
it first receives first 4 bytes of data(looping is done till a total of 4 bytes are received)
then it converts it to an integer, and that is the message length,
then it receives the rest of the messages. Ensured by looping till the total of message_length is received