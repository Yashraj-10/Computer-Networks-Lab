/*
Netwroks Lab Asgn-4

Group Members:
Yashraj Singh               20CS10079
Vikas Vijaykumar Bastewad   20CS10073
*/

// Server Implementation
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

// parse the http request
typedef enum Method
{
    GET,
    PUT,
    UNSUPPORTED
} Method;

typedef struct Header
{
    char *name;
    char *values;                   // if there are multiple values, they are separated by a comma
    struct Header *next;

} Header;

typedef struct Request
{
    Method request_method;
    char *ip;                       // client ip
    char *url;                      // request url

    char *HTTP_version;             // HTTP version
    struct Header *headers;         // request headers
    char *enttity_body;             // request body
} Request;

typedef struct Response
{
    char *HTTP_version;
    int status_code;
    char *status_message;
    struct Header *headers;
    char *entity_body;
} Response;

void free_response(struct Response *request)
{
    if (request)
    {
        if (request->headers)
            free_header_request(request->headers);
        if (request->status_message)
            free(request->status_message);
        if (request->HTTP_version)
            free(request->HTTP_version);
        free(request);
    }
}

struct Request *parse(const char *request)
{
    if (strlen(request) < 3)
        return NULL; // request is too short

    struct Request *requestStruct = malloc(sizeof(struct Request));

    if (!requestStruct)
        return NULL; // if malloc fails, return NULL

    memset(requestStruct, 0, sizeof(struct Request)); // initialize the request to 0
    size_t method_len = strcspn(request, " ");

    if (strncmp(request, "GET", method_len) == 0)
        requestStruct->request_method = GET;
    else if (strncmp(request, "PUT", method_len) == 0)
        requestStruct->request_method = PUT;
    else
        requestStruct->request_method = UNSUPPORTED;

    // parse url, and version
    request += method_len + 1; // move past method
    size_t url_len = strcspn(request, " ");
    requestStruct->url = malloc(url_len + 1);
    memcpy(requestStruct->url, request, url_len);

    requestStruct->url[url_len] = '\0';
    request += url_len + 1; // move past url
    size_t version_len = strcspn(request, "\r");
    requestStruct->HTTP_version = malloc(version_len + 1);
    memcpy(requestStruct->HTTP_version, request, version_len);
    requestStruct->HTTP_version[version_len] = '\0';
    request += version_len + 2; // move past version

    // Parsing the headers
    struct Header *headerStruct = NULL, *lastHeader = NULL;

    while (request[0] != '\r' || request[1] != '\n')
    {
        lastHeader = headerStruct;
        headerStruct = malloc(sizeof(struct Header));

        if (!headerStruct)
        {
            free_request(requestStruct);
            return NULL; // if malloc fails, return NULL
        }

        size_t header_len = strcspn(request, ":");
        headerStruct->name = malloc(header_len + 1);

        if (!headerStruct->name)
        {
            free_request(requestStruct);
            return NULL;
        }

        memcpy(headerStruct->name, request, header_len); // copy the header name
        headerStruct->name[header_len] = '\0';
        request += header_len + 1; // move past header name and ": "

        size_t value_len = strcspn(request, "\r");
        headerStruct->values = malloc(value_len + 1);

        if (!headerStruct->values)
        {
            free_request(requestStruct);
            return NULL;
        }

        memcpy(headerStruct->values, request, value_len); // copy the header value
        headerStruct->values[value_len] = '\0';
        request += value_len + 2; // move past header value and "\r\n"
        headerStruct->next = lastHeader;
    }

    requestStruct->headers = headerStruct;
    // handle blank line before entity body
    size_t len = strcspn(request, "\r");
    if (request[len + 2] == '\r')
        len += 2;
    request += len + 2; // move past "\r\n"

    // parse entity body
    len = strlen(request);
    requestStruct->enttity_body = malloc(len + 1);

    if (!requestStruct->enttity_body)
    {
        free_request(requestStruct);
        return NULL;
    }

    memcpy(requestStruct->enttity_body, request, len);
    requestStruct->enttity_body[len] = '\0';

    return requestStruct;
}

int getmonth(char *s)
{
    if (!strcmp("Jan", s))
        return 0;
    if (!strcmp("Feb", s))
        return 1;
    if (!strcmp("Mar", s))
        return 2;
    if (!strcmp("Apr", s))
        return 3;
    if (!strcmp("May", s))
        return 4;
    if (!strcmp("Jun", s))
        return 5;
    if (!strcmp("Jul", s))
        return 6;
    if (!strcmp("Aug", s))
        return 7;
    if (!strcmp("Sep", s))
        return 8;
    if (!strcmp("Oct", s))
        return 9;
    if (!strcmp("Nov", s))
        return 10;
    if (!strcmp("Dec", s))
        return 11;
    else
        return -1;
}

