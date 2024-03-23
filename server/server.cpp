#include "server.h"
#include <unistd.h>
#include <cstring>

ChatServer::ChatServer(int port) : m_port(port), m_serverSocket(-1), message_id_counter(0) {}

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

    if (bind(m_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket\n";
        exit(1);
    }

    
}

void ChatServer::handleClient(int clientSocket) {
    //send welcome message

    const char* welcomeMessage = "CONNECTED";
    send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0);

    char buffer [1024];
    std::string username;
    std::string currentBoard;

    //Read incoming messages untill disconnect

    while (true){
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        // Add a bytes received <= 0

        //parser
        std::string message = buffer;
        std::string command = message.substr(0,message.find(" "));
        std::string content = message.substr(message.find(" ") + 1);

        //Login
        if (command == "LOGIN") {
            username = content;
            m_mutex.lock();
            m_clients[clientSocket] = username;

            for (auto& board : m_boards) {
                if(std::find(m_boardMembers[board.first].begin(), m_boardMembers[board.first].end(),clientSocket) == m_boardMembers[board.first].end()) {
                    m_boardMembers[board.first].push_back(clientSocket);
                }
            }
            m_mutex.unlock();
            std::cout << "[CLIENT] Logged in username: " << username << std::endl;
            const char* Logginmsg = "SUCCESFULL";
            send(clientSocket, Logginmsg, strlen(Logginmsg), 0);
        }else if (command == "MSG") {
            //Receiv message
            
            Message msg;
            msg.id = message_id_counter++;
            msg.sender = username;
            msg.postDate = time(0);
            msg.subject = content.substr(0,content.find(":"));
            msg.content = content.substr(content.find(":") + 1);
            m_mutex.lock();
            m_boards[currentBoard].push_back(msg);

            std::string board_message = "\n\nMessage ID: " + std::to_string(msg.id) + "\n Sender: " + msg.sender + "\n Post Date: " + std::to_string(msg.postDate) + "\n Subject: " + msg.subject + "\n Message content: " + msg.content;
            //broadcast to board
            for (int userSocket : m_activeMembers[currentBoard]) {
                send(userSocket, board_message.c_str(), board_message.size(), 0);
            }
            m_mutex.unlock();


        }else if ( command == "RET") {
            //Retreive specific message
            //parse out specific message identifier
        } else if (command == "USERS") {
            //Send back a list of users
            m_mutex.lock();
            std::string userList = "Connected Users: ";
            for (const auto& client : m_clients) {
                userList += client.second + "\n ";
            }
            userList = userList.substr(0, userList.size() - 2);
            send(clientSocket, userList.c_str(), userList.size(), 0);
            m_mutex.unlock();
        } else if (command == "BOARD") {
            //Send back a list of boards
            //should only be boards the user was added to
            m_mutex.lock();
            std::string boardsList = "Available Boards: ";
            for (const auto& board : m_boards) {
                if (std::find(m_boardMembers[board.first].begin(), m_boardMembers[board.first].end(), clientSocket) != m_boardMembers[board.first].end()){
                    boardsList += board.first + "\n";
                }
                
            }
            for (const auto& group : m_groups) {
                if (std::find(m_groupMembers[group.first].begin(), m_groupMembers[group.first].end(), clientSocket) != m_groupMembers[group.first].end()){
                    boardsList += group.first + "\n";
                }
            }
            boardsList = boardsList.substr(0, boardsList.size()-1);
            send(clientSocket, boardsList.c_str(), boardsList.size(),0);
            m_mutex.unlock();
        } else if (command == "SWITCH") {
            //Switch to new board
            //retreive prior messages on board
            
            m_mutex.lock();
            if (m_boardMembers.find(content) != m_boardMembers.end() && std::find(m_boardMembers[content].begin(), m_boardMembers[content].end(), clientSocket) != m_boardMembers[content].end()) {
                m_activeMembers[currentBoard].erase(std::remove(m_activeMembers[currentBoard].begin(), m_activeMembers[currentBoard].end(), clientSocket), m_activeMembers[currentBoard].end());
                currentBoard = content;
                m_activeMembers[currentBoard].push_back(clientSocket);
            

            
                std::string boardMessages = "Messages from " + currentBoard + ": ";
                for (const auto& msg : m_boards[currentBoard]) {
                    boardMessages += "\n\nMessage ID: " + std::to_string(msg.id) + "\n Sender: " + msg.sender + "\n Post Date: " + std::to_string(msg.postDate) + "\n Subject: " + msg.subject + "\n Message content: " + msg.content;
                }
                send(clientSocket, boardMessages.c_str(), boardMessages.size(), 0);
            }else if (m_groupMembers.find(content) != m_groupMembers.end() && std::find(m_groupMembers[content].begin(), m_groupMembers[content].end(), clientSocket) != m_groupMembers[content].end()) {
                m_activeMembers[currentBoard].erase(std::remove(m_activeMembers[currentBoard].begin(), m_activeMembers[currentBoard].end(), clientSocket), m_activeMembers[currentBoard].end());
                currentBoard = content;
                m_activeMembers[currentBoard].push_back(clientSocket);

                std::string boardMessages = "Messages from " + currentBoard + ": ";
                for (const auto& msg : m_groups[currentBoard]) {
                    boardMessages += "\n\nMessage ID: " + std::to_string(msg.id) + "\n Sender: " + msg.sender + "\n Post Date: " + std::to_string(msg.postDate) + "\n Subject: " + msg.subject + "\n Message content: " + msg.content;
                }
                send(clientSocket, boardMessages.c_str(), boardMessages.size(), 0);

            }
             else {
                std::string not_allowed = "You are not a member of the Requested Board Group.";
                send(clientSocket, not_allowed.c_str(), not_allowed.size(),0);
            }
            m_mutex.unlock();
        } else if (command == "DISC") {
            //Disconnect from server
            break;
        } else if (command == "ADD") {
            //Add a new board group
            //Add a new user to a board
            std::string type = content.substr(0, content.find(" "));
            std::string name = content.substr(content.find(" ") + 1);


            m_mutex.lock();
            if (m_boards.find(name) != m_boards.end()) {
                // The board or group already exists
                std::string error = "A board or group with this name already exists.";
                send(clientSocket, error.c_str(), error.size(), 0);
            } else if (m_groups.find(name) != m_groups.end()){
                std::string error = "A board or group with this name already exists.";
                send(clientSocket, error.c_str(), error.size(), 0);
            } else{

            
                if (type == "BOARD") {
                    // Add a new board for all clients
                    
                    m_boards[name];
                    for (const auto& client: m_clients) {
                        m_boardMembers[name].push_back(client.first);
                    }
                    
                } else if (type == "GROUP") {
                    // Add a group with only the client
                    
                    m_groups[name];
                    m_groupMembers[name].push_back(clientSocket);
                    
                } else if (type == "USER") {
                    
                    
                    for (const auto& client: m_clients) {
                        if (client.second == name) {
                            m_groupMembers[currentBoard].push_back(client.first);
                            break;
                        }
                    }
                    
                    
                    
                }
            }
            m_mutex.unlock();

        }

    }

    //Remove disconnected client
    m_mutex.lock();
    m_clients.erase(clientSocket);
    m_mutex.unlock();

    close(clientSocket);
}


void ChatServer::run() {
    //listen for connections
    listen(m_serverSocket, SOMAXCONN);

    while (true) {
        //Accept client connections
        int clientSocket = accept(m_serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection \n";
            continue;
        }

        //create new thread to handle client
        std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
        clientThread.detach(); //Detatch thread
        
        std::cout << "[CONNECTION] Client connected. Socket: " << clientSocket << std::endl;
    }
}