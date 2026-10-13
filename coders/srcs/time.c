/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:33 by tokinira          #+#    #+#             */
/*   Updated: 2026/09/29 08:39:34 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timespec	current_time;

	clock_gettime(CLOCK_REALTIME, &current_time);
	return (current_time.tv_sec * 1000
		+ current_time.tv_nsec / 1000000);
}

void	set_abs_timeout(long target_ms, struct timespec *ts)
{
	ts->tv_sec = target_ms / 1000;
	ts->tv_nsec = (target_ms % 1000) * 1000000L;
}