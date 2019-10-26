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

# define GETADDRINFO_RESOLVE_PROTO	"http"
# define SOCK_TYPE					SOCK_RAW
# define SOCK_PROTO					IPPROTO_ICMP
# define ICMP_ECHO_SEQ_COUNT		5

unsigned short		calc_checksum(void *b, int len);

#endif
