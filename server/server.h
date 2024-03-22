#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include <map>
#include <algorithm>

struct Message {
    int id;
    std::string sender;
    time_t postDate;
    std::string subject;
    std::string content;
};

class ChatServer {
    private:
        int m_port;
        int m_serverSocket;
        std::vector<std::thread> m_clientThreads;
        std::mutex m_mutex;
        std::map<int, std::string>m_clients;
        std::map<std::string, std::vector<Message>> m_boards;
        std::map<std::string, std::vector<int>> m_boardMembers;
        //std::vector<Message> m_messages;
        //std::vector<std::string> m_boards;

        void handleClient(int clientSocket);

    public:
        ChatServer(int port);
        ~ChatServer();

        void setup();
        void run();

};

#endif //SERVER_H