void free_header_request(struct Header *header)
{
    if (header)
    {
        free_header_request(header->next);
        if (header->name)
            free(header->name);
        if (header->values)
            free(header->values);
        free(header);
    }
}

void free_request(struct Request *request)
{
    if (request)
    {
        if (request->headers)
            free_header_request(request->headers);
        if (request->url)
            free(request->url);
        if (request->HTTP_version)
            free(request->HTTP_version);
        if (request->enttity_body)
            free(request->enttity_body);
        free(request);
    }
}

char **tokenize_command(char *cmd)
{
    int index = 0;
    char temp[5000];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(1000 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int space = 0;

    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')
            continue;
        flag = 1;

        cnt = 0;
        if (space == 1 && cmd[i] == ' ')
            continue;
        else if (cmd[i] == ' ')
        {
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i - 1] != '\\'))
        {
            if (cmd[i] == '\0')
                break;
            if (cmd[i] == '\\')
            {
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            // added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        char **tmp = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        if (tmp == NULL)
        {
            printf("realloc failed:367\n");
            exit(1);
        }
        cmdarr = tmp;
        cmdarr[index] = (char *)malloc(1000 * sizeof(char));

        if (cmd[i] == '\0')
            break;
    }

    cmdarr[index] = NULL;

    return cmdarr;
}

int compare_date(char *last_modified, char *if_modified_since)
{
    // returns 1 if the last_modified is before the if_modified_since
    char **date1 = tokenize_command(last_modified);
    char **date2 = tokenize_command(if_modified_since);

    int year1 = atoi(date1[3]);
    int year2 = atoi(date2[3]);
    if (year1 < year2)
        return 1;
    if (year1 > year2)
        return 0;

    int month1 = getmonth(date1[2]);
    int month2 = getmonth(date2[2]);
    if (month1 < month2)
        return 1;
    if (month1 > month2)
        return 0;

    int day1 = atoi(date1[1]);
    int day2 = atoi(date2[1]);
    if (day1 < day2)
        return 1;
    if (day1 > day2)
        return 0;

    return 0;
}

char *modifydate(int changeday, struct tm tm)
{
    tm.tm_mday += changeday;
    mktime(&tm);
    char buf[50];
    strcpy(buf, asctime(&tm));

    buf[strlen(buf) - 1] = '\0';

    char **temp = tokenize_command(buf);

    // Now HTTP formatting
    char *final = (char *)malloc(100 * sizeof(char));
    strcpy(final, temp[0]);
    strcat(final, ", ");
    strcat(final, temp[2]);
    strcat(final, " ");
    strcat(final, temp[1]);
    strcat(final, " ");
    strcat(final, temp[4]);
    strcat(final, " ");
    strcat(final, temp[3]);
    strcat(final, " ");
    strcat(final, "IST");

    return final;
}

void print_request(struct Request *request)
{
    if (request)
    {
        printf("Method: %d\n", request->request_method);
        printf("Request-URI: %s\n", request->url);
        printf("HTTP-Version: %s\n", request->HTTP_version);
        puts("Headers:");
        struct Header *h;
        for (h = request->headers; h; h = h->next)
        {
            printf("%32s: %s\n", h->name, h->values);
        }
    }
}

// helper function to receive the data in chunks(50 bytes)
char *receiveHeader(int sockfd)
{
    char *recvBuffer = (char *)malloc(51 * sizeof(char));
    char *buff = (char *)malloc(51 * sizeof(char));
    strcpy(buff, "\0");
    char *startingEntityBody;
    int length = 0;

    while (1)
    {
        resetBuffer(recvBuffer, 51);
        int x = recv(sockfd, recvBuffer, 50, 0);
        
        if (x <= 0)
        {
            printf("Error receiving data!\n");
            return;
        }

        strcat(buff, recvBuffer);
        char *endOfHeader = strstr(recvBuffer, "\r\n\r\n");

        if (endOfHeader)
        {
            startingEntityBody = strdup(recvBuffer);
            break;
        }

        char *temp = realloc(buff, strlen(buff) + 51);

        if (temp == NULL)
        {
            printf("Error reallocating memory!\n");
            return;
        }

        buff = temp;
    }
    // printf("Header received succesfully!\n");
    free(recvBuffer);

    return buff;
}

