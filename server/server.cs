using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Threading;
using System.Net.Sockets;
using System.Text;
using System.Net;
using System.Linq;

public class Message
{
    public int Id {get; set;}
    public string Sender {get; set;}
    public DateTime PostDate {get; set;}
    public string Subject {get; set;}
    public string Content {get; set;}

}

public class ChatServer
{
    private int m_port;
    //private int m_serverSocket;
    private List<Thread> m_clientThreads;
    private Mutex m_mutex ;
    private Dictionary<string, Socket> m_clients;
    private Dictionary<string, List<Message>> m_boards;
    private Dictionary<string, List<Tuple<string, bool>>> m_boardMembers;
    private Socket m_listener;

    private int message_id_counter;

    public ChatServer(int port)
    {
        //Consturctor
        //Initialize Global Variables
        m_port = port;
        //m_serverSocket = -1;
        message_id_counter = 0;
        //m_clientThreads = new List<Thread>();
        m_mutex = new Mutex();
        m_clients = new Dictionary<string, Socket>();
        m_boards = new Dictionary<string, List<Message>>();
        m_boardMembers = new Dictionary<string, List<Tuple<string, bool>>>();

        //Initialize ChatServer's Boards
        m_boards["Public:"] = new List<Message>();
        m_boardMembers["Public:"] = new List<Tuple<string, bool>>();
        for (int i=1; i<=5; ++i)
        {
            m_boards["Private" + i.ToString() + ":"] = new List<Message>();
            m_boardMembers["Private" + i.ToString() + ":"] = new List<Tuple<string, bool>>();
        }
    }

    ~ChatServer()
    {
        //Deconstructor
    }

    public void Send(Socket socket, string message)
    {
        byte[] messageBytes = Encoding.ASCII.GetBytes(message);
        byte[] lenthBytes = BitConverter.GetBytes(messageBytes.Length);
        Console.WriteLine("Sending Message" + message);

        socket.Send(lenthBytes.Concat(messageBytes).ToArray());
    }

    public string Receive(Socket socket)
    {
        byte[] lengthBytes = new byte[4];
        socket.Receive(lengthBytes, 0, 4, SocketFlags.None);
        int messageLength = BitConverter.ToInt32(lengthBytes, 0);

        byte[] messageBytes = new byte[messageLength];
        socket.Receive(messageBytes, 0, messageLength, SocketFlags.None);

        return Encoding.UTF8.GetString(messageBytes);
    }

