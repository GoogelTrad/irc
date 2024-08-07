/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 13:51:23 by marvin            #+#    #+#             */
/*   Updated: 2024/01/24 13:51:23 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <cstdio>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <poll.h>
# include <cctype>
# include <algorithm>
# include <vector>
# include <csignal>
# include <stdlib.h>
# define MAX_CLIENTS 10
# define BUFFER_SIZE 1024


class Client
{
private:

public:
	Client();
	std::string nickName;
	std::string userName;
	std::string lastmessage;
	std::string buffer_perso;
	int			socket;
	int			currentChannel;
    bool        pass;
    bool        nick;
    bool        user;
    bool        ok;
};


struct channel
{
	std::string name;
	std::vector<std::string> message;
	std::vector<int> fd;
	std::vector<int> modo;
    std::vector<std::string> invites;
    bool invMode;
    bool topRights;
    bool passMode;
    unsigned long  maxUser;
    std::string passW;
    std::string topiC;
};

int         availableUserNickName(Client *clientInfo, char *buffer);
void    	command(const char* buffer, Client *clientsInfo, Client &clientInfo, std::vector<channel>&channels, char *passw);
void    	ft_join(const char * buffer, int fd, Client &clientInfo, std::vector<channel>&channels, Client *clientsInfo);
int 		passwCheck(char *av, char *buffer);
void 		newUser(char *buffer, Client *clientInfo, int i);
std::string receivedMsg(const char *buffer, Client &clientInfo);
void 		parsePrivmsg(const std::string& message, std::string &content, std::string &channel_name);
char *		convert(const char *s);
void        parsMode(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo);
int         chanName(std::string nameChannel);
void        parsInv(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo);
std::string ft_topic(const char  *buffer);
int         ft_invit(std::vector<channel>&channels,  Client &clientsInfo, int i);
int         ft_isModo(Client *clientInfo, std::vector<channel>&channels, std::string chanl);
void        clearClientInfo(Client &ClientInfo);
int         ft_isInChannel(Client &clientInfo, std::vector<channel>&channels, size_t i);
int         availableNick(Client *clientsInfo, std::string nick);
int         availableUser(Client *clientsInfo, std::string user);
void        parsCap(Client *clientsInfo, char *passw, Client &clientInfo, std::string buf);
int         ft_isOk(Client &clientInfo, int fct, std::string tosend, std::string arg);
std::string addCarRet(const char *buffer);
void        topic(const char *buffer, std::vector<channel>&channels, Client &clientInfo);
void        privMsg(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo);
void        pass(const char *buffer, Client &clientInfo, char *passw);
void        cmd_user(const char *buffer, Client &clientInfo, Client *clientsInfo);
void        cmd_nick(const char *buffer, Client &clientInfo, Client *clientsInfo);
void        cmd_part(const char *buffer, std::vector<channel>&channels, Client &clientInfo);
void        cmd_quit(Client &clientInfo, std::vector<channel>&channels, const char *buffer);
void        cmd_kick(Client &clientInfo, std::vector<channel>&channels, Client *clientsInfo, const char *buffer);
int         ft_which_client(Client *clientsInfo, std::string usr);
std::string ft_chanstate(channel &channel);
std::string ft_whoAtJoin(channel &channel, Client *clientsInfo);

/*struct identity
{
	std::string nickName;
	std::string userName;
};*/

#endif