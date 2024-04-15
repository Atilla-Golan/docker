#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/select.h>

#define PORT1 4001
#define PORT2 4002
#define PORT3 4003
#define HOST "172.17.0.1"
#define BUFFER_SIZE 1024

// Function to get current timestamp in milliseconds
long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    return milliseconds;
}

// Function to create a non-blocking socket and connect to a server
int setup_connection(int port) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set the socket as non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    memset(&servaddr, 0, sizeof(servaddr));

    // Server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(HOST);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect failed");
            exit(EXIT_FAILURE);
        }
    }

    printf("Connection to port %d initiated.\n", port);

    return sockfd;
}

// Function to read from a socket with a timeout
int read_with_timeout(int sockfd, char *buffer, int timeout_ms) {
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    // Wait for an input for a certain time
    int ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0) {
        // Data is available to read
        memset(buffer, 0, BUFFER_SIZE);
        int n = read(sockfd, buffer, BUFFER_SIZE - 1);

  if (n > 1) {
            int start = 0;
            int end = n-1;
            for(int lastnlpos = n - 2; lastnlpos >=0; lastnlpos--)
            {
                if (buffer[lastnlpos] == '\n')
                {
                    start = lastnlpos+1;
                    break;
                }
            }
           // printf("Before selecting last value: %s \n -----------------\n", buffer);
            int i = 0;
            for(; i < end-start; i++)
            {
                buffer[i] = buffer[i+start];
            }
            buffer[i] = '\0';
          //  printf("After  selecting last value: %s \n ========================================================\n", buffer);
            return 1;
        }
    } else if (ret == 0) {
        //printf("Read timeout occurred.\n"); // Debugging print
    }
    return 0;
} 

int main() {
    int sockfd1 = setup_connection(PORT1);
    int sockfd2 = setup_connection(PORT2);
    int sockfd3 = setup_connection(PORT3);

    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE], buffer3[BUFFER_SIZE];
    int has_data1, has_data2, has_data3;

    while (1) {
        long long start_time = current_timestamp();

        has_data1 = read_with_timeout(sockfd1, buffer1, 100);
        has_data2 = read_with_timeout(sockfd2, buffer2, 100);
        has_data3 = read_with_timeout(sockfd3, buffer3, 100);

        long long end_time = current_timestamp();

        printf("{\"timestamp\": %lld, \"out1\": \"%s\", \"out2\": \"%s\", \"out3\": \"%s\"}\n",
               end_time,
               has_data1 ? buffer1 : "--",
               has_data2 ? buffer2 : "--",
               has_data3 ? buffer3 : "--");

        // Calculate remaining time to wait to make up a total of 100ms.
        long long time_to_wait = 100 - (end_time - start_time); // in ms
        if (time_to_wait > 0) {
            usleep(time_to_wait * 1000); // convert ms to microseconds and sleep
        }
    }

    // Cleanup
    close(sockfd1);
    close(sockfd2);
    close(sockfd3);

    return 0;
}
