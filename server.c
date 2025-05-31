#include <stdio.h>          // printf, perror
#include <string.h>         // strlen, strncmp  
#include <stdlib.h>         // exit()
#include <unistd.h>         // close()
#include <sys/types.h>      // types
#include <sys/socket.h>     // socket, bind, listen, accept
#include <netinet/in.h>     // sockaddr_in
#include <fcntl.h>          // open()
#include <sys/stat.h>       // stat()
#include <time.h>           // time, localtime, strftime

#define PORT 8080
#define BUFFER_SIZE 4096
#define AUTHOR_NAME "Mykola Derevianko"

void send_response(int client_socket, const char *path) {
    char buffer[BUFFER_SIZE];

    // Open the requested file
    int file_fd = open(path, O_RDONLY);
    if (file_fd == -1) {
        // if no file - send 404
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
        write(client_socket, not_found, strlen(not_found));
        return;
    }

    // get file size
    struct stat st;
    stat(path, &st);

    // Send HTTP response header
    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    write(client_socket, header, strlen(header));

    // Send html
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        write(client_socket, buffer, bytes_read);
    }

    close(file_fd);
}

void print_start_info() {
    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("Application started at: %s UTC | Author: %s | Listening on port: %d\n", time_buffer, AUTHOR_NAME, PORT);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // allow address reuse (socket opt)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket to IP/port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all interfaces (0.0.0.0)
    server_addr.sin_port = htons(PORT);        // Host-to-network byte order

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 10) < 0) { //10 - max number of pending connections
        perror("listen");
        exit(EXIT_FAILURE);
    }

    print_start_info();

    while (1) {
        client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        int read_size = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (read_size > 0) {
            buffer[read_size] = '\0';  //make valid string

            printf("Received request:\n%s\n", buffer);

            // Check method and path
            if (strncmp(buffer, "GET / ", 6) == 0) {
                send_response(client_socket, "public/index.html");
            } else {
                // If not "/", return 404
                const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
                write(client_socket, not_found, strlen(not_found));
            }
        }

        close(client_socket);
    }

    return 0;
}
