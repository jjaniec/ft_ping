/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calc_checksum.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 17:51:20 by jjaniec           #+#    #+#             */
/*   Updated: 2020/01/10 14:38:27 by jjaniec          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

unsigned short		calc_checksum(void *b, int len)
{
	unsigned short		*buf;
	unsigned int		sum;
	unsigned short		result;

	if (!b)
		return (0);
	buf = b;
	sum = 0;
	while (len > 1)
	{
		sum += *buf;
		buf++;
		len -= 2;
	}
	if (len == 1)
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return (result);
}
