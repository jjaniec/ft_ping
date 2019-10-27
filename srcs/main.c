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

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, "http", &hints, &res))
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

/*
** Send icmp echo to ip address
**
** https://www.tenouk.com/Module43.html
** http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
*/


static ssize_t		send_icmp_echo(int s, struct sockaddr_in *addr)
{
	char            buff[FT_PING_BUFF_LEN];
	struct icmp     *icmp_out;

	icmp_out = (struct icmp *)buff;
	icmp_out->icmp_type = ICMP_ECHO;
	icmp_out->icmp_code = 0;
	icmp_out->icmp_id = getpid();
	icmp_out->icmp_seq = htons(1);
	icmp_out->icmp_cksum = 0;
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	icmp_out->icmp_cksum = calc_checksum(buff, sizeof(icmp_out) + 1);
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	return (
		sendto(s, buff, sizeof(icmp_out) + 1, 0, (struct sockaddr *)addr, sizeof(*addr))
	);
}

/*
** Send ICMP echo requests & wait for icmp echo replies
*/

static int          ft_ping(int s, struct sockaddr_in *addr)
{
	ssize_t		bytes_sent;

	if ((bytes_sent = send_icmp_echo(s, addr)) <= 0)
	{
		dprintf(2, "sendto() failed\n");
	}
	else
	{
		printf("Sent %zu bytes", bytes_sent);
	}
	return (0);
}

/*
** Init socket & set socket options
*/

static int          init_socket(int *s)
{
	int         r;
	int   		on;

	r = 0;
	on = 255;
	if ((*s = socket(PF_INET, FT_PING_SOCK_TYPE, FT_PING_SOCK_PROTO)) < 0)
	{
		dprintf(2, "socket() error, are you root ?\n");
		r = 1;
	}
	else if (setsockopt(*s, IPPROTO_IP, IP_TTL, &on, sizeof(on)) != 0)
	{
		dprintf(2, "Failed to set TTL option on socket\n");
		r = 1;
	}
	return (r);
}

/*
** in_addr: https://apr.apache.org/docs/apr/trunk/structin__addr.html
** sockaddr_in: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structsockaddr__in.html
*/

int		main(int ac, char **av)
{
	struct in_addr			ip;
	struct sockaddr_in		addr;
	int                     s;

	if (ac != 2)
	{
		printf("Usage: %s destination_ip\n", av[0]);
		return (1);
	}
	else if (resolve_hostname(av[1], &ip))
	{
		dprintf(2, "%s: cannot resolve %s: Unknown host\n", av[0], av[1]);
		return (1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr = ip;
	printf("PING %s (%s)\n", av[1], inet_ntoa(ip));
	return (init_socket(&s) || ft_ping(s, &addr));
}
