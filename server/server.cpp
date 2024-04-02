#include "server.h"
#include <unistd.h>
#include <cstring>

ChatServer::ChatServer(int port) : m_port(port), m_serverSocket(-1), message_id_counter(0) {
    m_boards["Public:"];
    for (int i = 1; i <=5; ++i) {
        m_boards["Private" + std::to_string(i)];
        m_boardMembers["Private" + std::to_string(i)];
    }
}

ChatServer::~ChatServer() {
    close(m_serverSocket);
}


void ChatServer::setup() {
    // Create socket
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        exit(1);
    }

    // Set up server Address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(m_port);

    std::cout << "[STARTUP] Server started on port: " << m_port << std::endl;

    // Bind Socket
    if (bind(m_serverSocket, 
            (struct sockaddr*)&serverAddress, 
            sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket\n";
        exit(1);
    }
}



void ChatServer::handleClient(int clientSocket) {
    //send welcome message
    send(clientSocket, "CONNECTED", 9, 0);

    char buffer [1024];
    std::string username, currentBoard;

    while (true){
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesReceived = recv(clientSocket, 
                                    buffer, sizeof(buffer) - 1, 0);

        std::string message = buffer;
        std::string command = message.substr(0,message.find(" "));
        std::string content = message.substr(message.find(" ") + 1);

        if (command == "LOGIN") {
            username = content;
            m_mutex.lock();
            bool usernameExists = false;
            for (const auto& client : m_clients) {
                if (client.second == username) {
                    usernameExists = true;
                    break;
                }
            }

            if (usernameExists) {
                const char* errorMsg = "ERROR: Username already exits";
                send(clientSocket, errorMsg, strlen(errorMsg), 0);

            }
            else {
                m_clients[clientSocket] = username;
                std::pair<int, bool> clientPair = std::make_pair(clientSocket, false);
                m_boardMembers["Public:"].push_back(clientPair);
                std::cout << "[CLIENT] Logged in username: " << username << std::endl;
                const char* Logginmsg = "SUCCESFULL";
                send(clientSocket, Logginmsg, strlen(Logginmsg), 0);
                std::string notification = "[SERVER] New User Joined: " + username;
                for (const auto& client : m_clients) {
                    if (client.first != clientSocket) {
                        send(client.first, notification.c_str()
                                , notification.size(), 0);
                    }
                }
            }
            m_mutex.unlock();       

            
        }
        
        else if (command == "POST") {
            Message msg;
            msg.id = message_id_counter++;
            msg.sender = username;
            msg.postDate = time(0);
            msg.subject = content.substr(0,content.find(":"));
            msg.content = content.substr(content.find(":") + 1);
            m_mutex.lock();
            if (currentBoard == "Public:") {
                m_boards[currentBoard].push_back(msg);

                std::string board_message = "\n\nMessage ID: " + std::to_string(msg.id) + 
                "\n Sender: " + msg.sender + "\n Post Date: " +
                std::to_string(msg.postDate) + "\n Subject: " + msg.subject + 
                "\n Message content: " + msg.content;

                for (auto& userSocket : m_boardMembers[currentBoard]) {
                    if (userSocket.second){
                        send(userSocket.first, board_message.c_str(), board_message.size(), 0);
                    }
                }
            }
            else {
                m_boards[currentBoard].push_back(msg);

                std::string group_message = "\n\nMessage ID: " + std::to_string(msg.id) + 
                "\n Sender: " + msg.sender + "\n Post date; " + std::to_string(msg.postDate) + 
                "\n Subject: " + msg.subject + "\n Message content: " + msg.content;

                for (auto& userSocket : m_boardMembers[currentBoard]) {
                    if (userSocket.second){
                            send(userSocket.first, group_message.c_str(), group_message.size(), 0);
                    }
                }
            }
            m_mutex.unlock();
        }
        
        else if ( command == "RET") {
            int messageID = std::stoi(content);
            bool msgFound = false;

            for (const auto& msg : m_boards[currentBoard]) {
                if(msg.id == messageID){
                    std::string retMsg = "\n\nMessage ID: " + std::to_string(msg.id) + 
                    "\n Sender: " + msg.sender + "\n Post date; " + std::to_string(msg.postDate) + 
                    "\n Subject: " + msg.subject + "\n Message content: " + msg.content;

                    send(clientSocket, retMsg.c_str(), retMsg.size(), 0);

                    msgFound = true;
                }
            }
            if (!msgFound){
                std::string errorMsg = "Could not retreive message: " + std::to_string(messageID);
                send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
            }
        } 
        
        else if (command == "USERS") {
            //Send back a list of users
            m_mutex.lock();
            std::string userList = "Users on this Board: ";
            for (const auto& client : m_boardMembers[currentBoard]) {
                userList += m_clients[client.first] + "\n ";
            }
            userList = userList.substr(0, userList.size() - 2);
            send(clientSocket, userList.c_str(), userList.size(), 0);
            m_mutex.unlock();

        } 

        else if (command == "BOARD") {
            //Send back a list of boards
            //should only be boards the user was added to
            m_mutex.lock();
            std::string boardsList = "All Boards: ";
            for (const auto& board : m_boards) {
                boardsList += board.first + "\n";
            }
            boardsList += "\nAll Groups: ";
            for (const auto& group : m_boardMembers) {
                boardsList += group.first + "\n";
            }
            boardsList = boardsList.substr(0, boardsList.size()-1);
            send(clientSocket, boardsList.c_str(), boardsList.size(), 0);
            m_mutex.unlock();
        } 
        
        else if (command == "JOIN") {
            std::string boardName = content;
            m_mutex.lock();
            if (m_boards.find(boardName) != m_boards.end()) {
                auto& boardMembers = m_boardMembers[boardName];
                bool isMember = false;
                for (auto& userSocket : boardMembers) {
                    if (userSocket.first == clientSocket) {
                        isMember = true;
                        userSocket.second = true;  // Update the status to true
                        break;
                    }
                }
                if (!isMember) {
                    // The client is not already a member of the board, so add them
                    std::string errorMsg = "You are not a member of this board: " + boardName;
                    send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
                }

                else {
                    int start = std::max(0, static_cast<int>(m_boards[boardName].size()) - 2);
                    for (int i = start; i < m_boards[boardName].size(); ++i) {
                        Message& msg = m_boards[boardName][i];
                        std::string messageDetails = "\n\nMessage ID:" + 
                        std::to_string(msg.id) + "\n Sender: " + msg.sender + 
                        "\n Post Date: " + std::to_string(msg.postDate) + 
                        "\n Subject: " + msg.subject + "\n Message Content: " + msg.content;

                        send(clientSocket, messageDetails.c_str(), messageDetails.size(), 0);
                    }
                    currentBoard = boardName;
                    }
            }
            m_mutex.unlock();
        }
        
        else if (command == "GJOIN") {
            std::string groupName = content;
            m_mutex.lock();
            std::pair<int, bool> clientPair = std::make_pair(clientSocket, false);
            if (m_boardMembers.find(groupName) != m_boardMembers.end()) {
                m_boardMembers[groupName].push_back(clientPair);
                std::string confirmation = "You have Joinded the group: " + groupName;
                send(clientSocket, confirmation.c_str(), confirmation.size(), 0);
                std::string notification = "[SERVER] New User joined the group: " + username;
                for (auto memberSocket : m_boardMembers[groupName]) {
                    if (memberSocket.first != clientSocket) {
                        send(memberSocket.first, notification.c_str(), notification.size(), 0);
                    }
                }
            } else {
                std::string error = "The group Does not exits: " + groupName;

                send(clientSocket, error.c_str(), error.size(), 0);
            }
            m_mutex.unlock();
        } 
        
        else if (command == "DISC") {
            m_mutex.lock();
            std::string username = m_clients[clientSocket];
            m_clients.erase(clientSocket);
            m_mutex.unlock();

            std::string notification = "[SERVER] User Disconnected: " + username;
            for (const auto& client : m_clients) {
                if (client.first != clientSocket) {
                    send(client.first, notification.c_str(), notification.size(), 0);
                }
            }
            const char* discMsg = "You have been disconnected from the server";
            send(clientSocket, discMsg, strlen(discMsg), 0);

            close(clientSocket);
            break;
      
        }
    }
    
}

