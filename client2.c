#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>

#define SERVER_IP "172.17.0.1"
#define UDP_PORT 4000
#define TCP_PORT1 4001
#define TCP_PORT2 4002
#define TCP_PORT3 4003
#define BUFFER_SIZE 1024
#define CONTROL_INTERVAL 20 // milliseconds

// Prototypes
int create_socket_and_connect(int port, int is_udp);
void configure_server_output(int udp_socket, unsigned short object, unsigned short property, unsigned short value);
void read_and_control(int sock1, int sock2, int sock3, int udp_sock);

int main() {
    int sock1 = create_socket_and_connect(TCP_PORT1, 0); // TCP
    int sock2 = create_socket_and_connect(TCP_PORT2, 0); // TCP
    int sock3 = create_socket_and_connect(TCP_PORT3, 0); // TCP
    int udp_sock = create_socket_and_connect(UDP_PORT, 1); // UDP

    read_and_control(sock1, sock2, sock3, udp_sock);

    close(sock1);
    close(sock2);
    close(sock3);
    close(udp_sock);

    return 0;
}

int create_socket_and_connect(int port, int is_udp) {
    int sock = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (!is_udp) { // TCP connections need to connect; UDP does not.
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(SERVER_IP);

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Set TCP sockets to non-blocking
        fcntl(sock, F_SETFL, O_NONBLOCK);
    }

    return sock;
}

void configure_server_output(int udp_socket, unsigned short object, unsigned short property, unsigned short value) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    unsigned short message[4];
    message[0] = htons(2); // Write operation
    message[1] = htons(object);
    message[2] = htons(property);
    message[3] = htons(value);

    sendto(udp_socket, message, sizeof(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void read_and_control(int sock1, int sock2, int sock3, int udp_sock) {
    fd_set readfds;
    struct timeval tv;
    int max_sd, activity;
    char buffer[3][BUFFER_SIZE];
    long long int last_timestamp = 0, current_timestamp;
    struct timespec spec;
    for (int i=0; i < 3; i++)
        strcpy(buffer[i], "--");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        FD_SET(sock3, &readfds);
        max_sd = sock1 > sock2 ? (sock1 > sock3 ? sock1 : sock3) : (sock2 > sock3 ? sock2 : sock3);

        // Set timeout to 20ms
        tv.tv_sec = 0;
        tv.tv_usec = 20000;

        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

        if ((activity < 0) && (errno != EINTR)) {
            printf("Select error");
        }

        clock_gettime(CLOCK_REALTIME, &spec);
        current_timestamp = spec.tv_sec * 1000 + spec.tv_nsec / 1.0e6;
        /* clock_t ticks;
        double current_timestamp;

        ticks = clock(); // Get the number of clock ticks elapsed since the program started

        // Convert clock ticks to milliseconds (1 tick = 1/CLOCKS_PER_SEC seconds)
        current_timestamp = (ticks / (double)CLOCKS_PER_SEC) * 1000.0; */

        if (current_timestamp - last_timestamp >=

 CONTROL_INTERVAL) {
            printf("{\"timestamp\": %lld, \"out1\": \"%s\", \"out2\": \"%s\", \"out3\": \"%s\"}\n",
                   current_timestamp, buffer[0], buffer[1], buffer[2]);
            last_timestamp = current_timestamp;
        }

        for (int i = 0; i < 3; i++) {
            if (FD_ISSET(i == 0 ? sock1 : (i == 1 ? sock2 : sock3), &readfds)) {
                memset(buffer[i], 0, BUFFER_SIZE);
                int n = read(i == 0 ? sock1 : (i == 1 ? sock2 : sock3), buffer[i], BUFFER_SIZE);
                if (n <= 1)
                {
                    strcpy(buffer[i], "--");
                }
                else
                {
                    int start = 0;
                    int end = n-1;
                    for(int lastnlpos = n - 2; lastnlpos >=0; lastnlpos--)
                    {
                        if (buffer[i][lastnlpos] == '\n')
                        {
                            start = lastnlpos+1;
                            break;
                        }
                    }
                    int j = 0;
                    for(; j < end-start; j++)
                    {
                        buffer[i][j] = buffer[i][j+start];
                    }
                    buffer[i][j] = '\0';
                }
            }
        }

        // Control logic based on out3's value
        if (strcmp(buffer[2], "--") != 0) { // Check if out3 has a value
            float out3_value = atof(buffer[2]);
            if (out3_value >= 3.0) {
                configure_server_output(udp_sock, 1, 1, 8000); // Set frequency and amplitude for output 1
            } else {
                configure_server_output(udp_sock, 1, 2, 4000); // Reset frequency and amplitude for output 1
            }
        }
    }
}