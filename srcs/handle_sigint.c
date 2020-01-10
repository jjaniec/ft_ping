/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_sigint.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/08 20:14:45 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 17:49:16 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

static void		print_statistics_header(char *hostname)
{
	printf("--- %s ping statistics ---\n", hostname);
}

/*
** Print how much packets were sent & received
*/

static void		print_pck_statistics(unsigned int pck_transmitted, \
					unsigned int pck_received, struct timeval *start, \
					struct timeval *stop)
{
	printf("%d packets transmitted, %d received, ", \
		pck_transmitted, \
		pck_received);
	if (pck_received == pck_transmitted)
		printf("0%%");
	else if (!pck_received)
		printf("100%%");
	else
		printf("%g%%", \
			((double)(1 - (double)((double)pck_received / \
				(double)pck_transmitted)) * 100));
	printf(" packet loss, time %.0fms\n",\
		(double)(stop->tv_sec - start->tv_sec) * 1000 + \
		(double)(stop->tv_usec - start->tv_usec) / 1000);
}

/*
** Print rtt info,
** mdev time omitted as sqrt functions from math lib are forbidden by
** the project subject
*/

static void		print_rtt(double min, double avg, double max)
{
	printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n", \
		min, \
		avg, \
		max);
}

void			handle_sigint(int sig)
{
	struct timeval	now;

	(void)sig;
	gettimeofday(&now, NULL);
	write(1, "\n", 1);
	print_statistics_header(g_ft_ping_info->hostname);
	print_pck_statistics(g_ft_ping_info->pck_transmitted, \
		g_ft_ping_info->pck_received, \
		&(g_ft_ping_info->starttime), &now);
	if (g_ft_ping_info->pck_received)
		print_rtt(g_ft_ping_info->min, g_ft_ping_info->avg, \
			g_ft_ping_info->max);
	exit(0);
}
