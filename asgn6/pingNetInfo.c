#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <ifaddrs.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#define MAX_HOP 30
const int print_data = 0;

// calculates checksum
u_int16_t check_sum(u_int16_t *buff, int nwords)
{
    u_int64_t sum = 0;

    while (nwords-- > 0)
        sum += *buff++;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

// return ip from a given hostname
char *get_ip(char *hostname)
{
    struct hostent *hptr = gethostbyname(hostname);
    if (!hptr)
    {
        printf("Unable to get host name for %s,check the  try again!\n", hostname);
        exit(0);
    }

    char str[INET_ADDRSTRLEN];
    char **currptr = hptr->h_addr_list;

    return inet_ntop(hptr->h_addrtype, *currptr, str, sizeof(str));
}

// initialize the IP header
void initialize_ip_header(int ttl, char *dest, struct ip **ip_hdr)
{
    (*ip_hdr)->ip_hl = 5;
    (*ip_hdr)->ip_v = 4;
    (*ip_hdr)->ip_tos = 0;
    (*ip_hdr)->ip_len = 20 + 8;
    (*ip_hdr)->ip_id = 10000;
    (*ip_hdr)->ip_off = 0;
    (*ip_hdr)->ip_ttl = ttl;
    (*ip_hdr)->ip_p = IPPROTO_ICMP;

    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[INET_ADDRSTRLEN];
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // getting the host address
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
           
        }
    }
    freeifaddrs(ifaddr);
    inet_pton(AF_INET, host, &((*ip_hdr)->ip_src)); // source IP address
    // printf("host: %s\n", host);
    inet_pton(AF_INET, dest, &((*ip_hdr)->ip_dst)); // set ip_dest
}

// initializing ICMP header
void init_icmp_header(struct icmphdr **icmphd)
{
    (*icmphd)->type = 8;
    (*icmphd)->code = 0;
    (*icmphd)->checksum = 0;
    (*icmphd)->un.echo.id = 0;
}
// print ICMP package

