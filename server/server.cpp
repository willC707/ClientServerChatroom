#include <iostream>
#include <thread>
#include <vector>
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class SocketServer {
public:
    SocketServer(int port) : port(port), server_socket(INVALID_SOCKET) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize winsock" << std::endl;
            return;
        }
#endif
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }

        sockaddr_in server_addr = { 0 };
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind socket" << std::endl;
            return;
        }

        if (listen(server_socket, 1) == SOCKET_ERROR) {
            std::cerr << "Failed to listen on socket" << std::endl;
            return;
        }
    }

    ~SocketServer() {
        closesocket(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void run() {
        while (true) {
            sockaddr_in client_addr;
            socklen_t client_size = sizeof(client_addr);
            SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_size);
            if (client_socket == INVALID_SOCKET) {
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }

            std::thread(&SocketServer::handle_client, this, client_socket).detach();
        }
    }

private:
    int port;
    SOCKET server_socket;

    void handle_client(SOCKET client_socket) {
        // Handle the client connection here
        // For now, we'll just close the socket
        closesocket(client_socket);
    }
};

int main() {
    SocketServer server(8080);
    server.run();
    return 0;
}