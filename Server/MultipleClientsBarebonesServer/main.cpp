#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        cerr << "Can't start Winsock, Err #" << wsResult << endl;
        return 0;
    }

    // Create socket
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET)
    {
        cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.s_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Tell Winsock the socket is for listening 
    listen(listening, SOMAXCONN);

    // Create a set to store connected clients
    fd_set master;
    FD_ZERO(&master);
    FD_SET(listening, &master);

    // Main server loop
    while (true)
    {
        fd_set copy = master;

        // Call select to check for any activity on sockets
        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

        // Check if there's incoming activity on the listening socket
        if (FD_ISSET(listening, &copy))
        {
            SOCKET client = accept(listening, nullptr, nullptr);
            if (client != INVALID_SOCKET)
            {
                // Add the new client socket to the master set
                FD_SET(client, &master);
                cout << "New client connected" << endl;
            }
        }

        // Loop through all the connected clients and check for activity
        for (int i = 0; i < master.fd_count; ++i)
        {
            SOCKET clientSocket = master.fd_array[i];
            if (clientSocket != listening && FD_ISSET(clientSocket, &copy))
            {
                char buf[4096];
                ZeroMemory(buf, sizeof(buf));

                // Receive data from client
                int bytesReceived = recv(clientSocket, buf, sizeof(buf), 0);
                if (bytesReceived <= 0)
                {
                    // Client disconnected
                    cout << "Client disconnected" << endl;
                    closesocket(clientSocket);
                    FD_CLR(clientSocket, &master);
                }
                else
                {
                    // Broadcast received message to all clients
                    for (int j = 0; j < master.fd_count; ++j)
                    {
                        SOCKET outSocket = master.fd_array[j];
                        if (outSocket != listening && outSocket != clientSocket)
                        {
                            send(outSocket, buf, bytesReceived, 0);
                        }
                    }
                }
            }
        }
    }

    // Cleanup winsock
    WSACleanup();
    return 0;
}