void ChatServer::serverState(){
    while (true) {
            m_mutex.lock();
            std::cout << "\n--- Server State ---\n";
            std::cout << "Available Boards: " << m_boards.size() << "\n";
            for (const auto& board : m_boards) {
                std::cout << "Board: " << board.first << ", Messages: " << board.second.size() << "\n";
            }
            std::cout << "Connected Users: " << m_clients.size() << "\n";
            for (const auto& client : m_clients) {
                std::cout << "User: " << client.second << "\n";
            }
            for (const auto& board : m_boards) {
                std::cout << "Board: " << board.first << ", Users: " << board.second.size() << "\n";
            }
            for (const auto& group : m_boardMembers) {
                std::cout << "Group: " << group.first << ", Users: " << group.second.size() << "\n";
            }
            m_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(10));  // Print server state every 10 seconds
        }
}

void ChatServer::run() {

    // Start a thread to print server state
    std::thread serverStateThread(&ChatServer::serverState, this);
    serverStateThread.detach();
    //listen for connections
    listen(m_serverSocket, SOMAXCONN);

    while (true) {
        //Accept client connections
        int clientSocket = accept(m_serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection \n";
            continue;
        }


        std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
        clientThread.detach(); //Detatch thread
        
        std::cout << "[CONNECTION] Client connected. Socket: " << clientSocket << std::endl;
    }
}