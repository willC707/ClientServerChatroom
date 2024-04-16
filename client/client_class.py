import argparse
import getpass
import socket
import threading
import colorama
from colorama import Fore, Style

colorama.init()

class Client:
    def __init__(self):
        self.client_socket = None
        self.username = None
        self.board = "-"
        
    def receive_handler(self):
        while True:
            msg = self.client_socket.recv(1024).decode()
            if not msg:
                break
            print(Fore.GREEN + msg + Style.RESET_ALL)
            print(f'[{self.board}]',end='',flush=True)

    def make_recv(self):
        threading.Thread(target = self.receive_handler).start()


    def setup(self, host='localhost', port=8080):
        self.client_socket = socket.socket()
        self.client_socket.connect((host,port))

        response = self.client_socket.recv(1024).decode()
        if (response =="CONNECTED"):
            print(Fore.GREEN + "[Connected Sucessfully]" + Style.RESET_ALL)
            return True
        else:
            print(Fore.RED + "[Connection Failed]" + Style.RESET_ALL)
            return False
        
    def login(self):
        self.username = input(Fore.CYAN + "[ Login Username ]::" + Style.RESET_ALL)
        self.client_socket.send(f'LOGIN {self.username}'.encode())
        response = self.client_socket.recv(1024).decode()

        if response == "SUCCESFULL":
            print(Fore.GREEN + "Logged into server as: " + self.username + Style.RESET_ALL)
            return True
        else:
            print(Fore.RED + "FAILED LOGIN" + Style.RESET_ALL)
            return False
        

    def runtime(self):
        cont = True
        print(f'Welcome to the Server!\n You are logged in as {self.username}\n'+ \
                  "Commands Availiable are: help, post, ret, users, groups, boards, join, gjoin, leave,  disc")
        while cont:
            
            
            message = input(f"[{self.board}]::")
            msg_string = None

            message = message.lower().strip()

            if message == 'post':
                sub = input("Subject:: ")
                con = input("Message:: ")
                msg_string = f'POST {sub}:{con}'
            elif message == 'users':
                msg_string = f'USERS '
            elif message == 'groups':
                msg_string = f'GROUPS '
            elif message == 'boards':
                msg_string = f'BOARD '
            elif message == 'join':
                bor = input("Board Name::")
                self.board = bor
                msg_string = f'JOIN {bor}'
            elif message == 'ret':
                mid = input("Message Id:: ")
                msg_string = f'RET {mid}'
            elif message == 'disc':
                msg_string = f'DISC '
                cont=False
            elif message == 'gjoin':
                bor = input("Board Group::")
                msg_string = f'GJOIN {bor}'
            elif message == 'leave':
                bor = input("Board Group::")
                msg_string = f'LEAVE {bor}'
            elif message == "help":
                print("Commands Availiable are: help, post, ret, users, groups, boards, join, gjoin, leave,  disc")
                msg_string = None
            else:
                print("Not a Valid Command")

            if msg_string is not None:
                self.client_socket.send(msg_string.encode())


    def close_connection(self):
        print(f'Goodbye {self.username}!')
        self.client_socket.close()


