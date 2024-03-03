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
    ClientInfo.socket = 0;
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

//fonction pour récupérer que le nom du channel dans le message PRIVMSG #channelname :message
void parsePrivmsg(const std::string& message, std::string &content, std::string &channel_name) 
{
    std::size_t pos = message.find("PRIVMSG ");
    if (pos != std::string::npos) 
    {
        pos += 8;
        std::size_t end = message.find(' ', pos);
        if (end != std::string::npos) 
        {
            channel_name = message.substr(pos, end - pos);
            pos = message.find(':', end);
            if (pos != std::string::npos) 
            {
                content = message.substr(pos + 1);
            }
        }
    }
}


void parsInv(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo)
{
    std::string buf = buffer;
    std::string tmp = buf.substr(7, std::string::npos - 7);
    std::string nickToInv = tmp.substr(0, tmp.find(" "));
    std::string tmp2 = tmp.substr(nickToInv.length() + 1, tmp.npos - nickToInv.length() + 1);
    std::string chanl = tmp2.substr(0, tmp2.find("\n") - 1);

    if (ft_isModo(&clientInfo, channels, chanl) == 0)
    {
        for (size_t j = 0; j < channels.size(); j++)
        {        
            for(size_t k = 0; k < channels[j].fd.size(); k++)
            {
                if (channels[j].fd[k] == clientInfo.socket)
                    send(clientInfo.socket, "You are not Moderator on this channel and cant perform this action \n", 69, 0);
            }
        }
        return;
    }
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientsInfo[i].nickName.compare(nickToInv) == 0)
        {
            for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].name.compare(chanl) == 0)
                {
                    std::string tosend = "You have been invited to join " + chanl + " by " + clientInfo.nickName + "\r\n";
                    send(clientsInfo[i].socket, tosend.c_str(), tosend.length(), 0);
                    if (channels[j].invMode == 1)
                    {
                        channels[j].invites.push_back(clientsInfo[i].nickName);
                        return;
                    }
                    else 
                        return;
                }
            }
        }
    }
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

void   sendToAllChannel(std::vector<channel> &channels, size_t j, std::string message)
{
    for (size_t y = 0; y < channels[j].fd.size(); y++)
        send(channels[j].fd[y], message.c_str(), message.size(), 0);
}

void parsMode(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo)
{
    std::string buf = buffer;
    std::string tmp = buf.substr(5, std::string::npos - 5);
    std::string chanl = tmp.substr(0, tmp.find(" "));
    if (buf.size() == chanl.length() + 5)
        return;
    if (ft_isModo(&clientInfo, channels, chanl) == 0)
    {            
        for (size_t j = 0; j < channels.size(); j++)
        {        
            for(size_t k = 0; k < channels[j].fd.size(); k++)
            {
                if (channels[j].fd[k] == clientInfo.socket)
                    send(clientInfo.socket, "You are not Moderator on this channel and cant perform this action \n", 69, 0);
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
                    std::string tosend = chanl + " channel is in invite only mode \r\n";
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
                    std::string tosend = chanl + " channel is NOT in invite only mode\r\n";
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
                    std::string tosend = chanl + " channel is in topicModoOnly mode\r\n";
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
                    std::string tosend = chanl + " channel is NOT in topicModoOnly mode\r\n";
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
                    std::string tosend = chanl + " channel changed the password requiered to join \r\n";
                    sendToAllChannel(channels, j, tosend);
                    return;
                }  
                else
                {
                    std::string tosend = chanl + " channel now requiers a password to be joined \r\n";
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
                    std::string tosend = chanl + " channel dont need password to be joined anymore \r\n";
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
                        std::string tosend = chanl + " now have " + argPostFlag + " has a moderator \r\n";
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
                                std::string tosend = chanl + " took off " + argPostFlag + "'s moderator rights\r\n";
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
                    std::string tosend = chanl + " now have a limit of " + argPostFlag + " slots for users \r\n";
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
                        std::string tosend = chanl + " took off the limit of slots for users \r\n";
                        sendToAllChannel(channels, j, tosend);
                        channels[j].maxUser = 99;
                    }
                }
            }
        return;
    }
    return;
}


int availableUserNickName(Client *clientInfo, char *buffer)
{
    char *tmp = strstr(buffer, "NICK");
    int valid = 0;
    if (tmp != NULL) 
    {
        std::string tmp2(tmp);
        size_t l = tmp2.find(" ");
        std::string nick;
        if (l != std::string::npos) 
            nick = tmp2.substr(l + 1, tmp2.find("\n") - l - 2);
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