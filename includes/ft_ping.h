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

#ifndef FT_PING_H
# define FT_PING_H

# define FT_PING_GETADDRINFO_RESOLVE_PROTO	"http"
# define FT_PING_SOCK_TYPE					SOCK_RAW
# define FT_PING_SOCK_PROTO					IPPROTO_ICMP
# define FT_PING_ICMP_ECHO_SEQ_COUNT		5

# define FT_PING_BUFF_LEN           2000

unsigned short		calc_checksum(void *b, int len);

#endif
