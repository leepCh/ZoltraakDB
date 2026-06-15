#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "./RESPParser/RespParser.hpp"
#include "./core/evalreq.hpp"
#include <sys/epoll.h>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CONNS 100

int acceptClientConnection(int serverFd)
{
    int clientSocket;
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof(clientAddress);

    if ((clientSocket = accept(serverFd, (struct sockaddr *)&clientAddress, &addrLen)) < 0)
    {
        perror("Unable to accept connection");
        exit(1);
    }
    return clientSocket;
}

int serverSetup()
{
    int server_fd;
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Binding the socket failed");
        exit(1);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("failed to listen");
        exit(1);
    }

    return server_fd;
}

int main()
{

    struct epoll_event serverEvent, clientEvents[MAX_CONNS];
    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor\n");

        return 1;
    }

    int server_fd = serverSetup();

    serverEvent.events = EPOLLIN;
    serverEvent.data.fd = server_fd;

    // Register server_fd in epoll now epoll listens for incoming connections.
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &serverEvent))
    {
        perror("Failed to add server file descriptor to epoll\n");
        close(epoll_fd);
        close(server_fd);
        exit(1);
    }

    cout << "Server listening on port " << PORT << endl;

    char buffer[BUFFER_SIZE + 1];

    int eventCount;

    while (1)
    {

       

        eventCount = epoll_wait(epoll_fd, clientEvents, MAX_CONNS, 300000);

        for (int i = 0; i < eventCount; i++)
        {

            int currFd = clientEvents[i].data.fd;

            if (currFd == server_fd)
            {
                // request for connection

                int clientFD = acceptClientConnection(server_fd);
                serverEvent.data.fd = clientFD;
                serverEvent.events = EPOLLIN;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientFD, &serverEvent))
                {
                    perror("Failed to add client file descriptor to epoll\n");
                    close(clientFD);
                    continue;
                }

                 cout << "\n[Server] Client connected!" << endl;
            }
            else  
            {
                // client has sent data

                int clientFD=currFd;

                ssize_t bytesread;
                bytesread = read(clientFD, buffer, BUFFER_SIZE);

                if(bytesread<=0){

                    //==0 means user has disconnected 

                    //<0 means there is an error

                    //delete the clientFd from epoll

                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,clientFD,NULL);
                    close(clientFD);
                    cout << "[Server] Client disconnected." << endl;
                    continue;
                }
                cout << "Read " << bytesread << " from client " << clientFD << endl;

                buffer[bytesread] = '\0';

                std::string dataInString(buffer);
                int bytesParsed = 0;

                RespResult result = RespParser::RequestDecode(dataInString, bytesParsed);

                

                if (!result.success)
                {
                    if (std::holds_alternative<std::string>(result.data))
                    {
                        cout << "[Parser Error]: " << std::get<std::string>(result.data) << endl;
                    }
                    write(clientFD, "-ERR parsing\r\n", 14);
                }
                else
                {
                    std::string response = RequestEvaluator::processAndEncode(result);

                    write(clientFD, response.data(), response.size());
                }

                memset(buffer, 0, BUFFER_SIZE + 1);
            }
        }

      
    }

    close(server_fd);
    return 0;
}