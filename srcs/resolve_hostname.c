/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_hostname.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 17:49:39 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

/*
** https://en.wikipedia.org/wiki/Getaddrinfo
** https://stackoverflow.com/questions/21099041/why-do-we-cast- \
**	sockaddr-in-to-sockaddr-when-calling-bind
** https://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
** freeaddrinfo() forbidden by the subject
*/

int			resolve_hostname(char *hostname, struct in_addr *ip)
{
	struct addrinfo		hints;
	struct addrinfo		*res;

	ft_memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, NULL, &hints, &res))
		return (1);
	*ip = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
	free(res);
	return (0);
}
