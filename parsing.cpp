#include "server.hpp"

typedef struct userList
{
    std::string NICK;
    std::string USER;
    int			socket;
	int			currentChannel;
}               userList;

void clearClientInfo(Client &ClientInfo)
{
    ClientInfo.nickName.clear();
    ClientInfo.lastmessage.clear();
    ClientInfo.userName.clear();
    ClientInfo.nick = 0;
    ClientInfo.pass = 0;
    ClientInfo.user = 0;
    ClientInfo.ok = 0;
    ClientInfo.socket = 0;
}

void   sendToAllChannel(std::vector<channel> &channels, size_t j, std::string message)
{
    for (size_t y = 0; y < channels[j].fd.size(); y++)
        send(channels[j].fd[y], message.c_str(), message.size(), 0);
}

int ft_which_client(Client *clientsInfo, std::string usr)
{
    size_t i = 0;
    while (i < MAX_CLIENTS)
    {
        if (clientsInfo[i].nickName.compare(usr) == 0)
            return(i);
        i++;
    }
    return (-1);
}

int ft_isChannel(std::vector<channel>&channels, std::string argChanl)
{
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].name.compare(argChanl) == 0)
            return(i);
    }
    return(-1);
}

int ft_isInChannel(Client &clientInfo, std::vector<channel>&channels, size_t i)
{
    for (size_t j = 0; j < channels[i].fd.size(); j++)
    {
        if (clientInfo.socket == channels[i].fd[j])
            return(1);
    }
    return (0);
}

int ft_isModo(Client *clientInfo, std::vector<channel>&channels, std::string chanl)
{
    for (size_t j = 0; j < channels.size(); j++)
    {
        if (channels[j].name.compare(chanl) == 0)
        {
            for(size_t k = 0; k < channels[j].modo.size(); k++)
            {
                if (channels[j].modo[k] == clientInfo->socket)
                    return(1);
            }
        }
    }
    return(0);
}

std::string addCarRet(const char *buffer)
{
    std::string ret;
    if (strchr(buffer, 13) == NULL)
    {
        std::string buf = buffer;
        ret = buf.substr(0, buf.find(10)) + "\r" + "\n";
    }
    else
        ret = buffer;
    return(ret); 
}

//fonction pour récupérer que le nom du channel dans le message PRIVMSG #channelname :message
void parsePrivmsg(const std::string& message, std::string &content, std::string &channel_name) 
{
    std::size_t pos = message.find("PRIVMSG ");
    if (pos != std::string::npos) 
    {
        pos += 8;
        std::size_t end = message.find(' ', pos);
        std::string tmp;
        if (end != std::string::npos) 
        {
            channel_name = message.substr(pos, end - pos);
            tmp = message.substr(end + 1);
            content = tmp.substr(0, tmp.find("\n") -1);
        }
        else
            content = "";
    }
}

void    cmd_nick(const char *buffer, Client &clientInfo, Client *clientsInfo)
{
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send (clientInfo.socket, "NICK : Not enough parameters \r\n", 32, 0);
        return;
    }
    std::string tmp = buffer + 5;
    std::string nick = tmp.substr(0, tmp.find("\n") - 1);
    if (nick.length() == 0)
    {
        send (clientInfo.socket, "NICK : Not enough parameters \r\n", 32, 0);
        return;
    }
    if (clientInfo.nick != 1)
    {
        if (nick.length() > 9)
        {
            send(clientInfo.socket, "Max length for Nickname is 9 ! \r\n", 34, 0);
            return;
        }
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].nickName.compare(nick) == 0)
            {
                send(clientInfo.socket, "Nickname already in use please change \r\n", 41, 0);
                return;
            }
        }
        clientInfo.nickName = nick;
        clientInfo.nick = 1;
    }
    std::string newNickName = ":" + clientInfo.nickName + " NICK " + nick + "\r\n";
    ft_isOk(clientInfo, 0, newNickName, nick);
}

