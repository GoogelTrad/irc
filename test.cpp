/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kortolan <kortolan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 11:20:32 by kortolan          #+#    #+#             */
/*   Updated: 2024/02/11 11:20:32 by kortolan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
int serverSocket;

void parseNicknameAndUsername(const std::string& message, std::string &nickname, std::string &username) 
{
    std::size_t nickPos = message.find("NICK ");
    std::size_t userPos = message.find("USER ");

    if (nickPos != std::string::npos && userPos != std::string::npos) 
    {
        nickPos += 5;
        std::size_t nickEnd = message.find(' ', nickPos);
        if (nickEnd != std::string::npos) 
        {
            nickname = message.substr(nickPos, nickEnd - nickPos);
        }
        userPos += 5;
        username = message.substr(userPos);
    }
}
void closeClientSocket(int clientSocket) {
    std::cout << "clientSocket : " << clientSocket << std::endl;
    close(clientSocket);

}

void signalCtrlC(int signal)
{
    (void)signal;
    close(serverSocket);
    exit(1);
}

int main(int ac, char **av) 
{
    int  maxClients = MAX_CLIENTS;
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    Client clientInfo[MAX_CLIENTS];
    char buffer[BUFFER_SIZE];
    std::vector<channel> channels;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) 
    {
        std::cerr << "Error creating server socket" << std::endl;
        return 1;
    }
    signal(SIGINT, signalCtrlC);
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(6667);
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) < 0) 
    {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }
    
    for (int i = 0; i < maxClients; i++)
    {
        clientInfo[i].socket = 0;
    }
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        clientInfo[i].currentChannel = -1;
    }
    
    while (true) 
    {
        struct pollfd fds[MAX_CLIENTS + 1];
        fds[0].fd = serverSocket;
        fds[0].events = POLLIN;
        for (int i = 0; i < MAX_CLIENTS; ++i) 
        {
            if (clientInfo[i].socket > 0) {
                fds[i + 1].fd = clientInfo[i].socket;
                fds[i + 1].events = POLLIN;
            } else {
                fds[i + 1].fd = -1;
                fds[i + 1].events = 0;
            }
        }

        int activity = poll(fds, maxClients + 1, -1);
        if (activity < 0) 
        {
            std::cerr << "Error in poll" << std::endl;
            break;
        }
        /*
        if (clientSocketToClose != -1) {
            closeClientSocket(clientSocketToClose);
            clientSocketToClose = -1;
        }
        */
        if (fds[0].revents & POLLIN) 
        {
            int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
            if (newSocket < 0) 
            {
                std::cerr << "Error accepting connection" << std::endl;
                continue;
            }
            
            /*
            memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = recv(newSocket, buffer, BUFFER_SIZE, 0);
            //std::cout << buffer << std::endl;
            if (bytesRead < 0) 
            {
                std::cerr << "error recv" << std::endl;
            }
            */
            int index = -1;
            for (int i = 0; i < maxClients; ++i) 
            {
                if (clientInfo[i].socket == 0) 
                {
                    clientInfo[i].socket = newSocket;
                    index = i;
                    break;
                }
            }
            if (index == -1)
            {
                std::cerr << "Maximum clients reached, cannot accept more connections." << std::endl;
                close(newSocket);
            }
            std::cout << index << std::endl;
        }

        for (int i = 0; i < maxClients; ++i) 
        {
            if (clientInfo[i].socket > 0 && (fds[i + 1].revents & POLLIN)) 
            {
                memset(buffer, 0, BUFFER_SIZE);
                int bytesRead = recv(clientInfo[i].socket, buffer, BUFFER_SIZE, 0);
                if (bytesRead <= 0) 
                {
                    close(clientInfo[i].socket);
                    clientInfo[i].socket = 0;
                } else
                {
                    bool eof = false;
                    int index = 0;
                    while(buffer[index])
                    {
                        if (buffer[index] == '\n')
                        {
                            eof = true;
                            clientInfo[i].buffer_perso += buffer;
                        }
                        index++;
                    }
                    if(eof)
                    {
                        std::string addCariage = addCarRet(clientInfo[i].buffer_perso.c_str());
                        std::cout << "addcariage = " << addCariage << std::endl;
                        command(addCariage.c_str(), clientInfo, clientInfo[i], channels, av[ac - 1]);
                        clientInfo[i].buffer_perso.clear();
                    }
                    else
                    {
                        std::cout << "pas de bashslash" << std::endl;
                        clientInfo[i].buffer_perso += buffer;
                        std::cout << "buffer perso = " << clientInfo[i].buffer_perso << std::endl;
                    }
                }
            }
        }
    }
    // ping \ pong ??? dans le while inf si pas de reponse, close le socket;
    for (int i = 0; i < maxClients; ++i) 
    {
        if (clientInfo[i].socket > 0)
            close(clientInfo[i].socket);
    }
    close(serverSocket);
    return 0;
}