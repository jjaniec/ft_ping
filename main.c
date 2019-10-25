
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define ICMP_ID 123
#define ICMP_ECHO_SEQ_COUNT 5

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
** http://www.microhowto.info/howto/calculate_an_internet_protocol_checksum_in_c.html
** One's Complement checksum algorithm
*/
static unsigned short calc_checksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;
    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
    if (nleft == 1)
    {
      *(unsigned char *)(&answer) = *(unsigned char *)w;
      sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}


static void        init(int *s, char **errmsg, int ac, char **av)
{
	int		on;

	on = 1;
    if ((*s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        *errmsg = "Socket creatiion failed, are you root?";
    /* Socket options, tell the kernel we provide the IP structure */
    if (setsockopt(*s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
        *errmsg = "setsockopt() for IP_HDRINCL error";
    else if (ac == 1)
        *errmsg = "Invalid parameters";
}

/*
** https://www.tenouk.com/Module43.html
** http://minirighi.sourceforge.net/html/structip.html
** http://api.tst-sistemas.es/apitsmart-1.6/structhostent.html#details
*/

static void			init_ip(struct ip *ipheader, char *dest_host, size_t data_sent_size)
{
	char			src_hostname[256];
    struct hostent	*src_hp;
	struct hostent	*dst_hp;
	char			src_ip[15];
	char			dst_ip[15];

	if (gethostname(src_hostname, sizeof(src_hostname)) < 0 || \
		!(src_hp = gethostbyname(src_hostname)))
		exit(1);
	if ((dst_hp = gethostbyname(dest_host)) == NULL && \
		((ipheader->ip_dst.s_addr = inet_addr(dest_host)) == -1))
	{
		printf("Unresolvable host: %s\n", dest_host);
		exit(1);
	}

	ipheader->ip_src = (*(struct in_addr *)src_hp->h_addr);
	ipheader->ip_dst = (*(struct in_addr *)dst_hp->h_addr);
    sprintf(src_ip, "%s", inet_ntoa(ipheader->ip_src));
    sprintf(dst_ip, "%s", inet_ntoa(ipheader->ip_dst));
    printf("Source IP: '%s' -- Destination IP: '%s'\n", src_ip, dst_ip);

	ipheader->ip_v = 4; // Version (ipv4)
	ipheader->ip_hl = 5; // Classic datagram length value
	ipheader->ip_tos = 0; // Type of service / priority
	ipheader->ip_len = htons(data_sent_size); //
	ipheader->ip_id = htons(321); // 
	// Flag + offset
	ipheader->ip_off = htons(0); // Fragment offset
	ipheader->ip_ttl = 255; // Max hops
	ipheader->ip_p = IPPROTO_ICMP; // 1
	// ip->ip_sum = 0; -> Checksum
}

/*
** https://www.tenouk.com/Module43.html
** http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
*/

static void			init_icmp(struct icmp *icmpheader)
{
	// http://download.opendds.org/doxygen/ace_tao/html/libace-doc/a00839.html
    icmpheader->icmp_type = ICMP_ECHO;
    icmpheader->icmp_code = 0;
	icmpheader->icmp_cksum = 0;
    icmpheader->icmp_id = getpid();
    icmpheader->icmp_seq = 0;
	// icmpheader->icmp_data = memset0;
	memset(icmpheader->icmp_data, 0, sizeof(icmpheader->icmp_data) /*sizeof(unsigned char) * 4*/ );
}

/*
** https://en.wikipedia.org/wiki/Ping_(networking_utility)
** https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Control_messages
** https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.hala001/sendto.htm
** https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
** https://linux.die.net/man/7/raw
*/

int                main(int ac, char **av)
{
	int				s;
	char			*errmsg = NULL;
    char			data_sent[400];
    char			data_recv[400];
	struct ip		*ip;
	struct icmp		*icmp;
	struct sockaddr_in	dest_addr;
	int				echo_seq_count;

    memset(data_sent, 0, sizeof(data_sent));
	// printf("Size of ip struct %zu - icmp: %zu - sockaddr: %zu - sockaddr_in: %zu - in_addr: %zu\n", \
		// sizeof(ip), sizeof(icmp), sizeof(from), sizeof(to), sizeof(struct in_addr));

	ip = (struct ip *)data_sent;
	icmp = (struct icmp *)(ip + 1);
	init(&s, &errmsg, ac, av);
	if (errmsg)
	{
		dprintf(2, "%s\n", errmsg);
		exit(1);
	}

	init_ip(ip, av[1], sizeof(data_sent));
	init_icmp(icmp);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr = ip->ip_dst;

	echo_seq_count = -1;
	while (++echo_seq_count < ICMP_ECHO_SEQ_COUNT)
	{
		ip->ip_sum = calc_checksum((unsigned short *)data_sent, ip->ip_hl);
		icmp->icmp_cksum = calc_checksum((unsigned short *)icmp, sizeof(data_sent) - sizeof(struct icmp));
		if ((sendto(s, data_sent, sizeof(data_sent), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr))) < 0)
		{
			printf("Sendto() failed!\n");
			exit(1);
		}
	}

    close(s);

    return (0);
}
