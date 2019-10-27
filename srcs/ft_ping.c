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

/*
** Send icmp echo to ip address
**
** https://www.tenouk.com/Module43.html
** http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
*/

static ssize_t		send_icmp_echo(int s, struct sockaddr_in *addr, struct timeval *time_echo, int _icmp_seq)
{
	char			buff[FT_PING_BUFF_LEN];
	struct icmp		*icmp_out;

	icmp_out = (struct icmp *)buff;
	icmp_out->icmp_type = ICMP_ECHO;
	icmp_out->icmp_code = 0;
	icmp_out->icmp_id = getpid();
	icmp_out->icmp_seq = _icmp_seq;
	icmp_out->icmp_cksum = 0;
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	icmp_out->icmp_cksum = calc_checksum(buff, sizeof(icmp_out) + 1);
	ft_memcpy(buff, icmp_out, sizeof(icmp_out));
	gettimeofday(time_echo, NULL);
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

static int			format_reply_output(ssize_t bytes_recv, struct sockaddr_in *addr_resp, char *reply_data)
{
	// struct cmsghdr 		*reply_cmhdr;
	struct icmp			*icmp_in;

	icmp_in = (struct icmp *)((void *)reply_data + sizeof(struct ip));
	printf("%zu bytes from %s: type=%u icmp_seq=%d\n", bytes_recv, inet_ntoa(addr_resp->sin_addr), icmp_in->icmp_type, icmp_in->icmp_seq);

	// reply_cmhdr = CMSG_FIRSTHDR(&reply);
	// while (reply_cmhdr)
	// {
	// 	printf("%zu bytes from %s: type=%u icmp_seq=%d\n", bytes_recv, inet_ntoa(addr_resp->sin_addr), icmp_in->icmp_type, icmp_in->icmp_seq);
	// 	if (reply_cmhdr->cmsg_level == IPPROTO_IP && reply_cmhdr->cmsg_type == IPPROTO_ICMP)
	// 	{
	// 		unsigned char tos = ((unsigned char *)CMSG_DATA(reply_cmhdr))[0];
	// 		printf("data read: %s, tos byte = %02X\n", buff, tos);
	// 		break ;
	// 	}
	// 	reply_cmhdr = CMSG_NXTHDR(&reply, reply_cmhdr);
	// }
	return (0);
}

int               ft_ping(int s, struct sockaddr_in *addr)
{
	char				reply_buff[FT_PING_BUFF_LEN];
	ssize_t				bytes_sent;
	ssize_t				bytes_recv;
	struct sockaddr_in	addr_resp;
	struct timeval		time_echo;
	struct timeval		time_reply;
	struct msghdr		reply;
	int					i;

	i = -1;
	while (++i < FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT)
	{
		if ((bytes_sent = send_icmp_echo(s, addr, &time_echo, i)) == 0)
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
		format_reply_output(bytes_recv, &addr_resp, reply_buff);
	}
	return (0);
}
