from flask import Flask, request, jsonify, render_template

import threading
import socket
import struct

import webbrowser
import os

import sys

import time




client_socket = socket.socket()
username = None
board = None
temp_messages = []

board_response = False
user_response = False
join_response = False


app = Flask(__name__)

@app.route('/', methods=['GET'])
def home():
    
    return render_template('index.html')

@app.route('/server.html', methods=['GET'])
def server():
    return render_template('server.html')

@app.route('/get_username', methods=['POST'])
def get_username():
    #print('IN GET USERNAME')
    data = request.get_json()
    username = data['username']
    #print(self.username)

    msgString = f'LOGIN {username}'
    
    #client_socket.send(msgString.encode())
    send(client_socket, msgString)

    #response = client_socket.recv(1024).decode()
    response = receive(client_socket)
    print(f'GOT RESPONSE FOR LOGIN')
    
    threading.Thread(target=receive_message).start()
    msgString = f'JOIN Public:'
    #client_socket.send(msgString.encode())
    send(client_socket, msgString)
    time.sleep(1)

    msgString = f'USERS '
    
    #client_socket.send(msgString.encode())
    send(client_socket, msgString)
    time.sleep(1)

    msgString = f'BOARD '
    
    #client_socket.send(msgString.encode())
    send(client_socket, msgString)
    time.sleep(1)
    

    if response != "SUCCESFULL":
        return jsonify({'status': 'error', 'message': response}), 200
    else:
        return jsonify({'status': 'success'}), 200
    
    



@app.route('/send_message', methods=['POST'])
def send_message():
    #print('IN SEND MESSAGE')
    data = request.get_json()
    message = data['message']
    subject = data['subject']

    messageString = f'POST {subject}:{message}'
    
    #client_socket.send(messageString.encode())
    send(client_socket, messageString)
    
    # Handle message construction and send to server here

    return jsonify({'status': 'success'}), 200



@app.route('/get_message', methods=['GET'])
def get_message():
    print('IN GET MESSAGE')
    # get message from where we are storign it
    parts = []
    if temp_messages:
        message = temp_messages.pop(0)
        print(message)
        parts = message.split('*')
        if parts[0] == '[SERVER]':
            return jsonify({'subject': parts[0], 'message': parts[1], 'username': 'none', 'msgid': 'none', 'date': 'none'})
        if parts[0] == 'USERS':
            return jsonify({'subject': parts[0], 'message': parts[1], 'username': 'none', 'msgid': 'none', 'date': 'none'})
        if parts[0] == 'BOARDS':
            return jsonify({'subject': parts[0], 'message': parts[1], 'username': 'none', 'msgid': 'none', 'date': 'none'})
        

    else:
        parts = ['none','none','none','none','none']

    return jsonify({'message': parts[4], 'subject': parts[3] , 'username':parts[1] , 'msgid':parts[0] , 'date': parts[2]}), 200
    

def receive_message():
    #print('STARTING TO LISTEN FOR MESSAGES')
    while True:
        
        #message = client_socket.recv(1024).decode()
        message = receive(client_socket)
    
        #client_socket.send("OK".encode())
        
        #print(message)
        if not message:
            break

        temp_messages.append(message)

def send(sock, message):

    message = message.encode('utf-8')

    length = struct.pack('!I', len(message))
    print(f'Sending {message} {length}')
    sock.sendall(length + message)


def receive(sock):
    length_bytes = sock.recv(4)
    length = struct.unpack('!I', length_bytes)[0]
    print(f'Getting message of len {length}')
    message = sock.recv(length)
    
    return message.decode('utf-8')

if __name__ == '__main__':
    client_socket.connect(('localhost', 8080))

    #response = client_socket.recv(1024).decode()
    response = receive(client_socket)

    print(response)

    app.run(debug=False, port=sys.argv[1])

    



