/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   util.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmichez <cmichez@student-42nice.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 11:38:21 by kortolan          #+#    #+#             */
/*   Updated: 2024/03/12 17:36:53 by cmichez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void    command(const char* buffer, Client *clientsInfo, Client &clientInfo, std::vector<channel>&channels, char *passw)
{
    //std::string a = buffer;
    //std::cout << a << std::endl;
    std::cout << "BUFFER : ";
    std::cout << buffer << std::endl;
    /*
    std::cout << "Socket : " << clientInfo.socket << std::endl;
    for (size_t i = 0; i < strlen(buffer); i++)
    {
        std::cout << buffer[i] << std::endl;
        printf("%d \n", buffer[i]);
    }
    */
    if (clientInfo.ok == 0 && strncmp(buffer, "USER", 4) != 0 && strncmp(buffer, "PASS", 4) != 0 && strncmp(buffer, "NICK", 4) != 0 && strncmp(buffer, "CAP", 3) != 0 && strncmp(buffer, "QUIT", 4) != 0)
        send(clientInfo.socket, "Please connect before using any other command \r\n", 49, 0);
    else
    {
        if (strncmp(buffer, "JOIN", 4) == 0)
            ft_join(buffer, clientInfo.socket, clientInfo, channels, clientsInfo);
        else if (strncmp(buffer, "CAP ", 4) == 0)
            parsCap(clientsInfo, passw, clientInfo, static_cast<std::string>(buffer));
        else if (strncmp(buffer, "NICK", 4) == 0)
            cmd_nick(buffer, clientInfo, clientsInfo);
        else if (strncmp(buffer, "USER", 4) == 0)
            cmd_user(buffer, clientInfo, clientsInfo);
        else if (strncmp(buffer, "PASS", 4) == 0)
            pass(buffer, clientInfo, passw);                  
        else if (strncmp(buffer, "QUIT", 4) == 0)
            cmd_quit(clientInfo, channels, buffer);
        else if (strncmp(buffer, "PRIVMSG", 7) == 0)
            privMsg(buffer, clientsInfo, channels, clientInfo);
        else if (strncmp(buffer, "PART", 4) == 0)
            cmd_part(buffer, channels, clientInfo);
        else if (strncmp(buffer, "MODE", 4) == 0)
            parsMode(buffer, clientsInfo, channels, clientInfo); 
        else if (strncmp(buffer, "INVITE", 6) == 0)
            parsInv(buffer, clientsInfo, channels, clientInfo);
        else if (strncmp(buffer, "TOPIC", 5) == 0)
            topic(buffer, channels, clientInfo);
        else if(strncmp(buffer, "KICK", 4) == 0)
            cmd_kick(clientInfo, channels, clientsInfo, buffer);
        else if(strncmp(buffer, "WHO", 3) == 0)
        {
            
        }
        else 
            send(clientInfo.socket, "UNKNOWN COMAND \r\n",  18, 0);
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
    std::cout << clientsInfo[0].nickName << std::endl;
    int x = 0;
    int y = 0;
    int z = 0;
    std::string tosend;
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        tosend = clientInfo.nickName + " JOIN : Not enough parameters \r\n";
        send(clientInfo.socket, tosend.c_str(), 32, 0);
        return;
    }
    std::string tmp = static_cast<std::string>(buffer).substr(5, std::string::npos);
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
    if (pass == "" && nameChannel.length() == 0)
    {
        tosend = clientInfo.nickName + " JOIN : Not enough parameters \r\n";
        send (clientInfo.socket, tosend.c_str(), tosend.length(), 0);
        return;
    }  
    if (chanName(nameChannel) == -1)
    {
        tosend = nameChannel + " :No such channel \r\n";
        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
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
                    if (ft_isInChannel(clientInfo, channels, i) == 0)
                    {
                        channels[i].fd.push_back(fd);
                        z = 1;
                    }
                    clientInfo.currentChannel = i;
                }
                else
                {
                    if (channels[i].invMode == 1 && ft_invit(channels, clientInfo, i) == 0)
                        tosend = nameChannel + " :Cannot join channel (+i)\r\n";
                    else if  (channels[i].passMode == 1 || pass.compare(channels[i].passW) != 0)
                        tosend = nameChannel + " :Cannot join channel (+k)\r\n";
                    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                }
            }
            else
            {
                tosend = nameChannel + " :Cannot join channel (+l)\r\n";
                send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
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
        std::string joinTopicMsg; 
        if (channels[clientInfo.currentChannel].topiC != "")
            joinTopicMsg = ":IRC 332 " + clientInfo.nickName + " " + channels[clientInfo.currentChannel].name + " " + channels[clientInfo.currentChannel].topiC + "\r\n";
        else
            joinTopicMsg = ":IRC 331 " + clientInfo.nickName + " " + channels[clientInfo.currentChannel].name + " :No topic is set\r\n" ;
        for (size_t i = 0; i < channels[clientInfo.currentChannel].fd.size(); i++)
        {
            send(channels[clientInfo.currentChannel].fd[i], joinMessage.c_str(), joinMessage.size(), 0);   
            //if (clientInfo.socket == channels[clientInfo.currentChannel].fd[i] && channels[clientInfo.currentChannel].topiC != "")
            if (clientInfo.socket == channels[clientInfo.currentChannel].fd[i])
            {
                std::string who = ft_whoAtJoin(channels[clientInfo.currentChannel], clientsInfo);
                std::string chanState = ft_chanstate(channels[clientInfo.currentChannel]);
                send(clientInfo.socket, who.c_str(), who.length(), 0);
                send(clientInfo.socket, chanState.c_str(), chanState.length(), 0);
                send(channels[clientInfo.currentChannel].fd[i], joinTopicMsg.c_str(), joinTopicMsg.size(), 0);
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
