/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 14:14:33 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

static const char	*g_icmp_responses_str[] =
{
	[ICMP_DEST_UNREACH]		= "Destination Unreachable",
	[ICMP_SOURCE_QUENCH]	= "Source Quench",
	[ICMP_REDIRECT]			= "Redirect (change route)",
	[ICMP_ECHO]				= "Echo Request",
	[ICMP_TIME_EXCEEDED]	= "Time to live exceeded",
	[ICMP_PARAMETERPROB]	= "Parameter Problem",
	[ICMP_TIMESTAMP]		= "Timestamp Request",
	[ICMP_TIMESTAMPREPLY]	= "Timestamp Reply",
	[ICMP_INFO_REQUEST]		= "Information Request",
	[ICMP_INFO_REPLY]		= "Information Reply",
	[ICMP_ADDRESS]			= "Address Mask Request",
	[ICMP_ADDRESSREPLY]		= "Address Mask Reply"
};

/*
** https://stackoverflow.com/questions/32593697/\
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

/*
** Calculate time difference between 2 timeval structs using tv_sec & tv_usec
*/

static double		calc_timeval_diff(struct timeval *start, \
						struct timeval *stop)
{
	return ((double)(stop->tv_sec - start->tv_sec) * 1000 + \
		(double)(stop->tv_usec - start->tv_usec) / 1000);
}

/*
** Send ICMP echo requests & wait for icmp echo replies
**
** iovec: https://doc.riot-os.org/structiovec.html
** msghdr: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structmsghdr.html
** https://www.quora.com/What-is-the-difference-between- \
**   recv-and-recvmsg-System-call-on-Linux
** cmsghdr: http://alas.matf.bg.ac.rs/manuals/lspe/snode=153.html
*/

static int			format_reply_output(struct sockaddr_in *addr_resp, \
						char *reply_data, struct timeval *stop)
{
	struct iphdr		*ip_in;
	struct icmphdr		*icmp_in;
	double				timeval_diff;

	ip_in = (struct iphdr *)reply_data;
	icmp_in = (void *)reply_data + IPHDR_SIZE;
	timeval_diff = calc_timeval_diff((struct timeval *)((void *)reply_data + \
		IPHDR_SIZE + ICMPHDR_SIZE + FT_PING_DATA_TIMESTAMP_OFFSET), stop);
	if (icmp_in->un.echo.id != htons(getpid()))
		return (0);
	if (icmp_in->type == ICMP_ECHOREPLY)
	{
		printf("%hu bytes from %s: icmp_seq=%d ttl=%d time=%'.1fms\n", \
			(uint16_t)(ntohs(ip_in->tot_len) - IPHDR_SIZE), \
			inet_ntoa((struct in_addr) {.s_addr = ip_in->saddr}), \
				icmp_in->un.echo.sequence, ip_in->ttl, timeval_diff);
		update_rtt(timeval_diff);
	}
	else
		printf("%zu bytes from %s: icmp_type=%d %s\n", \
			ip_in->tot_len - sizeof(struct iphdr), \
			inet_ntoa(addr_resp->sin_addr), icmp_in->type, \
			(icmp_in->type < sizeof(g_icmp_responses_str)) ? \
			(g_icmp_responses_str[icmp_in->type - 1]) : ("Unknown error code"));
	return (0);
}

static int			watch_icmp_replies(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_recv;
	char				reply_buff[FT_PING_BUFF_LEN];
	struct timeval		time_reply;
	struct sockaddr_in	addr_resp;

	while (g_ft_ping_info->pck_transmitted > g_ft_ping_info->pck_received || \
		g_ft_ping_info->pck_transmitted < g_ft_ping_info->pck_count)
	{
		if ((bytes_recv = recv_icmp_echo_reply(s, addr, reply_buff)) < 0)
		{
			dprintf(2, "recv() failed\n");
			exit(1);
		}
		g_ft_ping_info->pck_received++;
		gettimeofday(&time_reply, NULL);
		format_reply_output(&addr_resp, reply_buff, &time_reply);
	}
	handle_sigint(0);
}

int					ft_ping(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_sent;

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