    private void HandleClient(Socket clientSocket)
    {
        // Construct a Connection message and send to the Client
        byte[] messageBytes = Encoding.ASCII.GetBytes("CONNECTED");
        clientSocket.Send(messageBytes);
        //Send(clientSocket, "CONNECTED");

        //Set up a buffer lenght and client variables
        byte[] buffer = new byte[1024];
        string username = "None";
        string currentBoard = "None";

        while (true)
        {
            //Read incoming message from client and convert to string
            int received = clientSocket.Receive(buffer);
            string message = Encoding.UTF8.GetString(buffer, 0, received);
            //string message = Receive(clientSocket);
            Console.WriteLine(message);

            //Split the command from the content
            //TODO Error handle for missing command split
            string[] parts = message.Split(new char[] {' '}, 2);
            Console.WriteLine(parts);

            string command = parts[0];
            string content = parts[1];

            if (command == "LOGIN")
            {
                username = content;

                m_mutex.WaitOne();

                if (m_clients.ContainsKey(username))
                {
                    messageBytes = Encoding.ASCII.GetBytes("ERROR:Username already exists");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "ERROR");
                }
                else
                {
                    m_clients.Add(username, clientSocket);

                    Tuple<string, bool> c_tuple= new Tuple<string, bool>(username, false);

                    m_boardMembers["Public:"].Add(c_tuple);

                    Console.WriteLine("[CLIENT] Loggen in as : " + username);

                    messageBytes = Encoding.ASCII.GetBytes("SUCCESFULL");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "SUCCESFULL");

                    messageBytes = Encoding.ASCII.GetBytes("[SERVER]*New User Joined: " + username);

                    foreach (string user in m_clients.Keys)
                    {
                        if (user != username)
                        {
                            m_clients[user].Send(messageBytes);
                            //Send(m_clients[user], "[SERVER]*New User Joined: " + username);
                            
                        }
                    }

                }
                m_mutex.ReleaseMutex();

                

            }

        
            else if (command == "POST")
            {
                Message msg = new Message();
                msg.Id = message_id_counter++;
                msg.Sender = username;
                msg.PostDate = DateTime.Now;
                string[] msg_parts = content.Split(new char[] {':'}, 2);
                msg.Subject = msg_parts[0];
                msg.Content = msg_parts[1];

                m_mutex.WaitOne();

                if (currentBoard != "None")
                {
                    string messageString = "MSG*"+ msg.Id + 
                    "*" + msg.Sender + "*" + msg.PostDate + 
                    "*" + msg.Subject + "*" + msg.Content;

                    messageBytes = Encoding.ASCII.GetBytes(messageString);

                    foreach (Tuple<string,bool> user in m_boardMembers[currentBoard])
                    {
                        if (user.Item1 != username && user.Item2)
                        {
                            m_clients[user.Item1].Send(messageBytes);
                            //Send(m_clients[user.Item1], messageString);
                        }
                    }
                    m_boards[currentBoard].Add(msg);
                }
                else
                {
                    messageBytes = Encoding.ASCII.GetBytes("ERROR: You are not on a board.");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "ERROR");
                }
                m_mutex.ReleaseMutex();

            }
            else if (command == "RET")
            {
                int messageID = int.Parse(content);
                bool msgFound = false;
                
                m_mutex.WaitOne();
                foreach (Message msg in m_boards[currentBoard])
                {
                    if (msg.Id == messageID)
                    {
                        string messageString = "MSG*"+ msg.Id + 
                            "*" + msg.Sender + "*" + msg.PostDate + 
                            "*" + msg.Subject + "*" + msg.Content;

                        messageBytes = Encoding.ASCII.GetBytes(messageString);
                        clientSocket.Send(messageBytes);
                        //Send(clientSocket, messageString);

                        msgFound = true;
                    }
                }

                if (!msgFound)
                {
                    messageBytes = Encoding.ASCII.GetBytes("ERROR: Message not found");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "ERROR");
                }

                m_mutex.ReleaseMutex();
            }
            else if (command == "USERS")
            {
                m_mutex.WaitOne();

                string userList = "USERS*";

                foreach ( string user in m_clients.Keys)
                {
                    userList += user + "*";
                }
                userList = userList.Remove(userList.Length -1);
                messageBytes = Encoding.ASCII.GetBytes(userList);
                clientSocket.Send(messageBytes);
                //Send(clientSocket, userList);

                if (m_boardMembers.ContainsKey(currentBoard))
                {
                    userList = currentBoard + "MEMBERS*";
                    foreach ( Tuple<string, bool> user in m_boardMembers[currentBoard])
                    {
                        userList += user.Item1 + "*";
                    }
                    userList = userList.Remove(userList.Length -1);
                    messageBytes = Encoding.ASCII.GetBytes(userList);
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, userList);

                    
                }
                m_mutex.ReleaseMutex();


            }
            else if (command == "BOARD")
            {
                m_mutex.WaitOne();
                string boardList = "BOARDS*";
                foreach (string boardName in m_boards.Keys)
                {
                    boardList += boardName + "*";
                    
                }
                boardList = boardList.Remove(boardList.Length -1);
                messageBytes = Encoding.ASCII.GetBytes(boardList);
                clientSocket.Send(messageBytes);
                //Send(clientSocket, boardList);

                m_mutex.ReleaseMutex();

            }
            else if (command == "JOIN")
            {
                string boardName = content;

                m_mutex.WaitOne();

                if (m_boards.ContainsKey(currentBoard))
                {
                    if(m_boardMembers[currentBoard].Contains(new Tuple<string, bool>(username, true)))
                    {
                        int index = m_boardMembers[currentBoard].FindIndex(t => t.Equals(new Tuple<string, bool>(username, true)));
                        if (index != -1)
                        {
                            m_boardMembers[currentBoard][index] = new Tuple<string, bool>(username, false);
                        }
                    }
                }

                if ( m_boards.ContainsKey(boardName))
                {
                    bool partOfGroup = false;
                    foreach (Tuple<string, bool> user in m_boardMembers[boardName])
                    {
                        if(user.Item1 == username)
                        {   
                            partOfGroup = true;
                        }
                        
                    }
                    if (partOfGroup)
                    {
                        Tuple<string,bool> userTuple = new Tuple<string, bool>(username, false);
                        int index = m_boardMembers[boardName].FindIndex(t => t.Equals(userTuple));

                        if (index != -1)
                        {
                            userTuple = new Tuple<string, bool>(username, true);
                            m_boardMembers[boardName][index] = userTuple;

                            currentBoard = boardName;

                            messageBytes = Encoding.ASCII.GetBytes("[SERVER]*You are now logged into: " + boardName);
                            clientSocket.Send(messageBytes);
                            //Send(clientSocket, "[SERVER]*You are now logged into: " + boardName);

                            foreach (Tuple<string, bool> boardUser in m_boardMembers[boardName])
                            {
                                messageBytes = Encoding.ASCII.GetBytes("[SERVER]*User " + username + " Joined board " + boardName);

                                if (boardUser.Item1 != username && boardUser.Item2)
                                {
                                    m_clients[boardUser.Item1].Send(messageBytes);
                                    //Send(m_clients[boardUser.Item1], "[SERVER]*User " + username + " Joined board " + boardName);
                                }
                            }

                            if(m_boards[currentBoard].Count >=2)
                            {
                                Message msg1 = m_boards[currentBoard][m_boards[currentBoard].Count - 2];
                                Message msg2 = m_boards[currentBoard][m_boards[currentBoard].Count - 1];

                                string messageString = "MSG*"+ msg1.Id + 
                                    "*" + msg1.Sender + "*" + msg1.PostDate + 
                                    "*" + msg1.Subject + "*" + msg1.Content;
                                    
                                
                                messageBytes = Encoding.ASCII.GetBytes(messageString);
                                clientSocket.Send(messageBytes);
                                //Send(clientSocket, messageString);

                                messageString = "MSG*"+ msg2.Id + 
                                    "*" + msg2.Sender + "*" + msg2.PostDate + 
                                    "*" + msg2.Subject + "*" + msg2.Content;
                                    
                                
                                messageBytes = Encoding.ASCII.GetBytes(messageString);
                                clientSocket.Send(messageBytes);
                                //Send(clientSocket, messageString);
                            }
                            else
                            {
                                foreach (Message msgn in m_boards[currentBoard])
                                {
                                    string messageString = "MSG*"+ msgn.Id + 
                                        "*" + msgn.Sender + "*" + msgn.PostDate + 
                                        "*" + msgn.Subject + "*" + msgn.Content;
                                    
                                
                                    messageBytes = Encoding.ASCII.GetBytes(messageString);
                                    clientSocket.Send(messageBytes);
                                    //Send(clientSocket, messageString);
                                }
                            }
                        }
                        else
                        {
                            messageBytes = Encoding.ASCII.GetBytes("ERROR: You are already on this board");
                            clientSocket.Send(messageBytes);
                            //Send(clientSocket, "ERROR");
                        }
                    }
                    else
                    {
                        messageBytes = Encoding.ASCII.GetBytes("ERROR: You are not a part of the user group");
                        clientSocket.Send(messageBytes);
                        //Send(clientSocket, "ERROR");
                    }
                }
                else
                {
                    messageBytes = Encoding.ASCII.GetBytes("ERROR: Board does not Exist");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "ERROR");
                }
                m_mutex.ReleaseMutex();
            }
            else if (command == "GJOIN")
            {
                string groupName = content;

                m_mutex.WaitOne();
                if (m_boardMembers.ContainsKey(groupName))
                {
                    if (m_boardMembers[groupName].Contains(new Tuple<string, bool>(username, false)) || m_boardMembers[groupName].Contains(new Tuple<string, bool>(username, true)))
                    {
                        messageBytes = Encoding.ASCII.GetBytes("ERROR: Already a part of the group");
                        clientSocket.Send(messageBytes);
                        //Send(clientSocket, "ERROR");
                    }
                    else
                    {
                        m_boardMembers[groupName].Add(new Tuple<string, bool>(username, false));
                        messageBytes = Encoding.ASCII.GetBytes("[SERVER]*You joinded Board group: " + groupName);
                        clientSocket.Send(messageBytes);
                        //Send(clientSocket, "[SERVER]*You joinded Board group: " + groupName);
                    }
                }
                else
                {
                    messageBytes = Encoding.ASCII.GetBytes("ERROR: Board Group Does not EXIST");
                    clientSocket.Send(messageBytes);
                    //Send(clientSocket, "ERROR");
                }

                m_mutex.ReleaseMutex();


            }
            else if (command == "DISC")
            {
                m_mutex.WaitOne();

                foreach (string board in m_boardMembers.Keys)
                {
                    if( m_boardMembers[board].Contains(new Tuple<string, bool>(username, false)) || m_boardMembers[board].Contains(new Tuple<string, bool>(username, true)))
                    {
                        int index = m_boardMembers[board].FindIndex(t => t.Item1 == username && (t.Item2 == true || t.Item2 == false));
                        if (index != -1)
                        {
                            m_boardMembers[board].RemoveAt(index);
                        }
                    }
                }

                if (m_clients.ContainsKey(username))
                {
                    m_clients.Remove(username);
                }

                messageBytes = Encoding.ASCII.GetBytes("[SERVER]*User " + username + "Disconnected.");
            
                foreach ( string user in m_clients.Keys)
                {
                    m_clients[user].Send(messageBytes);   
                    //Send(m_clients[user], "[SERVER]*User " + username + "Disconnected.");   
                }

                m_mutex.ReleaseMutex();

                messageBytes = Encoding.ASCII.GetBytes("[SERVER]*You have disconnected from server.");
                clientSocket.Send(messageBytes);
                //Send(clientSocket, "[SERVER]*You have disconnected from server.");

                clientSocket.Shutdown(SocketShutdown.Both);
                clientSocket.Close();
                break;
            }
            else if (command == "LEAVE")
            {
                // Add function to leave a board group
                string groupName = content;

                m_mutex.WaitOne();

                if (m_boardMembers.ContainsKey(groupName))
                {
                    if(m_boardMembers[groupName].Contains(new Tuple<string, bool>(username, false)) || m_boardMembers[groupName].Contains(new Tuple<string, bool>(username, true)))
                    {
                        int index = m_boardMembers[groupName].FindIndex(t => t.Item1 == username && (t.Item2 == true || t.Item2 == false));
                        if (index != -1)
                        {
                            m_boardMembers[groupName].RemoveAt(index);
                        }
                    }   
                }
            }
            else
            {
                messageBytes = Encoding.ASCII.GetBytes("ERROR: Command is wrong");
                clientSocket.Send(messageBytes);
                //Send(clientSocket, "ERROR");
            }
        
        }

    }

    public void Setup()
    {
        m_listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

        IPEndPoint localEndPoint = new IPEndPoint(IPAddress.Any, m_port);

        try
        {
            {
                m_listener.Bind(localEndPoint);

                Console.WriteLine("[STARTUP] Server Starting Port: " + m_port);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine("Error Creatign or Binding Socket: " + e.ToString());
            
            throw;
        }

    }

    public void Run()
    {
        Thread _serverStateThread = new Thread(ServerState);
        _serverStateThread.Start();

        m_listener.Listen(10);

        while(true)
        {
            try
            {
                Socket clientSocket = m_listener.Accept();
                Console.WriteLine("[CONNECTION] Client Connected With Socket: " + clientSocket.RemoteEndPoint);

                Thread clientThread = new Thread(() => HandleClient(clientSocket));
                clientThread.Start();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error accepting client connection " + ex.Message);
                continue;
            }
        }

    }

    public void ServerState()
    {
        Console.WriteLine("Server State Not Set Up Yet");
    }


}

public class Program
{
    static void Main(string[] args)
    {
        const int port = 8080;
        ChatServer myServer = new ChatServer(port);
        myServer.Setup();
        myServer.Run();
    }
}