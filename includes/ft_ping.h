/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/10 14:10:41 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 14:31:28 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <ft_printf.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdio.h>
# include <arpa/inet.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <string.h>
# include <sys/time.h>
# include <signal.h>

/*
** sizeof(struct iphdr)
*/

# define IPHDR_SIZE									20

/*
** // sizeof(struct icmphdr) : 8
*/

# define ICMPHDR_SIZE								ICMP_MINLEN

# define FT_PING_GETADDRINFO_RESOLVE_PROTO			"http"
# define FT_PING_SOCK_TYPE							SOCK_RAW
# define FT_PING_SOCK_PROTO							IPPROTO_ICMP
# define FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT		-1

# define FT_PING_BUFF_LEN							2000
# define FT_PING_DATA_LEN							56
# define FT_PING_DATA_TIMESTAMP_OFFSET				4
# define FT_PING_TTL								64

/*
** # define FT_PING_IP_TOT_LEN (IPHDR_SIZE + ICMPHDR_SIZE + FT_PING_DATA_LEN)
*/

# define FT_PING_IP_TOT_LEN							84
# define FT_PING_ICMP_DATA_FILL						0x42

/*
** https://www.cymru.com/Documents/ip_icmp.h
** ICMP_ECHOREPLY Echo Reply
** ICMP_DEST_UNREACH Destination Unreachable
** ICMP_SOURCE_QUENCH Source Quench
** ICMP_REDIRECT Redirect (change route)
** ICMP_ECHO Echo Request
** ICMP_TIME_EXCEEDED Time Exceeded
** ICMP_PARAMETERPROB Parameter Problem
** ICMP_TIMESTAMP Timestamp Request
** ICMP_TIMESTAMPREPLY Timestamp Reply
** ICMP_INFO_REQUEST Information Request
** ICMP_INFO_REPLY Information Reply
** ICMP_ADDRESS Address Mask Request
** ICMP_ADDRESSREPLY Address Mask Reply
*/

# define ICMP_ECHOREPLY			0
# define ICMP_DEST_UNREACH		3
# define ICMP_SOURCE_QUENCH		4
# define ICMP_REDIRECT			5
# define ICMP_ECHO				8
# define ICMP_TIME_EXCEEDED		11
# define ICMP_PARAMETERPROB		12
# define ICMP_TIMESTAMP			13
# define ICMP_TIMESTAMPREPLY	14
# define ICMP_INFO_REQUEST		15
# define ICMP_INFO_REPLY		16
# define ICMP_ADDRESS			17
# define ICMP_ADDRESSREPLY		18
# define NR_ICMP_TYPES			18

/*
** Codes for UNREACH.
** ICMP_NET_UNREACH Network Unreachable
** ICMP_HOST_UNREACH Host Unreachable
** ICMP_PROT_UNREACH Protocol Unreachable
** ICMP_PORT_UNREACH Port Unreachable
** ICMP_FRAG_NEEDED Fragmentation Needed/DF set
** ICMP_SR_FAILED Source Route failed
** ICMP_NET_UNKNOWN
** ICMP_HOST_UNKNOWN
** ICMP_HOST_ISOLATED
** ICMP_NET_ANO
** ICMP_HOST_ANO
** ICMP_NET_UNR_TOS
** ICMP_HOST_UNR_TOS
** ICMP_PKT_FILTERED Packet filtered
** ICMP_PREC_VIOLATION Precedence violation
** ICMP_PREC_CUTOFF Precedence cut off
** NR_ICMP_UNREACH instead of hardcoding immediate value
*/

# define ICMP_NET_UNREACH		0
# define ICMP_HOST_UNREACH		1
# define ICMP_PROT_UNREACH		2
# define ICMP_PORT_UNREACH		3
# define ICMP_FRAG_NEEDED		4
# define ICMP_SR_FAILED			5
# define ICMP_NET_UNKNOWN		6
# define ICMP_HOST_UNKNOWN		7
# define ICMP_HOST_ISOLATED		8
# define ICMP_NET_ANO			9
# define ICMP_HOST_ANO			10
# define ICMP_NET_UNR_TOS		11
# define ICMP_HOST_UNR_TOS		12
# define ICMP_PKT_FILTERED		13
# define ICMP_PREC_VIOLATION	14
# define ICMP_PREC_CUTOFF		15
# define NR_ICMP_UNREACH		15

/*
** Codes for REDIRECT.
** ICMP_REDIR_NET Redirect Net
** ICMP_REDIR_HOST Redirect Host
** ICMP_REDIR_NETTOS Redirect Net for TOS
** ICMP_REDIR_HOSTTOS Redirect Host for TOS
*/

# define ICMP_REDIR_NET		0
# define ICMP_REDIR_HOST	1
# define ICMP_REDIR_NETTOS	2
# define ICMP_REDIR_HOSTTOS	3

/*
**  Codes for TIME_EXCEEDED.
** ICMP_EXC_TTL TTL count exceeded
** ICMP_EXC_FRAGTIME Fragment Reass time exceeded
*/
# define ICMP_EXC_TTL		0
# define ICMP_EXC_FRAGTIME	1

/*
** # define PRIu8 "hu"
** # define PRId8 "hd"
** # define PRIx8 "hx"
** # define PRIu16 "hu"
** # define PRId16 "hd"
** # define PRIx16 "hx"
** # define PRIu32 "u"
** # define PRId32 "d"
** # define PRIx32 "x"
** # define PRIu64 "llu"
** # define PRId64 "lld"
** # define PRIx64 "llx"
*/

struct						s_ft_ping_opt {
	bool					v : 1;
	bool					h : 1;
	bool					c : 1;
};

typedef struct				s_ft_ping_info {
	struct sockaddr_in		*addr;
	struct timeval			starttime;
	unsigned int			pck_transmitted;
	unsigned int			pck_received;
	int						socket;
	double					min;
	double					avg;
	double					max;
	double					mdev;
	int						pck_count;
	char					*hostname;
	struct s_ft_ping_opt	opt;
	sig_atomic_t			wait_for_sigalrm;
}							t_ft_ping_info;

int							ft_ping(int s, struct sockaddr_in *addr);

unsigned short				calc_checksum(void *b, int len);

int							resolve_hostname(char *hostname, \
								struct in_addr *ip);

void						handle_sigalrm(int sig);

void						handle_sigint(int sig);

ssize_t						send_icmp_echo(int s, struct sockaddr_in *dest);

void						update_rtt(double time);

int							format_reply_output(struct sockaddr_in *addr_resp, \
								char *reply_data, struct timeval *stop);

#endif
