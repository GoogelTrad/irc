/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   constructor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haouni <haouni@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 16:26:43 by kortolan          #+#    #+#             */
/*   Updated: 2024/03/03 20:31:26 by haouni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "server.hpp"


Client::Client(void)
{
	currentChannel = -1;
	nickName = "";
    pass = 0;
    nick = 0;
    user = 0;
    ok = 0; 
};