void    cmd_user(const char *buffer, Client &clientInfo, Client *clientsInfo)
{
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send (clientInfo.socket, "USER : Not enough parameters \r\n", 32, 0);
        return;
    }
    std::string tmp = buffer + 5;
    std::string user = tmp.substr(0, tmp.find("\n") - 1);
    if (user.length() == 0)
    {
        send (clientInfo.socket, "USER : Not enough parameters \r\n", 32, 0);
        return;
    }
    if (clientInfo.user != 1)
    {
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].userName.compare(user) == 0)
            {
                send(clientInfo.socket, "Username already in use please change \r\n", 41, 0);
                return;
            }
        }
        clientInfo.userName = user;
        clientInfo.user = 1;
    }
    std::string newUserName = ":" + clientInfo.userName + " USER " + user + "\r\n";
    ft_isOk(clientInfo, 1, newUserName, user);
}

void topic(const char *buffer, std::vector<channel>&channels, Client &clientInfo)
{
        
        std::string topic;
        std::string buf = buffer;
        std::string tmp = buf.substr(6, std::string::npos - 6);
        if (buf.find(" ") == buf.npos)
        {
            send (clientInfo.socket, "TOPIC : Not enough parameters \r\n", 33, 0);
            return;
        }
        std::string chanl;
        if (tmp.find(" ") != tmp.npos)
        {
            chanl = tmp.substr(0, tmp.find(" "));
            std::string tmp2 = tmp.substr(tmp.find(" "));
            topic = tmp2.substr(1, tmp2.find("\n") - 1);
        }
        else
            chanl = tmp.substr(0, tmp.find("\n") - 1);
        if (topic.find(":") == 0)
            topic = topic.substr(1);
        std::string tosend;
        std::cout << "topic :" << topic << topic.length() << std::endl;
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (chanl == channels[i].name)
            {
                if (!ft_isInChannel(clientInfo, channels, i))
                {
                    tosend = chanl + ":You're not on that channel \r\n";
                    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);;
                    return;
                }
                if ((topic.length() == 1 && topic.find(" ") == 0) || topic.length() == 0)
                {
                    if (channels[clientInfo.currentChannel].topiC == "")
                        tosend = ":IRC 331 " + clientInfo.nickName + " " + chanl + " :No topic is set\r\n";
                    else
                        tosend = ":IRC 332 " + clientInfo.nickName + " " + chanl + " " + channels[clientInfo.currentChannel].topiC + "\r\n";
                    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                    return;
                }
                std::string topicMessage = ":" + clientInfo.nickName + " TOPIC " + channels[i].name + " :" + topic + "\r\n";
                if (channels[i].topRights == 0 || ft_isModo(&clientInfo, channels, chanl) == 1)
                {
                    channels[i].topiC = topic;
                    for (size_t y = 0; y < channels[i].fd.size(); y++)
                        send(channels[i].fd[y], topicMessage.c_str(), topicMessage.size(), 0);
                }
                else
                {
                    std::string msg = chanl + " :You're not channel operator \r\n"; 
                    for(size_t k = 0; k < channels[i].fd.size(); k++)
                    {
                        if (channels[i].fd[k] == clientInfo.socket)
                            send(clientInfo.socket, msg.c_str(), msg.length(), 0);
                    }
                }
            }
        }
}

