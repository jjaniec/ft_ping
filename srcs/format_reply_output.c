/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   format_reply_output.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/08 20:14:45 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 16:42:56 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

static char			*g_icmp_responses_str[] =
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

static char			*get_error_msg(unsigned int idx)
{
	return ((idx < sizeof(g_icmp_responses_str)) ? \
		(g_icmp_responses_str[idx - 1]) : ("Unknown error code"));
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

void	debug_icmp_struct(struct icmphdr *icmp_in)
{
	printf("ICMPHDR STRUCT:\n{\n\tid: %"PRIu8"\n", icmp_in->type);
	printf("\tcode: %"PRIu8"\n", icmp_in->code);
	printf("\tun:\n" "\t\techo {\n" "\t\t\tid:, %"PRIu16"\n", icmp_in->un.echo.id);
	printf("\t\t\tsequence: %"PRIu16"\n\t\t}\n}\n", icmp_in->un.echo.sequence);
}

int					format_reply_output(struct sockaddr_in *addr_resp, \
						char *reply_data, struct timeval *stop)
{
	struct iphdr		*ip_in;
	struct icmphdr		*icmp_in;
	double				timeval_diff;

	ip_in = (struct iphdr *)reply_data;
	icmp_in = (void *)reply_data + IPHDR_SIZE;
	timeval_diff = calc_timeval_diff((struct timeval *)((void *)reply_data + \
		IPHDR_SIZE + ICMPHDR_SIZE + FT_PING_DATA_TIMESTAMP_OFFSET), stop);
	// printf("Rcvd ICMP Echo id: %u / %u\n", icmp_in->un.echo.id, ntohs(icmp_in->un.echo.id));
	if (icmp_in->un.echo.id != ntohs(getpid()))
	{
		if (g_ft_ping_info->opt.v)
			printf("Ignored packet (pid != echo.id) (%u != %"PRIu16") of %hu bytes from %s: icmp.type=%"PRIu8" icmp.code=%"PRIu8" icmp.echo.sequence=%"PRIu16"\n", \
				getpid(), htons(icmp_in->un.echo.id), (uint16_t)(ntohs(ip_in->tot_len) - IPHDR_SIZE), inet_ntoa((struct in_addr) {.s_addr = ip_in->saddr}), icmp_in->type, icmp_in->code, icmp_in->un.echo.sequence);
		return (1);
	}
	// debug_icmp_struct(icmp_in);
	if (icmp_in->type == ICMP_ECHOREPLY)
	{
		printf("%hu bytes from %s: icmp_seq=%d ttl=%d ", \
			(uint16_t)(ntohs(ip_in->tot_len) - IPHDR_SIZE), \
			inet_ntoa((struct in_addr) {.s_addr = ip_in->saddr}), \
				icmp_in->un.echo.sequence, ip_in->ttl);
		printf((timeval_diff >= 1) ? ("time=%'.2fms\n") : \
			("time=%'.3fms\n"), timeval_diff);
		update_rtt(timeval_diff);
	}
	else if (icmp_in->type != ICMP_ECHO)
		printf("%zu bytes from %s: icmp_type=%d %s\n", \
		ip_in->tot_len - sizeof(struct iphdr), inet_ntoa(addr_resp->sin_addr), \
			icmp_in->type, get_error_msg(icmp_in->type));
	else
		return (1);
	return (0);
}
