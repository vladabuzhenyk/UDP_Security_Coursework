#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    std::cout << "Клієнт запущений. Введіть повідомлення (quit для виходу):" << std::endl;

    while (true) {
        std::string message;
        std::getline(std::cin, message);

        if (message == "quit") break;

        sendto(sockfd, message.c_str(), message.size(), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));
    }

    close(sockfd);
    return 0;
}