void receive_and_write_to_file(int sockfd, char *url, int contentLength, char *startingMsg)
{
    FILE *fp = fopen(url, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        send_put_response(sockfd, 403);
        return;
    }

    if (strcmp(startingMsg, "") != 0)
        fwrite(startingMsg, 1, strlen(startingMsg), fp);

    char *recvBuffer = (char *)malloc(1025 * sizeof(char));
    int length = 0;

    while (1)
    {
        resetBuffer(recvBuffer, 1025);

        int x = recv(sockfd, recvBuffer, 1024, 0);
        if (x == 0)
        {
            printf("Connection closed!\n");
            break;
        }
        // printf("writing to file: %s\n", recvBuffer);
        fwrite(recvBuffer, 1, x, fp);
        length += x;

        if (length >= contentLength)
            break;
    }

    printf("file downloaded successfully! Size: %d bytes\n", length + strlen(startingMsg));
    fclose(fp);

    free(recvBuffer);
}

// helper function to reset the buffer
void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
        buff[i] = '\0';
}

void send_response_header(int client_sockfd, struct Response *response)
{
    char *responseString = malloc(strlen(response->HTTP_version) + 1);
    strcpy(responseString, response->HTTP_version);
    strcat(responseString, " ");

    char *status_code = malloc(4 * sizeof(char));
    sprintf(status_code, "%d", response->status_code);
    malloc(strlen(status_code) + 1);

    strcat(responseString, status_code);
    strcat(responseString, " ");
    malloc(strlen(response->status_message) + 2);
    strcat(responseString, response->status_message);
    strcat(responseString, "\r\n");

    struct Header *h;

    for (h = response->headers; h; h = h->next)
    {
        malloc(strlen(h->name) + 2);
        strcat(responseString, h->name);
        strcat(responseString, ": ");

        malloc(strlen(h->values) + 2);
        strcat(responseString, h->values);
        strcat(responseString, "\r\n");
    }

    strcat(responseString, "\r\n");
    printf("FOLLOWING RESPONSE STRING HAS BEEN SENT TO SERVER:\n");
    printf("%s\n", responseString);

    send(client_sockfd, responseString, strlen(responseString), 0); // send the header and HTTP version
}

void set_header_and_HTTPversion(int status_code, struct Response *response)
{
    char *status_message = malloc(10 * sizeof(char));
    if (status_code == 200)
        status_message = "OK";
    else if (status_code == 400)
        status_message = "Bad Request";
    else if (status_code == 403)
        status_message = "Forbidden";
    else if (status_code == 404)
        status_message = "Not Found";

    response->HTTP_version = "HTTP/1.1";
    response->status_code = status_code;
    response->status_message = status_message;

    struct Header *header = malloc(sizeof(struct Header));
    response->headers = header;
    header->name = strdup("Expires");

    // header->values = strdup("Thu, 01 Dec 1994 16:00:00 GMT");
    time_t t = time(NULL);
    struct tm tmarst = *localtime(&t);
    header->values = strdup(modifydate(3, tmarst));
    header->next = malloc(sizeof(struct Header));
    header = header->next;

    header->name = strdup("Cache-Control");
    header->values = strdup("no-store always");
    header->next = malloc(sizeof(struct Header));
    header = header->next;

    header->name = strdup("Content-language");
    header->values = strdup("en-us");
    header->next = NULL;
}

void send_put_response(int client_sockfd, int status_code)
{
    char *put_response = malloc(100 * sizeof(char));

    if (status_code == 200)
        strcpy(put_response, "HTTP/1.1 200 OK\r\n");
    else if (status_code == 400)
        strcpy(put_response, "HTTP/1.1 400 Bad Request\r\n");
    else if (status_code == 403)
        strcpy(put_response, "HTTP/1.1 403 Forbidden\r\n");
    else if (status_code == 404)
        strcpy(put_response, "HTTP/1.1 404 Not Found\r\n");

    strcat(put_response, "\r\n");
    send(client_sockfd, put_response, strlen(put_response), 0);
}

void send_response_file(int new_socket, char *url)
{
    // send the file
    FILE *fp = fopen(url, "r");
    char *buffer = malloc(1024 * sizeof(char));
    int n;
    int totalBytes = 0;

    while (1)
    {
        if ((n = fread(buffer, 1, 1024, fp)) <= 0)
        {
            break;
        }
        send(new_socket, buffer, n, 0);
        totalBytes += n;
    }

    printf("\nTotal bytes sent: %d\n", totalBytes);
    fclose(fp);

    printf("File sent successfully!\n");
}

int find_content_length_value(struct Request *request)
{
    struct Header *h;
    for (h = request->headers; h; h = h->next)
    {
        char *len = strdup(h->name); // HTTP RFC says that header names are case-insensitive
        for (int i = 0; len[i]; i++)
        {
            len[i] = tolower(len[i]);
        }
        // printf("len: %s\n", len);
        if (strcmp(len, "content-length") == 0)
        {
            // printf("content-length: %s\n", h->values);
            return atoi(h->values);
        }
    }

    return -1;
}

