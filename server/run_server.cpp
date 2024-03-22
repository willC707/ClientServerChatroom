#include "server.h"

int main() {
    const int port = 8080; // set port
    ChatServer ChatServer(port);
    ChatServer.setup();
    ChatServer.run();
    return 0;
}