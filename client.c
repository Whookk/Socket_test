#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>

#define UNIX_SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 10000

// Функція для вимірювання часу
double measure_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

// Клієнт для INET сокетів
void inet_client(const char* ip, int port, unsigned int num_packets, unsigned int packet_size) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[packet_size];
    struct timespec start, end, send_start, send_end, close_start, close_end;

    // Створення сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Встановлення з'єднання
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("INET Connection established. Time to connect: %f seconds\n", measure_time(start, end));

    // Початок вимірювання часу для відправки пакетів
    clock_gettime(CLOCK_MONOTONIC, &send_start);

    // Відправка пакетів
    for (unsigned int i = 0; i < num_packets; i++) {
        send(sockfd, buffer, packet_size, 0);
        //Bprintf("%d %d %ld\n",num_packets,packet_size, bytes);

    } 

    // Кінець вимірювання часу для відправки пакетів
    clock_gettime(CLOCK_MONOTONIC, &send_end);

    // Обчислення часу та швидкості відправки пакетів
    double total_send_time = 0;
    double packets_per_second_inet = 0;
    double megabytes_sent_inet = 0;
    double mbps_inet = 0;
    total_send_time = measure_time(send_start, send_end);
    printf("Total time to send packets: %f seconds\n", total_send_time);
    packets_per_second_inet = (double)num_packets / measure_time(send_start, send_end);
     printf("Speed: %f packets/second\n", packets_per_second_inet);
    megabytes_sent_inet = (double)(num_packets * packet_size) / (1024.0 * 1024.0);
    mbps_inet = megabytes_sent_inet / measure_time(send_start,send_end);

    
    
    printf("Speed: %f MB/second\n", mbps_inet);

    // Початок вимірювання часу закриття сокета
    clock_gettime(CLOCK_MONOTONIC, &close_start);

    // Закриття сокета
    close(sockfd);

    // Кінець вимірювання часу закриття сокета
    clock_gettime(CLOCK_MONOTONIC, &close_end);

    // Виведення часу закриття сокета
    double close_time = measure_time(close_start, close_end);
    printf("Time to close socket: %f seconds\n", close_time);
}

// Клієнт для UNIX сокетів
void unix_client(int num_packets, int packet_size) {
    int sockfd;
    struct sockaddr_un server_addr;
    char buffer[packet_size];
    struct timespec start, end, send_start, send_end, close_start, close_end;

    // Створення сокета
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);

    // Встановлення з'єднання
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("UNIX Connection established. Time to connect: %f seconds\n", measure_time(start, end));

    // Початок вимірювання часу для відправки пакетів
    clock_gettime(CLOCK_MONOTONIC, &send_start);

    // Відправка пакетів
    for (int i = 0; i < num_packets; i++) {
        send(sockfd, buffer, packet_size, 0);
    }

    // Кінець вимірювання часу для відправки пакетів
    clock_gettime(CLOCK_MONOTONIC, &send_end);

    // Обчислення часу та швидкості відправки пакетів
    double total_send_time = measure_time(send_start, send_end);
    printf("Total time to send packets: %f seconds\n", total_send_time);
    double packets_per_second = num_packets / total_send_time;
    printf("Speed: %f packets/second\n", packets_per_second);
    double megabytes_sent = (double)(num_packets * packet_size) / (1024 * 1024); // Мегабайти
    double mbps = megabytes_sent / total_send_time; // Швидкість в мегабайтах за секунду
    printf("Speed: %f MB/second\n", mbps);

    // Початок вимірювання часу закриття сокета
    clock_gettime(CLOCK_MONOTONIC, &close_start);

    // Закриття сокета
    close(sockfd);

    // Кінець вимірювання часу закриття сокета
    clock_gettime(CLOCK_MONOTONIC, &close_end);

    // Виведення часу закриття сокета
    double close_time = measure_time(close_start, close_end);
    printf("Time to close socket: %f seconds\n", close_time);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <inet/unix> <ip/port/path> <num_packets> <packet_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    

    if (strcmp(argv[1], "inet") == 0) {
        int num_packets = atoi(argv[4]);
        int packet_size = atoi(argv[5]);
        char *ip = argv[2];
        int port = atoi(argv[3]);
        inet_client(ip, port, num_packets, packet_size);
    } else if (strcmp(argv[1], "unix") == 0) {
        int num_packets = atoi(argv[3]);
        int packet_size = atoi(argv[4]);
        unix_client(num_packets, packet_size);
    } else {
        printf("Invalid socket type. Use 'inet' or 'unix'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