void cmd_kick(Client &clientInfo, std::vector<channel>&channels, Client *clientsInfo, const char *buffer)
{
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send(clientInfo.socket, "KICK : Not enough parameters \r\n", 32, 0);
        return;
    }
    std::string tmp = buffer + 5;
    if (tmp.find(" ") == tmp.npos)
    {
        send(clientInfo.socket, "KICK : Not enough parameters \r\n", 32, 0);
        return;
    }
    std::string chanl = tmp.substr(0, tmp.find(" "));
    size_t toget = tmp.find("\n") -1;
    std::string usr;
    std::string tmp2 = tmp.substr(chanl.length() + 1, tmp.npos - chanl.length() - 1);
    std::string reason;
    if (tmp2.find(" ") == tmp2.npos)
        usr = tmp.substr(chanl.length() + 1, toget - chanl.length() - 1);
    else
    {
        usr = tmp2.substr(0, tmp2.find(" "));
        toget = tmp2.find("\n") - 1;
        reason = tmp2.substr(usr.length() + 1, toget - usr.length() - 1);
    }
    std::cout << "Chanl : " << chanl << chanl.length() << std::endl;
    std::cout << usr.length() << std::endl;
    std::cout << "Usr : " << usr << std::endl;
    std::cout << "reason : " << reason << reason.length() << std::endl;
    if (usr.length() == 0)
    {
        send(clientInfo.socket, "KICK : Not enough parameters \r\n", 32, 0);
        return;
    }
    std::string tosend;
    int ichan = ft_isChannel(channels, chanl);
    if (ichan == -1)
        tosend = chanl + " :No such nick/channel \r\n";
    else if (ft_isInChannel(clientInfo, channels, ichan) == 0)
        tosend = chanl + " :You're not on that channel \r\n";
    else if (ft_isModo(&clientInfo, channels, chanl) == 0)
        tosend = chanl + " :You're not channel operator \r\n";
    else
    {
        int cl = ft_which_client(clientsInfo, usr);
        if (cl != -1)
        {
            if (ft_isInChannel(clientsInfo[cl], channels, ichan) == 1)
            {
                if (reason.length() == 0)
                    tosend = ":" + clientInfo.nickName + " KICK " + chanl + " " + usr + "\r\n";
                else
                    tosend = ":" + clientInfo.nickName + " KICK " + chanl + " " + usr + " " + reason + "\r\n";
                for (size_t i = 0; i < channels[ichan].fd.size(); i++)
                {
                    if (clientsInfo[cl].socket == channels[ichan].fd[i])
                    {
                        send(clientsInfo[cl].socket, tosend.c_str(), tosend.length(), 0);
                        channels[ichan].fd.erase(channels[ichan].fd.begin() + i);
                        if(channels[ichan].fd.size() == 0)
                            channels.erase(channels.begin() + ichan);
                    }
                }
                sendToAllChannel(channels, ichan, tosend);
            }
            return;
        }
    }
    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
}

std::string ft_whoAtJoin(channel &channel, Client *clientsInfo)
{
    std::string nicks = ":IRC 352 " + channel.name + " Users in chan :";
    for (size_t i = 0; i < channel.fd.size(); i++)
    {
        for(size_t j = 0; j < MAX_CLIENTS; j++)
        {
            if (clientsInfo[j].socket == channel.fd[i])
                nicks += clientsInfo[j].nickName + " ";
        }
    }
    nicks += " \r\n";
    return (nicks);
}

std::string ft_chanstate(channel &channel)
{
    std::string chanState = channel.name + ": CHANNEL MODE :\n-topicModoOnly : ";
    if (channel.topRights == true)
        chanState += "ON";
    else
        chanState += "OFF";
    chanState += "\r\n-invMode : ";
    if (channel.invMode == true)
        chanState += "ON";
    else
        chanState += "OFF";
    chanState += "\r\n-usrLimit : ";
    if (channel.maxUser != 99)
        chanState += "ON";
    else
        chanState += "OFF";
    chanState += "\r\n-passWord : ";
    if (channel.passMode == true)
        chanState += "ON : " + channel.passW;
    else
        chanState += "OFF";
    chanState += " \r\n";
    return (chanState);
}

