/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:52:43 by jjaniec           #+#    #+#             */
/*   Updated: 2019/10/26 19:50:22 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

static const char	*icmp_responses_str[] =
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
** https://stackoverflow.com/questions/32593697/understanding-the-msghdr-structure-from-sys-socket-h
** size_t recvmsg(int socket, struct msghdr *message, int flags);
** https://blog.benjojo.co.uk/post/linux-icmp-type-69
** https://www.perlmonks.org/?node_id=582132
*/

static ssize_t		recv_icmp_echo_reply(int s, struct sockaddr_in *addr_resp, char *buff)
{
	char			controlbuff[FT_PING_BUFF_LEN];
	ssize_t			bytes_recv;
	struct iovec	iov_reply;
	struct msghdr	reply;
	// struct cmsghdr *reply_cmhdr;

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
	return (
		recvmsg(s, &reply, 0)
	);
}

/*
** Send ICMP echo requests & wait for icmp echo replies
**
** iovec: https://doc.riot-os.org/structiovec.html
** msghdr: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structmsghdr.html
** https://www.quora.com/What-is-the-difference-between-recv-and-recvmsg-System-call-on-Linux
** cmsghdr: http://alas.matf.bg.ac.rs/manuals/lspe/snode=153.html
*/

static int			format_reply_output(ssize_t bytes_recv, struct sockaddr_in *addr_resp, char *reply_data, struct timeval *stop)
{
	// struct cmsghdr 		*reply_cmhdr;
	struct iphdr		*ip_in;
	struct icmphdr		*icmp_in;
	struct timeval		*start;

	ip_in = (struct iphdr *)reply_data;
	icmp_in = (void *)reply_data + IPHDR_SIZE;
	start = (struct timeval *)((void *)reply_data + IPHDR_SIZE + ICMPHDR_SIZE + FT_PING_DATA_TIMESTAMP_OFFSET);
	if (icmp_in->un.echo.id != htons(getpid()))
		return 0;
	if (icmp_in->type == ICMP_ECHOREPLY)
		printf("%"PRIu16" bytes from %s: icmp_seq=%d ttl=%d time=%'.1fms\n", \
			(uint16_t)(ntohs(ip_in->tot_len) - IPHDR_SIZE), \
			inet_ntoa((struct in_addr) {.s_addr = ip_in->saddr}), icmp_in->un.echo.sequence, ip_in->ttl, \
			(double)(stop->tv_sec - start->tv_sec) * 1000 + (double)(stop->tv_usec - start->tv_usec) / 1000);
	else
		printf("%zu bytes from %s: %s\n", \
			ip_in->tot_len - sizeof(struct iphdr), inet_ntoa(addr_resp->sin_addr), \
			(icmp_in->type < sizeof(icmp_responses_str)) ? (icmp_responses_str[icmp_in->type - 1]) : ("Unknown error code"));
	return (0);
}

static int			watch_icmp_replies(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_recv;
	char				reply_buff[FT_PING_BUFF_LEN];
	struct timeval		time_reply;
	struct msghdr		reply;
	struct sockaddr_in	addr_resp;

	if (g_ft_ping_info->pck_transmitted != FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT)
		alarm(1);
	else
		return (0);
	if ((bytes_recv = recv_icmp_echo_reply(s, addr, reply_buff)) < 0)
	{
		dprintf(2, "recv() failed\n");
		exit(1);
	}
	g_ft_ping_info->pck_received++;
	gettimeofday(&time_reply, NULL);
	format_reply_output(bytes_recv, &addr_resp, reply_buff, &time_reply);
	// update_rtt();
	if (g_ft_ping_info->pck_transmitted != FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT)
		return (watch_icmp_replies(s, addr));
	else
		return (0);
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
