#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>

#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex console_mutex; // М'ютекс для синхронізації виводу в консоль

class ServerUDP {
private:
    int sockfd;
    struct sockaddr_in server_addr;
    bool running;

public:
    ServerUDP(int port = PORT) {
        // Створення UDP сокета (SOCK_DGRAM)
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) throw std::runtime_error("Помилка створення сокета");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        // Прив'язка сокета до порту
        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Помилка прив'язки (bind)");

        running = true;
        std::cout << "UDP сервер (Multithreading) запущено на порту " << port << std::endl;
    }

    ~ServerUDP() { 
        close(sockfd); // Принцип RAII: закриття ресурсу в деструкторі
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
                // Створення нового потоку для обробки повідомлення
                // Це дозволяє серверу миттєво повернутися до recvfrom()
                std::thread t(&ServerUDP::handle_client, this, std::string(buffer), client_addr);
                t.detach(); // Від'єднуємо потік, щоб він працював незалежно
            }
        }
    }

private:
    void handle_client(const std::string message, sockaddr_in client_addr) {
        // Блокуємо консоль м'ютексом, щоб повідомлення від різних потоків не перемішувалися
        std::lock_guard<std::mutex> lock(console_mutex);
        
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        
        std::cout << "[Потік ID: " << std::this_thread::get_id() << "] "
                  << client_ip << ":" << client_port << " -> " << message << std::endl;
    }
};

int main() {
    try {
        ServerUDP server;
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Критична помилка: " << e.what() << std::endl;
    }
    return 0;
}