void cmd_quit(Client &clientInfo, std::vector<channel>&channels, const char *buffer)
{        
    if (clientInfo.ok == 1)
    {
        std::string buf = buffer;
        std::string msg;
        std::string quitMessage;
        if (buf.find(" ") != buf.npos)
            msg = buf.substr(5, buf.find("\n") - 1 - 5);
        if (msg.length() == 0)
            quitMessage = ":" + clientInfo.nickName + " QUIT\r\n";
        else
            quitMessage = ":" + clientInfo.nickName + " QUIT :" + msg + "\r\n";
        std::cout << "msg : " <<  msg << msg.length() << std::endl;
        if (clientInfo.currentChannel != -1)
        {
            for (size_t y = 0; y < channels.size(); y++)
            {
                if(ft_isInChannel(clientInfo, channels, y) == 1)
                {
                    for (size_t x = 0; x <= channels[y].fd.size(); x++)
                    {
                        send(channels[y].fd[x], quitMessage.c_str(), quitMessage.size(), 0);
                        if (channels[y].fd[x] == clientInfo.socket)
                        {
                            channels[y].fd.erase(channels[y].fd.begin() + x);
                            if(channels[y].fd.size() == 0)
                                channels.erase(channels.begin() + y);
                        }
                    }
                }
            }
        }
    }
    close(clientInfo.socket);
    clearClientInfo(clientInfo);
}

void parsInv(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo)
{
    std::string buf = buffer;
    std::string tmp = buf.substr(7, std::string::npos - 7);
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send (clientInfo.socket, "USER : Not enough parameters \r\n", 32, 0);
        return;
    }
    if (buf.find(" ") == buf.npos || tmp.find(" ") == tmp.npos)
    {
        send (clientInfo.socket, "INVITE : Not enough parameters \r\n", 34, 0);
        return;
    }
    std::string nickToInv = tmp.substr(0, tmp.find(" "));
    std::string tmp2 = tmp.substr(nickToInv.length() + 1);
    if (tmp2.length() == 2)
    {
        send (clientInfo.socket, "INVITE : Not enough parameters \r\n", 34, 0);
        return;
    }
    std::string chanl = tmp2.substr(0, tmp2.find("\n") - 1);
    std::string tosend;
    bool userX = false;
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientsInfo[i].nickName.compare(nickToInv) == 0)
        {
            userX = true;
            for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].name.compare(chanl) == 0)
                {
                    if (!ft_isInChannel(clientInfo, channels, j))
                    {
                        tosend = chanl + ":You're not on that channel \r\n";
                        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                        return;
                    }
                    else if (ft_isInChannel(clientsInfo[i], channels, j))
                    {
                        tosend = nickToInv + " " + chanl + " :is already on channel \r\n";
                        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                        return;
                    }
                    else if (channels[j].invMode == 1 && !ft_isModo(&clientInfo, channels, chanl))
                    {
                        tosend = chanl + ":You're not channel operator \r\n";
                        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                        return;
                    }
                    else if (channels[j].invMode == 0 || ft_isModo(&clientInfo, channels, chanl))
                    {
                        tosend = ":" + clientInfo.nickName + " INVITE " + nickToInv + " " + chanl + "\r\n";
                        send(clientsInfo[i].socket, tosend.c_str(), tosend.length(), 0);
                        if (channels[j].invMode == 1)
                            channels[j].invites.push_back(clientsInfo[i].nickName);
                        return;
                    }
                }
            }
        }
    }
    if (userX == false)
        tosend = nickToInv + " :No such nick/channel \r\n";
    else
        tosend = chanl + " :No such nick/channel \r\n";
    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
    return;
}

int passwCheck(char *av, char *buffer)
{
    char * tmp = strstr(buffer, "PASS");
    if (tmp == NULL)
        return (-1);
    char *ptr = tmp;
    if (strncmp(ptr + 5, av, strlen(av) )!= 0)
        return(-1);
    return(1);
}



void pass(const char *buffer, Client &clientInfo, char *passw)
{
    std::string buf = buffer;
    std::string tmp = buf.substr(5, std::string::npos - 5);
    std::string arg = tmp.substr(0, tmp.find("\n") - 1);
    if (clientInfo.ok == 1)
    {
        std::string errAlreadyRegistered = clientInfo.nickName + ":" + " You may not reregister \r\n";
        ft_isOk(clientInfo, 462, errAlreadyRegistered, "");
        return;
    }
    else if (clientInfo.ok == 0 && clientInfo.pass == 1)
    {   
        send(clientInfo.socket, "PASS : You already entered the pass \r\n", 39, 0);
        return;
    }
    if (arg.length() == 0 || strncmp(buffer, "PASS ", 5) != 0)
    {
        send(clientInfo.socket, "PASS : Not enough parameters \r\n", 32, 0);
        return;
    }
    else if (arg.compare(passw) != 0)
    {
        send(clientInfo.socket, "PASS : Wrong password \r\n", 25, 0);
        return;
    }
    if (clientInfo.pass != 1)
    {
        if (arg.compare(passw) == 0)
        {
            clientInfo.pass = 1;
            ft_isOk(clientInfo, 666, "", "");
            return;
        }
    }
}

