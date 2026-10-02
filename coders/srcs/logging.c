/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:09 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/02 23:53:04 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_event(t_simulation_data *simulation, int coder_id, char *message)
{
	long	timestamp;

	timestamp = get_time_ms() - simulation->start_time;
	if (strcmp(message, "burned out") == 0)
	{
		pthread_mutex_lock(&simulation->log_mutex);
		printf("\033[38;5;196m%ld %d %s\033[0m\n",
			timestamp, coder_id + 1, message);
	}
	else
	{
		pthread_mutex_lock(&simulation->log_mutex);
		printf("%ld %d %s\n", timestamp, coder_id + 1, message);
	}
	pthread_mutex_unlock(&simulation->log_mutex);
}
