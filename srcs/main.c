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

t_ft_ping_info		*g_ft_ping_info;

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
** Init socket & set socket options
** https://stackoverflow.com/questions/32593697/understanding-the-msghdr-structure-from-sys-socket-h
*/

static int		init_socket(int *s)
{
	int			r;
	int			on;

	r = 0;
	on = 255;
	if ((*s = socket(PF_INET, FT_PING_SOCK_TYPE, FT_PING_SOCK_PROTO)) < 0)
	{
		dprintf(2, "socket() error, are you root ?\n");
		r = 1;
	}
	else if (setsockopt(*s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != 0)
	{
		dprintf(2, "Failed to set IP_HDRINCL option on socket\n");
		r = 1;
	}
	else if (setsockopt(*s, IPPROTO_IP, IP_TTL, &on, sizeof(on)) != 0)
	{
		dprintf(2, "Failed to set TTL option on socket\n");
		r = 1;
	}
	else if (setsockopt(*s, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on)) != 0)
	{
		dprintf(2, "Failed to set PKTINFO option on socket\n");
		r = 1;
	}
	else if (setsockopt(*s, IPPROTO_IP, IP_TOS, &on, sizeof(on)) != 0)
	{
		dprintf(2, "Failed to set TOS option on socket\n");
		r = 1;
	}
	g_ft_ping_info->socket = *s;
	return (r);
}

/*
** Init shared global struct
*/

static int		init_ft_ping_info(t_ft_ping_info *ft_ping_info, char *hostname, struct sockaddr_in *addr)
{
	g_ft_ping_info = ft_ping_info;
	ft_memset(\
		(void *)ft_ping_info + sizeof(struct sockaddr_in *), \
		0, \
		sizeof(t_ft_ping_info) - sizeof(struct sockaddr_in *) \
	);
	ft_ping_info->addr = addr;
	ft_ping_info->hostname = hostname;
	return gettimeofday(&(g_ft_ping_info->starttime), NULL);

}

static char		**parse_opts(char **av)
{
	while (av && *av && **av == '-')
	{
		if (ft_strchr(*av, 'h'))
			g_ft_ping_info->opt.h = true;
		if (ft_strchr(*av, 'v'))
			g_ft_ping_info->opt.v = true;
		av++;
	}
	return (av);
}

/*
** in_addr: https://apr.apache.org/docs/apr/trunk/structin__addr.html
** sockaddr_in: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structsockaddr__in.html
** https://stackoverflow.com/questions/18609397/whats-the-difference-between-sockaddr-sockaddr-in-and-sockaddr-in6
*/

int		main(int ac, char **av)
{
	struct in_addr			ip;
	struct sockaddr_in		addr;
	int						s;
	t_ft_ping_info			ft_ping_info;
	char					**args;

	init_ft_ping_info(&ft_ping_info, av[1], &addr);
	args = parse_opts(av + 1);
	if (ac == 1 || ft_ping_info.opt.h)
	{
		printf("Usage: %s destination_ip\n", av[0]);
		return (1);
	}
	else if (resolve_hostname(args[0], &ip))
	{
		dprintf(2, "%s: %s: Name or service not known\n", av[0], args[1]);
		return (1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr = ip;
	signal(SIGALRM, handle_sigalrm);
	signal(SIGINT, handle_sigint);
	return (init_socket(&s) || ft_ping(s, &addr));
}
