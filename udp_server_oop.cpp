#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <vector>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;
    bool running;

public:
    ServerUDP(int port = PORT) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Помилка створення сокета");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Помилка прив'язки");

        // Автоматичне очищення завершених процесів (запобігання зомбі)
        signal(SIGCHLD, SIG_IGN); 

        running = true;
        std::cout << "UDP сервер (Multiprocessing) запущено на порту " << port << std::endl;
    }

    ~ServerUDP() {
        close(sockfd);
    }

    void start() {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[BUFFER_SIZE];

        while (running) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                 (struct sockaddr*)&client_addr, &addr_len);
            
            if (n > 0) {
                pid_t pid = fork(); // СТВОРЕННЯ НОВОГО ПРОЦЕСУ

                if (pid < 0) {
                    std::cerr << "Помилка створення процесу" << std::endl;
                } else if (pid == 0) {
                    // Це ДОЧІРНІЙ ПРОЦЕС
                    handle_client(std::string(buffer), client_addr);
                    exit(0);
                }
            }
        }
    }

private:
    void handle_client(const std::string &message, sockaddr_in client_addr) {
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        
        std::cout << "[Процес PID " << getpid() << "] Отримано від " 
                  << client_ip << ":" << client_port << " -> " << message << std::endl;
        
    }
};

int main() {
    try {
        ServerUDP server;
        server.start();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
