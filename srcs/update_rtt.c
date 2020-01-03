/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   update_rtt.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/03 22:51:00 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/03 22:51:01 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

extern t_ft_ping_info	*g_ft_ping_info;

void		update_rtt(double time)
{
	if (time > g_ft_ping_info->max)
		g_ft_ping_info->max = time;
	if (time < g_ft_ping_info->min || !g_ft_ping_info->min)
		g_ft_ping_info->min = time;
	g_ft_ping_info->avg = (((g_ft_ping_info->avg * (g_ft_ping_info->pck_received - 1)) + time) / (g_ft_ping_info->pck_received));
}
