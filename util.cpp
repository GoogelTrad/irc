/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   util.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmichez <cmichez@student.42nice.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 11:38:21 by kortolan          #+#    #+#             */
/*   Updated: 2024/03/02 19:31:59 by cmichez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

bool    isNickGood(std::string nick, Client *clientsInfo)
{
    if (nick.size() > 9)
    {
        send(clientsInfo->socket, "Max length for Nickname is 9 ! \n", 32, 0);
        return false;
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (nick.compare(clientsInfo[i].nickName) == 0)
        {
            send(clientsInfo->socket, "Nickname already used !\n", 24, 0);
            return false;
        }
    }
    return true;
}

bool    isUserGood(std::string user, Client *clientsInfo)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (user.compare(clientsInfo[i].userName) == 0)
        {
            send(clientsInfo->socket, "Username already used !\n", 24, 0);
            return false;
        }
    }
    return true;
}

void    command(const char* buffer, Client *clientsInfo, Client &clientInfo, std::vector<channel>&channels)
{
    std::cout << "command\n" << std::endl;
    std::cout << buffer << std::endl;
    if (strncmp(buffer, "JOIN ", 5) == 0)
    {
        ft_join(buffer + 5, clientInfo.socket, clientInfo, channels, clientsInfo);
    }
    else if (strncmp(buffer, "NICK ", 5) == 0)
    {
        std::string tmp = buffer + 5;
        std::string nick = tmp.substr(0, tmp.find("\n") - 1);
        if (isNickGood(nick, clientsInfo))
        {
            std::string newNickName = ":" + clientInfo.nickName + " NICK " + nick + "\r\n";
            clientInfo.nickName = nick;
            send(clientInfo.socket, newNickName.c_str(), newNickName.size(), 0);
        }
    }
    else if (strncmp(buffer, "USER ", 5) == 0)
    {
        std::string user = buffer + 5;
        if (isUserGood(user, clientsInfo))
        {
            std::string newUserName = ":" + clientInfo.userName + " USER " + user + "\r\n";
            clientInfo.userName = user;
        }
    }
    else if (strncmp(buffer, "QUIT", 4) == 0)
    {
        std::string quitMessage = ":" + clientInfo.nickName + " QUIT\r\n";
        if (clientInfo.currentChannel != -1)
        {
            for (size_t y = 0; y < channels.size(); y++)
            {
                if(ft_isInChannel(clientInfo, channels, y) == 1)
                {
                    for (size_t x = 0; x <= channels[y].fd.size(); x++)
                    {
                        if (channels[y].fd[x] == clientInfo.socket)
                            channels[y].fd.erase(channels[y].fd.begin() + x);
                        else 
                            send(channels[y].fd[x], quitMessage.c_str(), quitMessage.size(), 0);
                    }
                }
            }
        }
        close(clientInfo.socket);
        clearClientInfo(clientInfo);
    }
    else if (strncmp(buffer, "PRIVMSG ", 8) == 0)
    {
        std::string content;
        std::string channel_name;
        const std::string buff = buffer;
        parsePrivmsg(buff, content, channel_name);
        int x = 0;
        for (size_t i = 0; i < channels.size(); i++)
        {
            std::string str = channels[i].name;
            str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
            if (channel_name == str)
            {
                std::string message = ":" + clientInfo.nickName + " PRIVMSG " + channel_name + " :" + content + "\r\n";
                for (size_t j = 0; j < channels[i].fd.size(); j++)
                {
                    if (channels[i].fd[j] != clientInfo.socket)
                        send(channels[i].fd[j], message.c_str(), message.size(), 0);
                }
                x = 1;
                break;
            }
        }
        std::cout << x << std::endl;
        if (x == 0)
        {
            size_t clientsInfoSize = MAX_CLIENTS;
            for (size_t j = 0; j < clientsInfoSize; j++)
            {
                std::string str = clientsInfo[j].nickName;
                str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                
                str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
                if (str == channel_name)
                {
                    std::string message = ":" + clientInfo.nickName + " PRIVMSG " + channel_name + " " + content + "\r\n";
                    send(clientsInfo[j].socket, message.c_str(), message.size(), 0);
                    break;
                }
            }   
        }
    }
    else if (strncmp(buffer, "PART ", 5) == 0)
    {
        std::string buf = buffer;
        std::string tmp = buf.substr(5, std::string::npos - 4);
        std::string chanl = tmp.substr(0, tmp.find(" "));
        std::string msg =  ":" + clientInfo.nickName + " PART " + chanl + "\r\n";
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (chanl == channels[i].name)
            {
                for (size_t y = 0; y <= channels[i].fd.size(); y++)
                {
                    if (channels[i].fd[y] == clientInfo.socket)
                        channels[i].fd.erase(channels[i].fd.begin() + y);
                    else
                        send(channels[i].fd[y], msg.c_str(), msg.length(), 0);
                }
            }
        }
    }
    else if (strncmp(buffer, "MODE ", 5) == 0)
        parsMode(buffer, clientsInfo, channels, clientInfo); 
    else if (strncmp(buffer, "INVITE ", 7) == 0)
        parsInv(buffer, clientsInfo, channels, clientInfo);
    else if (strncmp(buffer, "TOPIC ", 5) == 0)
    {
        std::string topic = ft_topic(buffer);
        std::cout << "topic: " << topic << std::endl;
        
        std::string buf = buffer;
        std::string tmp = buf.substr(6, std::string::npos - 5);
        std::string chanl = tmp.substr(0, tmp.find(" "));
        
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (chanl == channels[i].name)
            {
                std::string topicMessage = ":" + clientInfo.nickName + " TOPIC " + channels[i].name + " :" + topic + "\r\n";
                if (channels[i].topRights == 0 || ft_isModo(&clientInfo, channels, chanl) == 1)
                {
                    channels[i].topiC = topic;
                    for (size_t y = 0; y < channels[i].fd.size(); y++)
                        send(channels[i].fd[y], topicMessage.c_str(), topicMessage.size(), 0);
                }
                else
                {
                    std::string msg = ":" +  chanl + " requires you to be moderator to change the topic \r\n"; 
                    for(size_t k = 0; k < channels[i].fd.size(); k++)
                    {
                        if (channels[i].fd[k] == clientInfo.socket)
                            send(clientInfo.socket, msg.c_str(), msg.length(), 0);
                    }
                }
            }
        }
    }
}



std::string ft_topic(const char  *buffer)
{
    std::string bufferStr = buffer;
    std::size_t pos = bufferStr.find(":");
    if (pos != std::string::npos)
    {
        std::string topic = bufferStr.substr(pos + 1);
        return (topic);
    }
    return "";
}


int     ft_invit(std::vector<channel>&channels,  Client &clientInfo, int i)
{
    for (unsigned long j = 0; j < channels[i].invites.size(); j++)
    {
        if (channels[i].invites[j].compare(clientInfo.nickName) == 0)
            return(1);
        
    }
    return(0);
}

void    ft_join(const char * buffer, int fd, Client &clientInfo, std::vector<channel>&channels, Client *clientsInfo)
{
    int x = 0;
    int y = 0;
    int z = 0;
    std::string tmp = buffer;
    std::string nameChannel;
    std::string pass = "";
    if (tmp.find(" ") == std::string::npos)
        nameChannel = tmp.substr(0, tmp.find("\n") - 1);
    else
    {
        nameChannel = tmp.substr(0, tmp.find(" "));
        std::string tmp2 = tmp.substr(nameChannel.length() + 1, tmp.npos - tmp.length() + 1);
        pass = tmp2.substr(0, tmp2.find("\n") - 1);
    }
    if (chanName(nameChannel) == -1)
    {
        send(clientInfo.socket, "Please verify channel creation Name rules \n", 44, 0);
        return;
    }
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (nameChannel == channels[i].name)
        {
            if (channels[i].fd.size() < channels[i].maxUser)
            {
                if ((channels[i].invMode == 0 || ft_invit(channels, clientInfo, i) == 1) && (channels[i].passMode == 0 || pass.compare(channels[i].passW) == 0))
                {
                    if (clientInfo.currentChannel != (int)i)
                    {
                        channels[i].fd.push_back(fd);
                        z = 1;
                    }    
                    clientInfo.currentChannel = i;
                }
                else
                    send(clientInfo.socket, "The channel either requires to be invited or a password to be joined ! \n", 73, 0);
            }
            x = 1;
            break;   
        }
        y++;
    }
    if (x == 0)
    {
        std::cout << "newchannel" << std::endl;
        channel newChannel;
        newChannel.name = nameChannel;
        newChannel.fd.push_back(fd);
        newChannel.modo.push_back(fd);
        clientInfo.currentChannel = y;
        newChannel.invMode = 0;
        newChannel.topRights = 0;
        newChannel.passMode = 0;
        newChannel.passW = "";
        newChannel.maxUser = 99;
        z = 1;
        newChannel.topiC = "";
        channels.push_back(newChannel);
    }
    if (z == 1)
    {   
        std::string joinMessage = ":" + clientInfo.nickName + " JOIN " + nameChannel + "\r\n";
        std::string joinTopicMsg = ":" + clientInfo.nickName + " TOPIC " + channels[clientInfo.currentChannel].name + " :" + channels[clientInfo.currentChannel].topiC + "\r\n";
        std::string isWho = "LIST " +  channels[clientInfo.currentChannel].name + "\r\n";
        for (size_t i = 0; i < channels[clientInfo.currentChannel].fd.size(); i++)
        {
            send(channels[clientInfo.currentChannel].fd[i], joinMessage.c_str(), joinMessage.size(), 0);   
            if (clientInfo.socket == channels[clientInfo.currentChannel].fd[i] && channels[clientInfo.currentChannel].topiC != "")
                send(channels[clientInfo.currentChannel].fd[i], joinTopicMsg.c_str(), joinTopicMsg.size(), 0);
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                send(clientsInfo[j].socket, isWho.c_str(), isWho.size(), 0);
            }
        }
    }
}


std::string receivedMsg(const char *buffer, Client &clientInfo)
{
    std::string buf = buffer;
    size_t tmp = buf.find(" ");
    std::string ret = "IRC 332 " + clientInfo.userName + " " + buf.substr(tmp + 1, std::string::npos - tmp);
    return ret;
}