void print_icmp_packet(char *buffer, int size, int flag)
{
    struct iphdr *ip_header = (struct iphdr *)buffer;
    int ip_header_size = ip_header->ihl * 4;
    struct icmphdr *icmp_header = (struct icmphdr *)(buffer + ip_header_size);
    int icmp_header_size = sizeof(struct icmphdr);
    // char *icmp_data = (buffer + ip_header_size + icmp_header_size);
    char *data = (buffer + ip_header_size + icmp_header_size);
    flag == 0 ? printf("Received ") : printf("Sent ");
    printf(" ICMP Packet:\n");
    printf("    |-Type          : %d\n", (unsigned int)(icmp_header->type));
    printf("    |-Code          : %d\n", (unsigned int)(icmp_header->code));
    printf("    |-Checksum      : %d\n", ntohs(icmp_header->checksum));
    printf("    |-Identifier    : %d\n", ntohs(icmp_header->un.echo.id));
    printf("    |-Sequence Number: %d\n", ntohs(icmp_header->un.echo.sequence));
    if (print_data)
    {
        // print data in hex
        printf("    |-Data:\n");
        int byte_count = 0;
        while (byte_count < (size - ip_header_size - icmp_header_size))
        {
            printf("    | ");
            for (int i = 0; i < 8 && byte_count < (size - ip_header_size - icmp_header_size); i++)
            {
                printf("%02X ", (unsigned int)(data[byte_count]));
                byte_count++;
            }
            printf("\n");
        }
    }
}
void PingNetInfo(char *dest_ip, int n, int T)
{

    // create a socket
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // set socket option
    int one = 1;
    int *val = &one;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("Unable to set HDRINCL!\n");

    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(7);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip, &(server_addr.sin_addr));

    // initializing to 0
    char *buf = (char *)calloc(4094, sizeof(char));
    struct ip *ip_header = (struct ip *)buf;
    initialize_ip_header(0, dest_ip, &ip_header);

    int curr_ttl = 1;
    int destn_reached = 0;
    double prev_latency = 0, prev_rrt1 = 0, prev_rrt2 = 0;
    while (1)
    {
        printf("--------------------------------------------------------\n");

        ip_header->ip_ttl = curr_ttl++;
        ip_header->ip_sum = check_sum((unsigned short *)buf, 9);
        struct icmphdr *icmphd = (struct icmphdr *)(buf + 20);
        init_icmp_header(&icmphd);
        icmphd->un.echo.sequence = curr_ttl;
        icmphd->checksum = check_sum((unsigned short *)(buf + 20), 4);

        // sending the ICMP packet 5 times to locate the node
        // then sending the packets to the finalized nodes to calculate the latency and bandwidth
        char IPofNode[40];
        char PrevNode[40];
        int icmp_type;
        int successful_probe = 5;
        struct sockaddr_in addr2;
        socklen_t len = sizeof(struct sockaddr_in);
        struct pollfd fds;
        fds.events = POLLIN;
        fds.fd = sockfd;
        for (int i = 0; i < 5; i++)
        {
            sendto(sockfd, buf, sizeof(struct ip) + sizeof(struct icmphdr), 0, (struct sockaddr *)&server_addr, sizeof server_addr);
            print_icmp_packet(buf, sizeof(buf), 1);

            char buff[4096] = {0};

            // receiving the ICMP packet
            printf("Probe%d\n", i + 1);

            if (poll(&fds, 1, 100) <= 0)
            {
                successful_probe--;
            }
            else
            {
                recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&addr2, &len);
                print_icmp_packet(buff, sizeof(buff), 0);
                // segregatine the ICMP part from the packet
                struct icmphdr *icmphd2 = (struct icmphdr *)(buff + 20);
                // checking the response
                icmp_type = icmphd2->type;
                // Assuming we have same node everytime
                strcpy(IPofNode, inet_ntoa(addr2.sin_addr));
                printf("IP:%s\n", IPofNode);
            }

            sleep(1);
        }
        if (successful_probe) // if any of the 5 probe is succesful
        {

            printf("Decided Node IP : %s  Node Number : %d\n", IPofNode, curr_ttl - 1);
            // estimate latency and  bandwidth
            struct sockaddr_in server_addr2;
            server_addr2.sin_port = htons(7);
            server_addr2.sin_family = AF_INET;
            inet_pton(AF_INET, IPofNode, &(server_addr2.sin_addr));
            double latency = 0.0, bandwidth = 0.0;

            char *buff2 = (char *)calloc(4094, sizeof(char));

            struct ip *ip_header2 = (struct ip *)buff2;
            initialize_ip_header(60, IPofNode, &ip_header2);
            ip_header2->ip_sum = check_sum((unsigned short *)buff2, 9);

            struct icmphdr *icmp_header = (struct icmphdr *)(buff2 + 20);
            init_icmp_header(&icmp_header);
            icmp_header->un.echo.sequence = 0;
            icmp_header->checksum = check_sum((unsigned short *)(buff2 + 20), 4);
            double rrt2 = 0, rrt1 = 0;

            for (int i = 0; i < n; i++)
            {
                // in each probe send two different data size. one with 0 to calculate latency another with 4094-ipheaader-icmpheader
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                    {
                        struct timespec start_time, end_time;
                        int64_t elapsed_ns;

                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);

                        sendto(sockfd, buff2, sizeof(struct ip) + sizeof(struct icmphdr) + 500, 0, (struct sockaddr *)&server_addr2, sizeof server_addr2);
                        print_icmp_packet(buff2, sizeof(buff2), 1);

                        struct sockaddr_in recv_addr;
                        char recv_buff[4096] = {0};
                        if (poll(&fds, 1, 100) <= 0) /// handle if no reply
                        {
                            continue;
                        }
                        recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&recv_addr, &len);
                        print_icmp_packet(recv_buff, sizeof(recv_buff), 0);

                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
                        elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
                        rrt2 += (double)elapsed_ns / 1000000.0;
                    }
                    else
                    {
                        struct timespec start_time, end_time;
                        int64_t elapsed_ns;

                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
                        sendto(sockfd, buff2, sizeof(struct ip) + sizeof(struct icmphdr), 0, (struct sockaddr *)&server_addr2, sizeof server_addr2);
                        print_icmp_packet(buff2, sizeof(buff2), 1);

                        struct sockaddr_in recv_addr;
                        char recv_buff[4096] = {0};
                        if (poll(&fds, 1, 100) <= 0) // handle if no reply
                        {
                            continue;
                        }
                        recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&recv_addr, &len);
                        print_icmp_packet(recv_buff, sizeof(recv_buff), 0);
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
                        elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
                        rrt1 += (double)elapsed_ns / 1000000.0;
                    }
                }

                // initialize_ip_header(60,)

                sleep(T); // sleep for T second
            }
            printf("rrt1: %f rrt2: %f\n", rrt1, rrt2);
            bandwidth = (2.0 * (500)) / (rrt2 - prev_rrt2 - rrt1 + prev_rrt1);
            latency = rrt1 / n;
            printf("Latency: %f ms\n", (latency - prev_latency));
            printf("Bandwidth: %f Bs\n", (bandwidth));
            prev_latency = latency;
            prev_rrt1 = rrt1;
            prev_rrt2 = rrt2;

            if (icmp_type == 0)
            {
                {
                    printf("Reached destination : %s with number of hops : %d\n", IPofNode, curr_ttl - 1);
                    destn_reached = 1;
                    break;
                }
            }

            // we have the IP of the node, now we calculate the latency and bandwidth of the Link
            // calculate_latency_and_bandwidth(IPofNode, PrevNode);
            if (destn_reached || curr_ttl >= MAX_HOP)
                break;

            strcpy(PrevNode, IPofNode);
        }
        else
        {
            printf("* * * *\n");
        }
        printf("--------------------------------------------------------\n");
    }
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: ./pingNetInfo site num_probes time_diff_btw_probes");
        exit(0);
    }
    char *dest_ip = get_ip(argv[1]);
    strcpy(argv[1], dest_ip);
    printf("Destination IP Address is %s\n ", dest_ip);
    PingNetInfo(argv[1], atoi(argv[2]), atoi(argv[3]));
    printf("Done.....\n");

    return 0;
}