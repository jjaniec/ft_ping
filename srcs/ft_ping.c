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

/*
** Send icmp echo to ip address
**
** https://www.tenouk.com/Module43.html
** http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
*/

static ssize_t		send_icmp_echo(int s, struct sockaddr_in *addr)
{
	char			buff[FT_PING_BUFF_LEN];
	struct icmp		*icmp_out;

	g_ft_ping_info->pck_transmitted++;
	icmp_out = (struct icmp *)buff;
	icmp_out->icmp_type = ICMP_ECHO;
	icmp_out->icmp_code = 0;
	icmp_out->icmp_id = getpid();
	icmp_out->icmp_seq = g_ft_ping_info->pck_transmitted;
	ft_memset(icmp_out->icmp_data, 0x77, FT_PING_DATA_LEN);
	gettimeofday((struct timeval *)icmp_out->icmp_data, NULL);
	icmp_out->icmp_cksum = 0;
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	icmp_out->icmp_cksum = calc_checksum(buff, sizeof(icmp_out) + 1);
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	return (
		sendto(s, buff, sizeof(icmp_out) + 1, 0, (struct sockaddr *)addr, sizeof(*addr))
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
	struct ip			*ip_in;
	struct icmp			*icmp_in;
	struct timeval		*start;

	ip_in = (struct ip *)reply_data;
	icmp_in = (struct icmp *)((void *)reply_data + sizeof(struct ip));
	start = (struct timeval *)icmp_in->icmp_data;
	printf("%zu bytes from %s: icmp_seq=%d ttl=%d time=%'.3fms\n", \
		bytes_recv, inet_ntoa(addr_resp->sin_addr), icmp_in->icmp_seq, ip_in->ip_ttl, \
		(double)(stop->tv_sec - start->tv_sec) * 1000 + (double)(stop->tv_usec - start->tv_usec) / 1000);
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

	printf("PING %s (%s) %d(%zu) bytes of data.\n", g_ft_ping_info->hostname, \
		inet_ntoa(g_ft_ping_info->addr->sin_addr), \
		FT_PING_DATA_LEN, FT_PING_DATA_LEN + sizeof(struct iphdr) + sizeof(struct icmphdr));
	while (g_ft_ping_info->pck_transmitted != FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT)
	{
		while (g_ft_ping_info->wait_for_sigalrm)
			;
		if ((bytes_sent = send_icmp_echo(s, addr)) == 0)
		{
			dprintf(2, "sendto() failed\n");
			exit(1);
		}
		if ((bytes_recv = recv_icmp_echo_reply(s, &addr_resp, reply_buff)) < 0)
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