void privMsg(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo)
{
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send (clientInfo.socket, "PRIVMSG : Not enough parameters \r\n", 35, 0);
        return;
    }
    if (clientInfo.ok == 1)
    {
        std::string content;
        std::string channel_name;
        const std::string buff = buffer;
        parsePrivmsg(buff, content, channel_name);
        int x = 0;
        if (content.length() == 0)
            send(clientInfo.socket, ":No text to send \r\n", 20, 0);
        else
        {
            for (size_t i = 0; i < channels.size(); i++)
            {
                std::string str = channels[i].name;
                str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
                if (channel_name == str)
                {
                    if (ft_isInChannel(clientInfo, channels, i))
                    {
                        std::string message = ":" + clientInfo.nickName + " PRIVMSG " + channel_name + " :" + content + "\r\n";
                        for (size_t j = 0; j < channels[i].fd.size(); j++)
                        {
                            if (channels[i].fd[j] != clientInfo.socket)
                                send(channels[i].fd[j], message.c_str(), message.size(), 0);
                        }
                        x = 1;
                    }
                    else
                    {
                        std::string err = channel_name + " : You're not on that channel \r\n";
                        send(clientInfo.socket, err.c_str(), err.length(), 0);
                        return;
                    }
                    break;
                }
            }
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
                        x = 2;
                        break;
                    }
                }   
                if (x != 2)
                {
                    std::string err = channel_name + " :No such nick/chanl \r\n";
                    send(clientInfo.socket, err.c_str(), err.length(), 0);
                }
            }
        }
    }
}

