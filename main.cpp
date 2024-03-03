/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/23 16:52:26 by marvin            #+#    #+#             */
/*   Updated: 2024/01/23 16:52:26 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

std::vector<Channel> channelList;
std::vector<std::string> defaultChannel;

/*int	true_channel(std::string name)
{
	for (Channel &channel : channelList)
	{
		if(channel.name == name)
			return (1);
	}
	return (0);
}*/

/*void	ft_join(int socketClient, std::string name)
{
	if (true_channel(name) == 1)
	{

	}

}*/

void*	newClient(void *client)
{
	int socketClient = *((int*)client);

	while(19)
	{
		//UTILISATION DE RECV POUR OBTENIR DES INFORMATIONS DEPUIS LE CLIENT
		char buffer[1024];
    	ssize_t bytesRead = recv(socketClient, buffer, sizeof(buffer), 0);

    	if (bytesRead == -1) 
		{
        	std::cout << "ERROR RECV" << std::endl;
        	close(socketClient);
        	return NULL;
    	}
		if (strncmp(buffer + 5, "QUIT", 4) == 0)
		{
			std::cout << "Deconnexion" << std::endl;
			close(socketClient);
			return NULL;
		}
		printf("%s\n", buffer);
		/*else if (strncmp(buffer, "JOIN ", 5) == 0)
			ft_join(socketClient, buffer + 6);*/
		defaultChannel.push_back(std::string(buffer, bytesRead));
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Invalid arg" << std::endl;
		return 19;
	}
	int port = std::atoi(argv[1]);

	//CREATION DU SOCKET SERVER
	int server = socket(AF_INET, SOCK_STREAM, 0);

	if(server == -1)
	{
		std::cout << "error socket" << std::endl;
		return 19;
	}

	// en cas de pb pour bien cloturer le port
	/*close(server);
	return 19;*/

	// CREATION DE LA STRUCTURE SOCKADDR (_IN pour IPV4)
	struct sockaddr_in adressServerIpv4;
	adressServerIpv4.sin_family = AF_INET; //CODE POUR IPV4
	adressServerIpv4.sin_port = htons(port); //DEFINITION DU PORT
	adressServerIpv4.sin_addr.s_addr = INADDR_ANY; //CODE POUR ACCEPTER TOUTES LES INTERFACES

	//CREATION DE LA LIASON ENTRE LE SOCKET ET LADRESSE
	if (bind(server, (struct sockaddr*)&adressServerIpv4, sizeof(adressServerIpv4)) == -1)
	{
		std::cout << "Error BINDING ADRESSE" << std::endl;
		close(server);
		return 19;
	}

	//MISE EN MODE ECOUTE DE NOUVELLE CONNEXION
	if (listen(server, 10) == -1) // DEFINITION DU SOCKET EN MODE ECOUTE ET DE SA LISTE DATTENTE MAX
	{
		std::cout << "ERROR MISE EN MODE ECOUTE" << std::endl;
		close(server);
		return 19;
	}

	while(19)
	{
		//Creation de la nouvelle structure
		struct sockaddr_in adressClient;
		// Connexion avec le socket
		socklen_t adressClientLen = sizeof(adressClient);
		int* socketClient = new int(accept(server, (struct sockaddr*)&adressClient, &adressClientLen));
		if (*socketClient == -1)
		{
			std::cout << "ERROR CONNEXION CLIENT SOCKET" << std::endl;
			close(server);
			delete socketClient;
			return 19;
		}
		pthread_t clients;
		if (pthread_create(&clients, NULL, newClient, socketClient) != 0)
		{
			std::cout << "ERROR THREAD" << std::endl;
			close(*socketClient);
			delete socketClient;
			return 19;
		}
		pthread_detach(clients);
	}
	close(server);
	return 19;
}


// TENTATIVE DAJOUT DUN MDP GENERIQUE DEMANDER LORS DE LA CONNEXION TELNET
	/*const char*	mdp = argv[2];
	send(socketClient, mdp, strlen(mdp), 0);
	char	mdpBuffer[62];
	ssize_t byteread = recv(socketClient, mdpBuffer, sizeof(mdpBuffer), 0);
	if (byteread == -1)
	{
		std::cout << "Erreur obtention du mpd" << std::endl;
		close(socketClient);
		close(server);
		return 19;
	}
	mdpBuffer[byteread] = '\0';

	if (strcmp(mdpBuffer, mdp) != 0) 
	{
        std::cout << "Authentication failed" << std::endl;
        close(socketClient);
        close(server);
        return 1;
    }*/