#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <netinet/in.h>

class ChatServer {
    private:
        int m_port;
        int m_serverSocket;
        std::vector<std::thread> m_clientThreads;
        std::mutex m_mutex;

        void handleClient(int clientSocket);

    public:
        ChatServer(int port);
        ~ChatServer();

        void setup();
        void run();

};

#endif //SERVER_H