void parsMode(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo)
{
    std::string buf = buffer;
    std::string tmp = buf.substr(5, std::string::npos - 5);
    std::string chanl = tmp.substr(0, tmp.find(" "));
    std::string tosend;
    if (buf.find(" ") == buf.npos)
    {
        tosend = clientInfo.nickName + "MODE : Not enough parameters \r\n";
        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
        return;
    }
    if (tmp.find(" ") == buf.npos)
    {
        chanl = tmp.substr(0, tmp.find("\n") -1);
        if (ft_isChannel(channels, chanl) ==  -1)
        {
            tosend = chanl + " :No such nick/channel \r\n";
            send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
        }
        return;
    }
    if (ft_isModo(&clientInfo, channels, chanl) == 0)
    {            
        for (size_t j = 0; j < channels.size(); j++)
        {        
            for(size_t k = 0; k < channels[j].fd.size(); k++)
            {
                if (channels[j].fd[k] == clientInfo.socket)
                {
                    tosend = chanl + " :You're not channel operator \r\n";
                    send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                }
            }
        }
        return;
    }
    std::string tmp2 = tmp.substr(chanl.length() + 1, tmp.npos - chanl.length() + 1);
    std::string flag = tmp2.substr(0, 2);
    std::string tmp3;
    if (tmp2.size() > 2)    
        tmp3 = tmp2.substr(flag.length() + 1, tmp2.npos - flag.length() + 1);
    std::string argPostFlag;
    if (flag.compare("+i") == 0)
    {
    //  InviteOnlyON
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].invMode == 1)
                    return;
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " +  chanl + " +i\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].invMode = 1;
                    return;
                }
            }
        }
    }
    if (flag.compare("-i") == 0)
    {
        //  InviteOnlyOFF
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].invMode == 0)
                    return;
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " -i\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].invMode = 0;
                    channels[j].invites.clear();
                    return;
                }
            }
        }
    }
    
    if (flag.compare("+t") == 0)
    {
        //topicModoOnlyON
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].topRights == 1)
                    return;
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " +t\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].topRights = 1;
                    return;
                }
            }
        }
    }
    if (flag.compare("-t") == 0)
    {
        //topicModoOnlyOFF
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].topRights == 0)
                    return;
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " -t\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].topRights = 0;
                    return;
                }
            }
        }
    }
    if (flag.compare("+k") == 0)
    {
        //chanHasPASS
        argPostFlag = tmp3.substr(0, tmp3.find("\n") - 1);
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].passMode == 1)
                {
                    channels[j].passW = argPostFlag;
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " +k " + argPostFlag + "\r\n";
                    sendToAllChannel(channels, j, tosend);
                    return;
                }  
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " +k " + argPostFlag + "\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].passMode = 1;
                    channels[j].passW = argPostFlag;
                    return;
                }
            }
        }
    }
    if (flag.compare("-k") == 0)
    {
        //removeChanPASS
        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j].name.compare(chanl) == 0)
            {
                if (channels[j].passMode == 0)
                    return;
                else
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " -k\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].passMode = 0;
                    channels[j].passW = "";
                    return;
                }
            }
        }
    }
    if (flag.compare("+o") == 0)
    {
        //giveModoRIGHTS
        argPostFlag = tmp3.substr(0, tmp3.find("\n") - 1);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].nickName.compare(argPostFlag) == 0)
            {
                for (size_t j = 0; j < channels.size(); j++)
                {
                    if (channels[j].name.compare(chanl) == 0)
                    {
                        std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " +o " + argPostFlag + " \r\n";
                        sendToAllChannel(channels, j, tosend);
                        channels[j].modo.push_back(clientsInfo[i].socket);
                        return;
                    }
                }
            }
        }
        return;
    }
    if (flag.compare("-o") == 0)
    {
        //REMOVEModoRIGHTS
        argPostFlag = tmp3.substr(0, tmp3.find("\n") - 1);
        for (size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].nickName.compare(argPostFlag) == 0)
            {
                for (size_t j = 0; j < channels.size(); j++)
                {
                    if (channels[j].name.compare(chanl) == 0)
                    {
                        for(size_t k = 0; k < channels[j].modo.size(); k++)
                        {
                            if (channels[j].modo[k] == clientsInfo[i].socket)
                            {
                                std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " -o " + argPostFlag + " \r\n";
                                sendToAllChannel(channels, j, tosend);
                                channels[j].modo[k] = 0;
                                return;
                            }
                        }
                    }
                }
            }
        }
        return;
    }
    if (flag.compare("+l") == 0)
    {
        //chanMaxUserON
        argPostFlag = tmp3.substr(0, tmp3.find("\n") - 1);
        int nbOfMaxUser = std::atoi(argPostFlag.c_str());
        if (nbOfMaxUser < 1)
            return ;
        else
        {
            for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].name.compare(chanl) == 0)
                {
                    std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " +l " + argPostFlag + "\r\n";
                    sendToAllChannel(channels, j, tosend);
                    channels[j].maxUser = nbOfMaxUser;
                    return;
                }
            }
        }
    }
    if (tmp.find("-l") != std::string::npos)
    {
        //chanMaxUserOFF
        for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].name.compare(chanl) == 0)
                {
                    if (channels[j].maxUser == 99)
                        return;
                    else
                    {
                        std::string tosend = ":" + clientInfo.nickName + " MODE " + chanl + " -l\r\n";
                        sendToAllChannel(channels, j, tosend);
                        channels[j].maxUser = 99;
                    }
                }
            }
        return;
    }
    return;
}

