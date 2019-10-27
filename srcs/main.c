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
	icmp_out->icmp_seq = 1;//htons(1);
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

static ssize_t		recv_icmp_echo_reply(int s, struct sockaddr_in *addr, struct sockaddr_in *addr_resp)
{
	char			buff[FT_PING_BUFF_LEN];
	char			controlbuff[FT_PING_BUFF_LEN];
	ssize_t			bytes_recv;
	struct iovec	iov_reply;
	struct msghdr	reply;
	struct cmsghdr *reply_cmhdr;
	struct icmp		*icmp_in;

	(void)addr;
	iov_reply.iov_base = buff;
	iov_reply.iov_len = sizeof(buff);
	reply.msg_name = (struct sockaddr *)addr_resp;
    reply.msg_namelen = sizeof(addr_resp);
	reply.msg_iov = &iov_reply;
	reply.msg_iovlen = 1;
	reply.msg_control = controlbuff;
	reply.msg_controllen = sizeof(controlbuff);
	reply.msg_flags = 0;
	icmp_in = (struct icmp *)(buff + sizeof(struct ip));
	ft_memset(buff, 0, sizeof(buff));
	ft_memset(icmp_in, 0, sizeof(icmp_in));
	bytes_recv = recvmsg(s, &reply, 0);
	if (bytes_recv > 0)
	{
		reply_cmhdr = CMSG_FIRSTHDR(&reply);
		while (reply_cmhdr)
		{
			printf("%zu bytes from %s: type=%u icmp_seq=%d\n", bytes_recv, inet_ntoa(addr_resp->sin_addr), icmp_in->icmp_type, icmp_in->icmp_seq);
		    if (reply_cmhdr->cmsg_level == IPPROTO_IP && reply_cmhdr->cmsg_type == IPPROTO_ICMP)
			{
				unsigned char tos = ((unsigned char *)CMSG_DATA(reply_cmhdr))[0];
			    printf("data read: %s, tos byte = %02X\n", buff, tos);
				break ;
        	}
			reply_cmhdr = CMSG_NXTHDR(&reply, reply_cmhdr);
		}
	}
	return bytes_recv;
}

/*
** Send ICMP echo requests & wait for icmp echo replies
**
** iovec: https://doc.riot-os.org/structiovec.html
** msghdr: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structmsghdr.html
** https://www.quora.com/What-is-the-difference-between-recv-and-recvmsg-System-call-on-Linux
** cmsghdr: http://alas.matf.bg.ac.rs/manuals/lspe/snode=153.html
*/

static int          ft_ping(int s, struct sockaddr_in *addr)
{
	ssize_t				bytes_sent;
	ssize_t				bytes_recv;
	struct sockaddr_in	addr_resp;

	if ((bytes_sent = send_icmp_echo(s, addr)) == 0)
	{
		dprintf(2, "sendto() failed\n");
	}
	else
	{
		printf("Sent %zu bytes\n", bytes_sent /*+ sizeof(struct ip)*/);
	}
	// while ((bytes_recv = recv_icmp_echo_reply(s, addr)) == 0)
		// ;
	if ((bytes_recv = recv_icmp_echo_reply(s, addr, &addr_resp)) < 0)
	{
		dprintf(2, "recv() failed\n");
		strerror(errno);
		// break ;
	}
	else
	{
		printf("Received %zu bytes\n", bytes_recv);
		// break ;
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
	return (r);
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