char *get_if_modified_since(struct Request *request)
{
    struct Header *h;
    for (h = request->headers; h; h = h->next)
    {
        char *len = strdup(h->name); // HTTP RFC says that header names are case-insensitive

        for (int i = 0; len[i]; i++)
            len[i] = tolower(len[i]);
            
        if (strcmp(len, "if-modified-since") == 0)
            return h->values;
    }

    return NULL;
}


int main()
{
    // normal tcp server routine
    // creatte an access log file
    FILE *access_log = fopen("access_log.txt", "a");

    int server_fd, new_socket;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(8084);

    printf("Server address: %s Port: %d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int addrlen;
    while (1)
    {
        printf("Waiting for new connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        char *record = malloc(1000 * sizeof(char));
        // current time and date
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        if (!fork())
        {
            // handle the client here
            close(server_fd);
            char *buff;
            buff = receiveHeader(new_socket);
            struct Request *req = parse(buff);
            free(buff);
            sprintf(record, "%d-%d-%d : %d:%d:%d : %s : %d : %s : %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), req->request_method == 0 ? "GET" : "PUT", req->url);
            fwrite(record, 1, strlen(record), access_log);
            free(record);
            printf("Received request:\n");
            print_request(req);
            
            if (req->request_method == 2) // Unsupported method
                send_put_response(new_socket, 400);
            else
            {
                // check if the file is a directory
                struct stat path_stat;

                if (!stat(req->url, &path_stat) && S_ISDIR(path_stat.st_mode))
                {
                    // file is a directory then return 403

                    send_put_response(new_socket, 403);
                    exit(EXIT_SUCCESS);
                }

                if ((req->request_method == 0))
                {
                    // handle GET request
                    // check if the file exists
                    if (access(req->url, F_OK | R_OK) < 0)
                    {
                        printf("File doesn't exist or can't be read");
                        send_put_response(new_socket, 404);
                        exit(EXIT_SUCCESS);
                    }

                    // file exists and can be read
                    struct Response *response = malloc(sizeof(struct Response));
                    set_header_and_HTTPversion(200, response);
                    struct Header *h = response->headers;
                    struct stat st;
                    stat(req->url, &st);
                    struct tm dt = *(gmtime(&st.st_mtime));
                    char *last_modified = modifydate(0, dt);
                    char *if_modified_since = get_if_modified_since(req);
                    int is_modified = 1;

                    if (!if_modified_since && compare_date(last_modified, if_modified_since))
                        is_modified = 0;
                        
                    char *content_length = malloc(100 * sizeof(char));

                    if (is_modified)
                        sprintf(content_length, "%ld", st.st_size);
                    else
                        content_length = "0";

                    h->next = malloc(sizeof(struct Header));
                    h = h->next;
                    h->name = strdup("Content-Length");
                    h->values = strdup(content_length);
                    h->next = malloc(sizeof(struct Header));
                    h = h->next;
                    h->name = strdup("Last-Modified");

                    h->values = strdup(last_modified); // this is not in GMT and it is not in the correct format so we need to convert it to GMT and then to the correct format
                    h->next = NULL;

                    free(content_length);
                    // check if the file is a pdf

                    h->next = malloc(sizeof(struct Header));
                    if (h->next == NULL)
                        printf("malloc failed\n");

                    h = h->next;
                    h->name = strdup("Content-Type");
                    char *extension = strrchr(req->url, '.');

                    if (extension == NULL)
                        h->values = strdup("text/*");
                    else if (strcmp(extension, ".pdf") == 0)
                        h->values = strdup("application/pdf");          // file is a pdf
                    else if (strcmp(extension, ".html") == 0)
                        h->values = strdup("text/html");                // file is a html
                    else if (strcmp(extension, ".jpg") == 0)
                        h->values = strdup("image/jpeg");               // file is a txt
                    else
                        h->values = strdup("text/*");                   // any other file

                    h->next = NULL;

                    send_response_header(new_socket, response);

                    if (is_modified)
                        send_response_file(new_socket, req->url);
                }
                else
                {
                    // handle PUT request
                    int content_length = find_content_length_value(req) - strlen(req->enttity_body);
                    printf("content length: %d\n", content_length);

                    if (content_length == -1)
                    {
                        // content length not found
                        send_put_response(new_socket, 400);
                        exit(EXIT_SUCCESS);
                    }

                    receive_and_write_to_file(new_socket, req->url, content_length, req->enttity_body);
                    send_put_response(new_socket, 200);
                }
            }

            free_request(req);
            printf("Connection closed with client\n");
            exit(EXIT_SUCCESS);
        }

        close(new_socket);
    }
    
    fclose(access_log);
    // free_request(req);
    return 0;
}