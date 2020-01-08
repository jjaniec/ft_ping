/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_sigalrm.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/08 20:13:59 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/08 20:13:59 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

void		handle_sigalrm(int sig)
{
	(void)sig;
	g_ft_ping_info->wait_for_sigalrm = false;
	send_icmp_echo(g_ft_ping_info->socket, g_ft_ping_info->addr);
}