void cmd_part(const char *buffer, std::vector<channel>&channels, Client &clientInfo)
{
    
    std::string tosend;
    if (static_cast<std::string>(buffer).find(" ") == std::string::npos)
    {
        send (clientInfo.socket, "PART : Not enough parameters \r\n", 32, 0);
        return;
    }
    if (clientInfo.ok == 1)
    {
        int sent = 0;
        std::string buf = buffer;
        std::string tmp = buf.substr(5, std::string::npos - 5);
        std::string chanl;
        if (tmp.find(" ") == tmp.npos)
            chanl = tmp.substr(0, tmp.find("\n") - 1);
        else
            chanl = tmp.substr(0, tmp.find(" "));
        std::string msg =  ":" + clientInfo.nickName + " PART " + chanl + "\r\n";
        if (chanl.length() == 0)
        {
            send(clientInfo.socket, "PART : Not enough parameters \r\n", 32, 0);
            return;
        }
        if (sent != 1)
        {
            int noChan = 0;
            for (size_t i = 0; i < channels.size(); i++)
            {
                if (chanl == channels[i].name)
                {
                    if (ft_isInChannel(clientInfo, channels, i) != 1)
                    {
                        tosend = chanl + " :You're not on that channel \r\n";
                        send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
                    }
                    else
                    {
                        for (size_t y = 0; y <= channels[i].fd.size(); y++)
                        {
                            send(channels[i].fd[y], msg.c_str(), msg.length(), 0);
                            if (channels[i].fd[y] == clientInfo.socket)
                            {
                                channels[i].fd.erase(channels[i].fd.begin() + y);
                                if(channels[i].fd.size() == 0)
                                    channels.erase(channels.begin() + i);
                            }
                        }
                    }
                    noChan = 1;
                }
            }
            if (noChan != 1)
            {
                tosend = chanl + " :No such channel \r\n";
                send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
            }
        }
    }
}


int ft_isOk(Client &clientInfo, int fct, std::string tosend, std::string arg)
{   
    if (clientInfo.ok != 1)
    {
        if (clientInfo.nick != 1 || clientInfo.pass != 1 || clientInfo.user != 1)
        {
            if (clientInfo.nick == 0)
                send(clientInfo.socket, "Use NICK command to set up a nickname \r\n", 41, 0);
            else if (clientInfo.pass == 0)
                send(clientInfo.socket, "Use PASS command to connect with the good password \r\n", 54, 0);
            else if (clientInfo.user == 0)
                send(clientInfo.socket, "Use USER command to set up a username \r\n", 41, 0);
        }
        else if(clientInfo.nick == 1 && clientInfo.pass == 1 && clientInfo.user == 1)
        {
            clientInfo.ok = 1;
            if (fct == -1)
                send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
            else
            {
                tosend = clientInfo.nickName + " Welcome to the IRC server \r\n";
                send(clientInfo.socket, tosend.c_str(), tosend.length(), 0);
            }
        }
    }
    else if (clientInfo.ok == 1)
    {
        if (fct == 0)
            clientInfo.nickName = arg;
        else if (fct == 1)
            clientInfo.userName = arg;
        send(clientInfo.socket, tosend.c_str(), tosend.size(), 0);
            
    }
    return(clientInfo.ok);
}

int availableNick(Client *clientsInfo, std::string nick)
{
    int valid = 0;
    if (nick.empty() != 1)
    {
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].nickName.compare(nick) == 0)
                valid = -1;
        }
    }
    return(valid);
}

int availableUser(Client *clientsInfo, std::string user)
{
    int valid = 0;
    if (user.empty() != 1)
    {
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientsInfo[i].userName.compare(user) == 0)
                valid = -1;
        }
    }
    else
        return(-1);
    return(valid);
}

