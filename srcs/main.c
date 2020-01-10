/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 17:49:32 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

t_ft_ping_info		*g_ft_ping_info;

/*
** Allowed functions:
** - getpid.
** - getuid.
** - getaddrinfo.
** - gettimeofday.
** - inet_ntop.
** - inet_pton.
** - exit.
** - signal.
** - alarm.
** - setsockopt.
** - recvmsg.
** - sendto.
** - socket.
*/

/*
** Init socket & set socket options
** https://stackoverflow.com/questions/32593697/ \
**   understanding-the-msghdr-structure-from-sys-socket-h
*/

static int		init_socket(int *s)
{
	int			r;
	int			on;

	r = 1;
	on = 255;
	if ((*s = socket(PF_INET, FT_PING_SOCK_TYPE, FT_PING_SOCK_PROTO)) < 0)
		dprintf(2, "socket() error, are you root ?\n");
	else if (setsockopt(*s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != 0)
		dprintf(2, "Failed to set IP_HDRINCL option on socket\n");
	else if (setsockopt(*s, IPPROTO_IP, IP_TTL, &on, sizeof(on)) != 0)
		dprintf(2, "Failed to set TTL option on socket\n");
	else if (setsockopt(*s, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on)) != 0)
		dprintf(2, "Failed to set PKTINFO option on socket\n");
	else if (setsockopt(*s, IPPROTO_IP, IP_TOS, &on, sizeof(on)) != 0)
		dprintf(2, "Failed to set TOS option on socket\n");
	else
		r = 0;
	g_ft_ping_info->socket = *s;
	return (r);
}

/*
** Init shared global struct
*/

static int		init_ft_ping_info(t_ft_ping_info *ft_ping_info, \
					struct sockaddr_in *addr)
{
	g_ft_ping_info = ft_ping_info;
	ft_memset(addr, 0, sizeof(struct sockaddr_in));
	ft_memset(\
		(void *)ft_ping_info + sizeof(struct sockaddr_in *), \
		0, \
		sizeof(t_ft_ping_info) - sizeof(struct sockaddr_in *));
	ft_ping_info->addr = addr;
	ft_ping_info->pck_count = FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT;
	return (gettimeofday(&(g_ft_ping_info->starttime), NULL));
}

static int		ft_str_isnum(char *s)
{
	while (s++ && *s)
		if (*s > '9' || *s < '0')
			return (0);
	return (1);
}

static char		**parse_opts(char **av)
{
	char	*s;

	while (av && *av && **av == '-')
	{
		s = (*av);
		while (s++ && *s)
		{
			if (*s == 'h')
				g_ft_ping_info->opt.h = true;
			else if (*s == 'v')
				g_ft_ping_info->opt.v = true;
			else if (*s == 'c')
			{
				g_ft_ping_info->opt.c = true;
				if (!av[1] || !ft_str_isnum(av[1]) || \
					(g_ft_ping_info->pck_count = ft_atoi(av[1])) < 1)
					return (NULL);
				av++;
			}
			else
				return (NULL);
		}
		av++;
	}
	return (av);
}

/*
** in_addr: https://apr.apache.org/docs/apr/trunk/structin__addr.html
** sockaddr_in: https://docs.huihoo.com/doxygen/ \
**   linux/kernel/3.7/structsockaddr__in.html
** https://stackoverflow.com/questions/18609397/ \
**   whats-the-difference-between-sockaddr-sockaddr-in-and-sockaddr-in6
*/

int				main(int ac, char **av)
{
	struct in_addr			ip;
	struct sockaddr_in		addr;
	int						s;
	t_ft_ping_info			ft_ping_info;
	char					**args;

	init_ft_ping_info(&ft_ping_info, &addr);
	args = parse_opts(av + 1);
	if (ac == 1 || ft_ping_info.opt.h || !args || !args[0])
	{
		printf("Usage: %s [-h -v -c [count]] destination_ip\n", av[0]);
		return (1);
	}
	else if (resolve_hostname(args[0], &ip))
	{
		dprintf(2, "%s: %s: Name or service not known\n", av[0], args[0]);
		return (1);
	}
	g_ft_ping_info->hostname = args[0];
	addr.sin_family = AF_INET;
	addr.sin_addr = ip;
	signal(SIGALRM, handle_sigalrm);
	signal(SIGINT, handle_sigint);
	return (init_socket(&s) || ft_ping(s, &addr));
}
