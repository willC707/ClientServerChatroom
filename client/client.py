import socket
import threading

def receive_handler(client_socket):
    while True:
        msg = client_socket.recv(1024).decode()
        if not msg:
            break

        print(msg)
        print("->",end='',flush=True)



def client_program():
    host = 'localhost'
    port = 8080

    client_socket = socket.socket()
    client_socket.connect((host, port))

    connected = client_socket.recv(1024).decode()
    if(connected == "CONNECTED"):
        print("Conneced Sucessfully!")

    username = input("Login to Server: ")
    login_command = f'LOGIN {username}'
    client_socket.send(login_command.encode())

    response = client_socket.recv(1024).decode()
    if response == "SUCCESFULL":
        print("Logged into server as: " + username)

    threading.Thread(target=receive_handler, args=(client_socket,)).start()

    cont = True
    while cont:
        message = input("->")
        if message.lower().strip() == 'help':
            #help message
            print("This is a help message")
            continue
        else:
            msg_string = None
            if message.lower().strip() == 'message':
                subject = input("Subject: ")
                content = input("Message: ")
                msg_string = f'MSG {subject}:{content}'
                #client_socket.send(msg_string.encode())
            elif message.lower().strip() == 'users':
                msg_string = f'USERS '
            elif message.lower().strip() == 'boards':
                msg_string = f'BOARD '
            elif message.lower().strip() == 'switch':
                new_board = input("board: ")
                msg_string = f'SWITCH {new_board}'

            elif message.lower().strip() == 'add':
                add_type = input("Select Type to Add [BOARD/GROUP/USER]")
                type_name = input("Name: ")
                msg_string = f'ADD {add_type} {type_name}'
            
            elif message.lower().strip() == 'bye':
                print("Entered quit phase")
                print(message.lower().strip())
                msg_string = f'DISC '
                cont = False
            else:
                print("Not valid command")

            if msg_string is not None:
                client_socket.send(msg_string.encode())

    client_socket.close()



if __name__ == '__main__':
    client_program()