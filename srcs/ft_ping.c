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
** https://en.wikipedia.org/wiki/IPv4#Header
**
** struct iphdr:
**	{
**		unsigned int 	ihl:4
**		unsigned int 	version:4
**		uint8_t 	tos
**		uint16_t 	tot_len
**		uint16_t 	id
**		uint16_t 	frag_off
**		uint8_t 	ttl
**		uint8_t 	protocol
**		uint16_t 	check
**		uint32_t 	saddr
**		uint32_t 	daddr
**	}
*/

static void			fill_iphdr(struct iphdr *ip, struct in_addr *dest)
{
	ip->ihl = IPHDR_SIZE / 4;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = htons(FT_PING_IP_TOT_LEN);
	ip->id = htons(0);
	ip->frag_off = htons(0);
	ip->ttl = FT_PING_TTL;
	ip->protocol = IPPROTO_ICMP;
	ip->check = 0;
	ip->saddr = INADDR_ANY; /*inet_aton("0.0.0.0")*/;
	ip->daddr = dest->s_addr;
}

/*
** https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
** https://en.wikipedia.org/wiki/Ping_(networking_utility)
** https://docs.huihoo.com/doxygen/linux/kernel/3.7/structicmphdr.html
**
** struct icmphdr:
**	{
**		__u8 	type
**		__u8 	code
**		__sum16 	checksum
**		union {
**		struct {
**			__be16   id
**			__be16   sequence
**		}   echo
**		__be32   gateway
**		struct {
**			__be16   __unused
**			__be16   mtu
**		}   frag
**		} 	un
**	}
*/

static void			fill_icmphdr(struct icmphdr *icmp)
{
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid());
	icmp->un.echo.sequence = g_ft_ping_info->pck_transmitted;//htons(g_ft_ping_info->pck_transmitted);
	icmp->checksum = 0;
	icmp->checksum = calc_checksum(icmp, ICMPHDR_SIZE + FT_PING_DATA_LEN);
}

static void			fill_packet_data(void *data_ptr, size_t data_len)
{
	ft_memset(data_ptr, FT_PING_ICMP_DATA_FILL, data_len);
	gettimeofday((void *)data_ptr + FT_PING_DATA_TIMESTAMP_OFFSET, NULL);
}

/*
** Send icmp echo to ip address
**
** https://www.tenouk.com/Module43.html
** http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
** iphdr:https://docs.huihoo.com/doxygen/linux/kernel/3.7/structicmphdr.html
*/

static ssize_t		send_icmp_echo(int s, struct sockaddr_in *dest)
{
	char			buff[FT_PING_BUFF_LEN];
	struct iphdr	*ip_out;
	struct icmphdr	*icmp_out;
	void			*data_ptr;

	g_ft_ping_info->pck_transmitted++;
	ip_out = (struct iphdr *)buff;
	icmp_out = (void *)buff + IPHDR_SIZE;
	data_ptr = (void *)buff + IPHDR_SIZE + ICMPHDR_SIZE;
	fill_iphdr(ip_out, &(dest->sin_addr));
	fill_packet_data(data_ptr, FT_PING_DATA_LEN);
	fill_icmphdr(icmp_out);
	return (
		sendto(s, buff, FT_PING_IP_TOT_LEN, 0, (struct sockaddr *)dest, sizeof(*dest))
	);
}

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

	g_ft_ping_info->pck_received++;
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
		printf("%"PRIu16" bytes from %s: icmp_seq=%d ttl=%d time=%'.3fms\n", \
			(uint16_t)(ntohs(ip_in->tot_len) /*- (uint16_t)sizeof(struct iphdr)*/), \
			inet_ntoa((struct in_addr) {.s_addr = ip_in->saddr}), icmp_in->un.echo.sequence, ip_in->ttl, \
			(double)(stop->tv_sec - start->tv_sec) * 1000 + (double)(stop->tv_usec - start->tv_usec) / 1000);
	else
		printf("%zu bytes from %s: %s\n", \
			ip_in->tot_len - sizeof(struct iphdr), inet_ntoa(addr_resp->sin_addr), \
			(icmp_in->type < sizeof(icmp_responses_str)) ? (icmp_responses_str[icmp_in->type - 1]) : ("Unknown error code"));
	return (0);
}

int					ft_ping(int s, struct sockaddr_in *addr)
{
	char				reply_buff[FT_PING_BUFF_LEN];
	ssize_t				bytes_sent;
	ssize_t				bytes_recv;
	struct sockaddr_in	addr_resp;
	struct timeval		time_reply;
	struct msghdr		reply;

	printf("PING %s (%s) %d(%d) bytes of data.\n", g_ft_ping_info->hostname, \
		inet_ntoa(g_ft_ping_info->addr->sin_addr), \
		FT_PING_DATA_LEN, FT_PING_DATA_LEN + IPHDR_SIZE + ICMPHDR_SIZE);
	while (g_ft_ping_info->pck_transmitted != FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT)
	{
		while (g_ft_ping_info->wait_for_sigalrm)
			;
		errno = 0;
		if ((bytes_sent = send_icmp_echo(s, addr)) <= 0)
		{
			dprintf(2, "sendto() failed: %s\n", strerror(errno));
			exit(1);
		}
		if ((bytes_recv = recv_icmp_echo_reply(s, addr /*&addr_resp*/, reply_buff)) < 0)
		{
			dprintf(2, "recv() failed\n");
			exit(1);
		}
		gettimeofday(&time_reply, NULL);
		format_reply_output(bytes_recv, &addr_resp, reply_buff, &time_reply);
		g_ft_ping_info->wait_for_sigalrm = true;
		alarm(1);
	}
	return (0);
}