void    parsCap(Client *clientsInfo, char *passw, Client &clientInfo, std::string buf)
{
    if (buf.find("PASS ") != std::string::npos)
    {
        std::string passarg = passw;
        std::string tmp = buf.substr(buf.find("PASS ") + 5, std::string::npos - buf.find("PASS "));
        std::string pass = tmp.substr(0, tmp.find("\n") - 1);
        if (passarg.compare(pass) != 0)
        {
            send(clientInfo.socket, "The password is false \r\n", 25, 0);
            close(clientInfo.socket);
            clientInfo.socket = 0;
            return;
        }
        if (tmp.find("NICK ") != std::string::npos)
        {
            std::string tmp2 = tmp.substr(tmp.find("NICK ") + 5, std::string::npos - tmp.find("NICK "));
            std::string nick = tmp2.substr(0, tmp2.find("\n") - 1);
            if (nick.length() > 9)
            {
                send(clientInfo.socket, "Your nickname must be less than 9 char length \r\n", 49, 0);
                close(clientInfo.socket);
                clientInfo.socket = 0;
                return;
            }
            if(availableNick(clientsInfo, nick) != 0)
            {
                send(clientInfo.socket, "Your nickname is already in use, please pick an other one \r\n", 61, 0);
                close(clientInfo.socket);
                clientInfo.socket = 0;
                return;
            }
            if (tmp2.find("USER ") != std::string::npos)
            {
                std::string tmp3 = tmp2.substr(tmp2.find("USER " ) + 5, std::string::npos - tmp2.find("USER "));
                std::string user = tmp3.substr(0, tmp3.find("0") - 1);
                if(availableUser(clientsInfo, user) == -1)
                {
                    send(clientInfo.socket, "Your username is already in use, please pick an other one \r\n", 61, 0);
                    close(clientInfo.socket);
                    clientInfo.socket = 0;
                    return;
                }
                clientInfo.nick = 1;
                clientInfo.nickName = nick;
                clientInfo.user = 1;
                clientInfo.userName = user;
                clientInfo.pass = 1;
                std::string connected = clientInfo.nickName + " Welcome to the IRC server \r\n";
                ft_isOk(clientInfo, -1, connected, "");
            }
        }
    }
    else
    {
        send(clientInfo.socket, "A PASSWORD IS REQUIERED TO CONNECT \r\n", 38, 0);
        close(clientInfo.socket);
        clientInfo.socket = 0;
        return;
    }
}
/*
int availableUserNickName(Client *clientInfo, std::string nick, std::string user)
{
    char *tmp = strstr(buffer, "NICK");
    int valid = 0;
    if (nick.empty() != 1) 
    {
        if (nick.length() > 9)
            return (-2);
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientInfo[i].nickName.compare(nick) == 0)
                valid = -1;
        }
    }
    char *tmp3 = strstr(buffer, "USER");
    if (tmp3 != NULL)
    {
        std::string user;
        std::string tmp4(tmp3);
        size_t m = tmp4.find(" ");
        if (m != std::string::npos)
            user = tmp4.substr(m + 1, tmp4.find("0") - m - 2);
        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientInfo[i].userName.compare(user) == 0)
                valid = -1;
        }
    }
    if (valid == -1)
        return (-1);
    return (0);
}
*/
void newUser(char *buffer, Client *clientInfo, int i)
{
    //userList user;
    char *tmp = strstr(buffer, "NICK");
    if (tmp != NULL) 
    {
        std::string tmp2(tmp);
        size_t l = tmp2.find(" ");
        if (l != std::string::npos) 
            clientInfo[i].nickName = tmp2.substr(l + 1, tmp2.find("\n") - l - 2);
    }
    char *tmp3 = strstr(buffer, "USER");
    if (tmp3 != NULL)
    {
        std::string tmp4(tmp3);
        size_t m = tmp4.find(" ");
        if (m != std::string::npos)
            clientInfo[i].userName = tmp4.substr(m + 1, tmp4.find("0") - m - 2);
    }
}

int chanName(std::string nameChannel)
{
    if (nameChannel.find("&") != 0 && nameChannel.find("#") != 0)
        return (-1);
    if (nameChannel.find(" ") != nameChannel.npos)
        return (-1);
    if (nameChannel.find(",") != nameChannel.npos)
        return (-1);
    if (nameChannel.find("\a") != nameChannel.npos)
        return (-1);
    if (nameChannel.length() > 200 || nameChannel.length() < 2)
        return (-1);
    return(0);
}

char *convert(const char *s)
{
    char *p = new char[strlen(s) + 1];
    strcpy(p, s);
    return p;
}