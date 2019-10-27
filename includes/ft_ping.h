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

# define FT_PING_GETADDRINFO_RESOLVE_PROTO			"http"
# define FT_PING_SOCK_TYPE							SOCK_RAW
# define FT_PING_SOCK_PROTO							IPPROTO_ICMP
# define FT_PING_DEFAULT_ICMP_ECHO_SEQ_COUNT		-1

# define FT_PING_BUFF_LEN							2000

typedef struct			s_ft_ping_info {
	struct sockaddr_in	*addr;
	char				*hostname;
	struct timeval		starttime;
	unsigned int		pck_transmitted;
	unsigned int		pck_received;
	sig_atomic_t		wait_for_sigalrm;
}						t_ft_ping_info;


int					ft_ping(int s, struct sockaddr_in *addr);

unsigned short		calc_checksum(void *b, int len);

int					resolve_hostname(char *hostname, struct in_addr *ip);

void				handle_sigalrm(int sig);

void				handle_sigint(int sig);

#endif
