#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

void		handle_sigalrm(int sig)
{
	(void)sig;
	g_ft_ping_info->wait_for_sigalrm = false;
}