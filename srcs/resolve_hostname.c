/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2019/10/27 19:50:22 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

/*
** https://en.wikipedia.org/wiki/Getaddrinfo
** https://stackoverflow.com/questions/21099041/why-do-we-cast-sockaddr-in-to-sockaddr-when-calling-bind
** https://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
*/

int			resolve_hostname(char *hostname, struct in_addr *ip)
{
	struct addrinfo		hints;
	struct addrinfo		*res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, NULL, &hints, &res))
	{
		printf("getaddrinfo() failed!\n");
		return (1);
	}
	// res_ptr = res;
	// while (res_ptr)
	// {
	// 	printf("Resolved to: %s\n", 
	// 		inet_ntoa((((struct sockaddr_in *)res_ptr->ai_addr)->sin_addr)));
	// 	res_ptr = res_ptr->ai_next;
	// }
	*ip = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
	return (0);
}