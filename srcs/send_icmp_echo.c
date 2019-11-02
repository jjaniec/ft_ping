#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

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
	icmp->un.echo.sequence = g_ft_ping_info->pck_transmitted + 1;
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

ssize_t				send_icmp_echo(int s, struct sockaddr_in *dest)
{
	char			buff[FT_PING_BUFF_LEN];
	struct iphdr	*ip_out;
	struct icmphdr	*icmp_out;
	void			*data_ptr;
	ssize_t			r;

	ip_out = (struct iphdr *)buff;
	icmp_out = (void *)buff + IPHDR_SIZE;
	data_ptr = (void *)buff + IPHDR_SIZE + ICMPHDR_SIZE;
	fill_iphdr(ip_out, &(dest->sin_addr));
	fill_packet_data(data_ptr, FT_PING_DATA_LEN);
	fill_icmphdr(icmp_out);
	if ((r = (sendto(s, buff, FT_PING_IP_TOT_LEN, 0, (struct sockaddr *)dest, sizeof(*dest)))) != 1)
		g_ft_ping_info->pck_transmitted++;
	else
		dprintf(2, "Sendto() error\n");
	g_ft_ping_info->wait_for_sigalrm = true;
	if (g_ft_ping_info->pck_transmitted < g_ft_ping_info->pck_count)
		alarm(1);
	return (r);
}
