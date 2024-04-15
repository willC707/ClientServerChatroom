import socket, threading, sys

from flask import Flask, render_template, request, jsonify, redirect, url_for
from flask_socketio import SocketIO, emit

clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

app = Flask(__name__)
socketio = SocketIO(app)

@app.route('/')
def home():
    return render_template('index.html')


@app.route('/server', methods=['GET'])
def server_page():
    print('LOADING SERVER PAGE')
    return render_template('server.html')

@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data['username']

    print('SENDING LOGIN MESSAGE')
    message = 'LOGIN ' + username
    clientSocket.send(message.encode('utf-8'))

    print('WAINTING FOR RESPONSE')
    response = clientSocket.recv(1024).decode('utf-8')
    print(f'GOT RESPONSE {response}')

    threading.Thread(target=listen_for_message).start()

    if response == "SUCCESFULL":
        return jsonify({'status':'success', 'message': 'Login successful'})
    
    return jsonify({'status': 'Login attempt made'})

@app.route('/send_message', methods=['POST'])
def send_message():
    data = request.get_json()
    subject = data['subject']
    message = data['message']

    msgString = f'POST {subject}:{message}'
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/join_board', methods=['POST'])
def join_board():
    data = request.get_json()
    name = data['board_name']
    msgString = f'JOIN {name}'
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/disc_server', methods=['POST'])
def disc_server():
    data = request.get_json()
    msgString = f'DISC '
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/get_boards', methods=['POST'])
def get_boards():
    data = request.get_json()

    msgString = f'BOARD '
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/join_group', methods=['POST'])
def join_group():
    data = request.get_json()
    group = data['group_name']

    msgString = f'GJOIN {group}'
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/leave_group', methods=['POST'])
def leave_group():
    data = request.get_json()
    group = data['group_name']

    msgString = f'LEAVE {group}'
    clientSocket.send(msgString.encode('utf-8'))

    return jsonify({'message': msgString})

@app.route('/get_users', methods=['POST'])
def get_users():
    data = request.get_json()

    msgString = f'USERS '
    clientSocket.send(msgString.encode('utf-8'))
    return jsonify({'message': msgString})

@app.route('/get_group', methods=['POST'])
def get_group():
    data = request.get_json()

    msgString = f'GROUPS '
    clientSocket.send(msgString.encode('utf-8'))
    return jsonify({'message': msgString})

def create_socket_connection():
    host = 'localhost'
    port = 8080

    clientSocket.connect((host, port))

    data = clientSocket.recv(1024).decode('utf-8')
    if data == "CONNECTED":
        print("Connected to the server")
    else:
        print("Connection Failed")

def listen_for_message():
    while True:
        response = clientSocket.recv(1024).decode('utf-8')
        print(f'Received Message: {response}')

        parts = response.split('*')
        data = {}

        if parts[0] == "[SERVER]":
            # Notification
            data['type'] = 'notification'
            data['msg'] = parts[1]
        elif parts[0] == "USERS":
            # USER LIST
            data['type'] = 'user_list'
            data['users'] = parts[1:]

        elif parts[0] == "MEMBERS":
            # GROUP LIST
            data['type'] = 'group_list'
            data['groups'] = parts[1:]

        elif parts[0] == "BOARDS":
            # BOARD LIST
            data['type'] = 'board_list'
            data['boards'] = parts[1:]

        elif parts[0] == "MSG":
            #NEW MESSAGE
            data['type'] = 'new_message'
            data['id'] = parts[1]
            data['sender'] = parts[2]
            data['date'] = parts[3]
            data['subject'] = parts[4]
            data['msg'] = parts[5]

        else:
            print("ERROR MESSAGE")
            data['type'] = 'error'
        print(data)
        socketio.emit('server_response', {'data': data})



if __name__ == "__main__":
    create_socket_connection()
    # app.run(debug=True)
    socketio.run(app, debug=True, port=int(sys.argv[1]))
