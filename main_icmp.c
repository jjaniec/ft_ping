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

void hexDump (const char *desc, const void *addr, const int len) {
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

unsigned short checksum(void *b, int len)
{   unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void ping(struct sockaddr_in *addr) {
    int sd;
    const int val=255;
    struct sockaddr_in r_addr;
    struct icmp *icmp_send, *icmp_recv;
    unsigned char buff[2000];
    int             bytes_recv, bytes_sent;
    unsigned char recv_buf[2000];
    unsigned char *p = buff;
    socklen_t len=sizeof(r_addr);


    sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);

    if ( sd < 0 ) {
        perror("socket");
        return;
    }
    if ( setsockopt(sd, IPPROTO_IP, IP_TTL, &val, sizeof(val)) != 0)
        perror("Set TTL option");
/*    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
        perror("Request nonblocking I/O");*/

    icmp_send = ((struct icmp *)buff);
    icmp_send->icmp_type = ICMP_ECHO;
    icmp_send->icmp_code = 0;
    icmp_send->icmp_id = getpid();
    icmp_send->icmp_seq = htons(1);
    p += sizeof(icmp_send);
    //*p = 'A';

    icmp_send->icmp_cksum = 0;
    memcpy(buff, icmp_send, sizeof(icmp_send)) ;

    icmp_send->icmp_cksum = checksum(buff, sizeof(icmp_send) + 1);
    memcpy(buff, icmp_send, sizeof(icmp_send)) ;

    if ((bytes_sent = sendto(sd, (unsigned char*)buff, sizeof(icmp_send) + 1, 0, (struct sockaddr*)addr, sizeof(*addr))) <= 0 ) {
                printf("Send err.\n");
    }
    else {
      printf("Sent %d bytes\n", bytes_sent);
      hexDump("sent buff:", buff, sizeof(buff));
    }

    int     addr_len = sizeof(addr);
    if((bytes_recv = recvfrom(sd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr, (socklen_t *)&addr_len)) < 0)
    {
      printf("recvfrom failed\n");
    }
    else
    {
      printf("Received %d bytes\n", bytes_recv);
      hexDump("recv buff:", recv_buf , sizeof(recv_buf));

    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s destination_ip\n", argv[0]);
        return 1;
    }

    struct sockaddr_in addr;
    struct in_addr dst;

    if (inet_aton(argv[1], &dst) == 0) {

        perror("inet_aton");
        printf("%s isn't a valid IP address\n", argv[1]);
        return 1;
    }


    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = dst;

    ping(&addr);
    return 0;
}

