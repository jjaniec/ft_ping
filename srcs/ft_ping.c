/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 17:48:57 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

/*
** https://stackoverflow.com/questions/32593697/
**   understanding-the-msghdr-structure-from-sys-socket-h
** size_t recvmsg(int socket, struct msghdr *message, int flags);
** https://blog.benjojo.co.uk/post/linux-icmp-type-69
** https://www.perlmonks.org/?node_id=582132
*/

static ssize_t		recv_icmp_echo_reply(int s, struct sockaddr_in *addr_resp, \
						char *buff)
{
	char			controlbuff[FT_PING_BUFF_LEN];
	struct iovec	iov_reply;
	struct msghdr	reply;

	iov_reply.iov_base = buff;
	iov_reply.iov_len = sizeof(char) * FT_PING_BUFF_LEN;
	reply.msg_name = (struct sockaddr *)addr_resp;
	reply.msg_namelen = sizeof(addr_resp);
	reply.msg_iov = &iov_reply;
	reply.msg_iovlen = 1;
	reply.msg_control = controlbuff;
	reply.msg_controllen = sizeof(controlbuff);
	reply.msg_flags = 0;
	ft_memset(buff, 0, sizeof(char) * FT_PING_BUFF_LEN);
	return (recvmsg(s, &reply, 0));
}

static int			watch_icmp_replies(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_recv;
	char				reply_buff[FT_PING_BUFF_LEN];
	struct timeval		time_reply;
	struct sockaddr_in	addr_resp;

	while (g_ft_ping_info->pck_transmitted > g_ft_ping_info->pck_received || \
		g_ft_ping_info->pck_transmitted < \
			(unsigned int)g_ft_ping_info->pck_count)
	{
		if ((bytes_recv = recv_icmp_echo_reply(s, &addr_resp, reply_buff)) < 0)
		{
			dprintf(2, "recv() failed\n");
			exit(1);
		}
		gettimeofday(&time_reply, NULL);
		if (!format_reply_output(&addr_resp, reply_buff, &time_reply))
			g_ft_ping_info->pck_received++;
	}
	handle_sigint(0);
	return (0);
}

int					ft_ping(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_sent;

	printf("Getpid() = %d / %d\n", getpid(), htons(getpid()));
	printf("PING %s (%s) %d(%d) bytes of data.\n", g_ft_ping_info->hostname, \
		inet_ntoa(g_ft_ping_info->addr->sin_addr), \
		FT_PING_DATA_LEN, FT_PING_DATA_LEN + IPHDR_SIZE + ICMPHDR_SIZE);
	if ((bytes_sent = send_icmp_echo(s, addr)) <= 0)
	{
		dprintf(2, "sendto() failed: %s\n", strerror(errno));
		exit(1);
	}
	watch_icmp_replies(s, addr);
	return (0);
}
