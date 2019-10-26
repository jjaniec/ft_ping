/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2019/10/26 19:50:22 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

/*
Allowed functions:
- getpid.
- getuid.
- getaddrinfo.
- gettimeofday.
- inet_ntop.
- inet_pton.
- exit.
- signal.
- alarm.
- setsockopt.
- recvmsg.
- sendto.
- socket.
*/

/*
** https://en.wikipedia.org/wiki/Getaddrinfo
** https://stackoverflow.com/questions/21099041/why-do-we-cast-sockaddr-in-to-sockaddr-when-calling-bind
** https://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
*/

static int			resolve_hostname(char *hostname, struct in_addr *ip)
{
	struct addrinfo		hints;
	struct addrinfo		*res;
	struct addrinfo		*res_ptr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, "http", &hints, &res))
	{
		printf("getaddrinfo() failed!\n");
		return (1);
	}
	res_ptr = res;
	// while (res_ptr)
	// {
	// 	printf("Resolved to: %s\n", \
	// 		inet_ntoa((((struct sockaddr_in *)res_ptr->ai_addr)->sin_addr)));
	// 	res_ptr = res_ptr->ai_next;
	// }
	*ip = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
	return (0);
}

/*
** in_addr: https://apr.apache.org/docs/apr/trunk/structin__addr.html
** sockaddr_in: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structsockaddr__in.html
** 
*/

int		main(int ac, char **av)
{
	struct in_addr			ip;
	struct sockaddr_in		addr;
	// struct in_addr			dst;

	(void)ac;
	if (ac != 2)
	{
		printf("Usage: %s destination_ip\n", av[0]);
		return (1);
	}
	else if (resolve_hostname(av[1], &ip))
	{
		printf("%s: cannot resolve %s: Unknown host\n", av[0], av[1]);
		return (1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr = ip;
	printf("PING %s (%s)\n", av[1], inet_ntoa(ip));
	return (0);
}
