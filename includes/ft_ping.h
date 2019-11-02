#include <ft_printf.h>
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>


#ifndef FT_PING_H
# define FT_PING_H

# define IPHDR_SIZE									20 // sizeof(struct iphdr)
# define ICMPHDR_SIZE								ICMP_MINLEN // 8 // sizeof(struct icmphdr)

# define FT_PING_GETADDRINFO_RESOLVE_PROTO			"http"
# define FT_PING_SOCK_TYPE							SOCK_RAW
# define FT_PING_SOCK_PROTO							IPPROTO_ICMP
# define FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT		-1

# define FT_PING_BUFF_LEN							2000
# define FT_PING_DATA_LEN							56
# define FT_PING_DATA_TIMESTAMP_OFFSET				4
# define FT_PING_TTL								64
# define FT_PING_IP_TOT_LEN							(IPHDR_SIZE + ICMPHDR_SIZE + FT_PING_DATA_LEN)
# define FT_PING_ICMP_DATA_FILL						0x42

// https://www.cymru.com/Documents/ip_icmp.h
# define ICMP_ECHOREPLY		0	/* Echo Reply			*/
# define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
# define ICMP_SOURCE_QUENCH	4	/* Source Quench		*/
# define ICMP_REDIRECT		5	/* Redirect (change route)	*/
# define ICMP_ECHO		8	/* Echo Request			*/
# define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
# define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
# define ICMP_TIMESTAMP		13	/* Timestamp Request		*/
# define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply		*/
# define ICMP_INFO_REQUEST	15	/* Information Request		*/
# define ICMP_INFO_REPLY		16	/* Information Reply		*/
# define ICMP_ADDRESS		17	/* Address Mask Request		*/
# define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/
# define NR_ICMP_TYPES		18


/* Codes for UNREACH. */
# define ICMP_NET_UNREACH	0	/* Network Unreachable		*/
# define ICMP_HOST_UNREACH	1	/* Host Unreachable		*/
# define ICMP_PROT_UNREACH	2	/* Protocol Unreachable		*/
# define ICMP_PORT_UNREACH	3	/* Port Unreachable		*/
# define ICMP_FRAG_NEEDED	4	/* Fragmentation Needed/DF set	*/
# define ICMP_SR_FAILED		5	/* Source Route failed		*/
# define ICMP_NET_UNKNOWN	6
# define ICMP_HOST_UNKNOWN	7
# define ICMP_HOST_ISOLATED	8
# define ICMP_NET_ANO		9
# define ICMP_HOST_ANO		10
# define ICMP_NET_UNR_TOS	11
# define ICMP_HOST_UNR_TOS	12
# define ICMP_PKT_FILTERED	13	/* Packet filtered */
# define ICMP_PREC_VIOLATION	14	/* Precedence violation */
# define ICMP_PREC_CUTOFF	15	/* Precedence cut off */
# define NR_ICMP_UNREACH		15	/* instead of hardcoding immediate value */

/* Codes for REDIRECT. */
# define ICMP_REDIR_NET		0	/* Redirect Net			*/
# define ICMP_REDIR_HOST		1	/* Redirect Host		*/
# define ICMP_REDIR_NETTOS	2	/* Redirect Net for TOS		*/
# define ICMP_REDIR_HOSTTOS	3	/* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. */
# define ICMP_EXC_TTL		0	/* TTL count exceeded		*/
# define ICMP_EXC_FRAGTIME	1	/* Fragment Reass time exceeded	*/

# define PRIu8 "hu"
# define PRId8 "hd"
# define PRIx8 "hx"
# define PRIu16 "hu"
# define PRId16 "hd"
# define PRIx16 "hx"
# define PRIu32 "u"
# define PRId32 "d"
# define PRIx32 "x"
# define PRIu64 "llu"
# define PRId64 "lld"
# define PRIx64 "llx"

typedef struct			s_ft_ping_info {
	struct sockaddr_in	*addr;
	char				*hostname;
	struct timeval		starttime;
	unsigned int		pck_transmitted;
	unsigned int		pck_received;
	int					socket;
	double				min;
	double				avg;
	double				max;
	double				mdev;
	sig_atomic_t		wait_for_sigalrm;
}						t_ft_ping_info;


int					ft_ping(int s, struct sockaddr_in *addr);

unsigned short		calc_checksum(void *b, int len);

int					resolve_hostname(char *hostname, struct in_addr *ip);

void				handle_sigalrm(int sig);

void				handle_sigint(int sig);

ssize_t				send_icmp_echo(int s, struct sockaddr_in *dest);

#endif
