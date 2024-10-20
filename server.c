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
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <time.h>


#define UNIX_SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 10000

// Функція для вимірювання часу
double measure_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

// Сервер для INET сокетів
void inet_server(int port, int non_blocking, int async) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    struct timespec start, end;

    // Створення сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("INET Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (non_blocking) {
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Прив'язка сокета
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослуховування з'єднань
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("INET server listening on port %d...\n", port);
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Основний цикл для прийняття з'єднань
   while (1) {
        if (async) {
            // Асинхронний режим з використанням poll
            struct pollfd pfd;
            pfd.fd = server_fd;
            pfd.events = POLLIN;

            int ret = poll(&pfd, 1, 1000); // 1 секунда таймаут
            if (ret == -1) {
                perror("Poll failed");
                break;
            }
            if (ret > 0 && (pfd.revents & POLLIN)) {
                // Прийняття клієнтського підключення
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Немає доступних з'єднань, продовжуємо
                        continue;
                    } else {
                        perror("Accept failed");
                        continue;
                    }
                }
                clock_gettime(CLOCK_MONOTONIC, &end);
                printf("Connection established. Time to accept: %f seconds\n", measure_time(start, end));
                
                // Читання даних від клієнта
                ssize_t bytes_received;
                while ((bytes_received = read(client_fd, buffer, sizeof(buffer))) > 0) {
                    //printf("Received %ld bytes\n", bytes_received);
                }
                close(client_fd);
            }
        } else {
            // Синхронний режим
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Немає доступних з'єднань, продовжуємо
                    continue;
                } else {
                    perror("Accept failed");
                    continue;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            printf("Connection established. Time to accept: %f seconds\n", measure_time(start, end));
            
            // Читання даних від клієнта
            ssize_t bytes_received;
            while ((bytes_received = read(client_fd, buffer, sizeof(buffer))) > 0) {
                //printf("Received %ld bytes\n", bytes_received);
            }
            close(client_fd);
        }
    }

    close(server_fd);
}
    

// Сервер для UNIX сокетів
void unix_server(int non_blocking, int async) {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    struct timespec start, end;

    // Створення UNIX сокета
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("UNIX Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (non_blocking) {
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
    }server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);
    unlink(UNIX_SOCKET_PATH);

    // Прив'язка сокета
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослуховування з'єднань
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("UNIX server listening on path %s...\n", UNIX_SOCKET_PATH);
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Основний цикл для прийняття з'єднань
    while (1) {
        if (async) {
            // Асинхронний режим з використанням poll
            struct pollfd pfd;
            pfd.fd = server_fd;
            pfd.events = POLLIN;

            int ret = poll(&pfd, 1, 1000); // 1 секунда таймаут
            if (ret == -1) {
                perror("Poll failed");
                break;
            }
            if (ret > 0 && (pfd.revents & POLLIN)) {
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Якщо немає нових з'єднань, продовжуємо без виведення помилки
                        continue;
                    } else {
                        perror("Accept failed");
                        continue;
                    }
                }
                clock_gettime(CLOCK_MONOTONIC, &end);
                printf("Connection established. Time to accept: %f seconds\n", measure_time(start, end));
                
                // Читання даних від клієнта
                int bytes_received;
                while ((bytes_received = read(client_fd, buffer, sizeof(buffer))) > 0) {
                    //printf("Received %d bytes\n", bytes_received);
                }
                close(client_fd);
            }
        } else {
            // Синхронний режим
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Якщо немає нових з'єднань, продовжуємо без виведення помилки
                    continue;
                } else {
                    perror("Accept failed");
                    continue;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            printf("Connection established. Time to accept: %f seconds\n", measure_time(start, end));
            
            // Читання даних від клієнта
            int bytes_received;
            while ((bytes_received = read(client_fd, buffer, sizeof(buffer))) > 0) {
                //printf("Received %d bytes\n", bytes_received);
            }
            close(client_fd);
        }
    }

    close(server_fd);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <inet/unix> <port/path> <blocking (0/1)> <async (0/1)>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int non_blocking = atoi(argv[3]);
    int async = atoi(argv[4]);

    if (strcmp(argv[1], "inet") == 0) {
        int port = atoi(argv[2]);
        inet_server(port, non_blocking, async);
    } else if (strcmp(argv[1], "unix") == 0) {
        unix_server(non_blocking, async);
    } else {
        printf("Invalid socket type. Use 'inet' or 'unix'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}