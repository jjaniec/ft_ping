#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

static void		print_statistics_header(char *hostname)
{
	write(1, "--- ", 4);
	write(1, hostname, ft_strlen(hostname));
	write(1, " ping statistics ---\n", 22);
}

static void		print_pck_statistics(unsigned int pck_transmitted, unsigned int pck_received, 
					struct timeval *start, struct timeval *stop)
{
	ft_putnbr(pck_transmitted);
	write(1, " packets transmitted, ", 22);
	ft_putnbr(pck_received);
	write(1, " received, ", 11);
	ft_putnbr(100 - (100 * (pck_transmitted / pck_received)));
	write(1, "% packet loss, time ", 20);
	ft_putnbr(\
		(double)(stop->tv_sec - start->tv_sec) * 1000 + \
		(double)(stop->tv_usec - start->tv_usec) / 1000 \
	);
	write(1, "ms\n", 3);
	write(1, "rtt min/avg/max/mdev = ", 23);
	write(1, "\n", 1);
}

void		handle_sigint(int sig)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	print_statistics_header(g_ft_ping_info->hostname);
	print_pck_statistics(g_ft_ping_info->pck_transmitted, g_ft_ping_info->pck_received, \
		&(g_ft_ping_info->starttime), &now);
	exit(0);
}