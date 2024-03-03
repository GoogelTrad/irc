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
	int			socket;
	int			currentChannel;
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
void    	command(const char* buffer, Client *clientsInfo, Client &clientInfo, std::vector<channel>&channels);
void    	ft_join(const char * buffer, int fd, Client &clientInfo, std::vector<channel>&channels, Client *clientsInfo);
int 		passwCheck(char *av, char *buffer);
void 		newUser(char *buffer, Client *clientInfo, int i);
std::string receivedMsg(const char *buffer, Client &clientInfo);
void 		parsePrivmsg(const std::string& message, std::string &content, std::string &channel_name);
char *		convert(const char *s);
void         parsMode(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo);
int         chanName(std::string nameChannel);
void         parsInv(const char *buffer, Client *clientsInfo, std::vector<channel>&channels, Client &clientInfo);
std::string ft_topic(const char  *buffer);
int         ft_invit(std::vector<channel>&channels,  Client &clientsInfo, int i);
int         ft_isModo(Client *clientInfo, std::vector<channel>&channels, std::string chanl);
void        clearClientInfo(Client &ClientInfo);
int         ft_isInChannel(Client &clientInfo, std::vector<channel>&channels, size_t i);

/*struct identity
{
	std::string nickName;
	std::string userName;
};*/

#endif