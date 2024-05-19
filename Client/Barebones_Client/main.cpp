#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    string ipAddress = "127.0.0.1";
    int port = 54000;

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
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
        WSACleanup();
        return 0;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // Set the socket to non-blocking mode
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    // Input loop
    char buf[4096];
    string userInput;
    int bytesReceived;
    int sendResult;

    do
    {
        cout << "> ";
        getline(cin, userInput);

        if (userInput.size() > 0)
        {
            // Send user input
            sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult == SOCKET_ERROR)
            {
                cerr << "Send failed" << endl;
                break;
            }
        }

        // Receive server response asynchronously
        do
        {
            bytesReceived = recv(sock, buf, sizeof(buf), 0);
            if (bytesReceived > 0)
            {
                cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
            }
        } while (bytesReceived > 0);

        // Check for error or connection closed
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != 0)
        {
            cerr << "Error in receive: " << error << endl;
            break;
        }

    } while (userInput.size() > 0);

    // Clean up
    closesocket(sock);
    WSACleanup();
    return 0;
}
