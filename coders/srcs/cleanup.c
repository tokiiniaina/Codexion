/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:36:58 by tokinira          #+#    #+#             */
/*   Updated: 2026/09/29 08:36:59 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_coder_conditions(t_coder_data *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&coders[i].cond);
		i++;
	}
}

static void	destroy_dongle_mutexes(t_dongle_data *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&dongles[i].mutex);
		i++;
	}
}

void	free_simulation(t_simulation_data *simulation)
{
	if (simulation->coders)
	{
		destroy_coder_conditions(simulation->coders,
			simulation->inited_coders);
		free(simulation->coders);
	}
	pthread_cond_destroy(&simulation->monitor_cond);
	pthread_cond_destroy(&simulation->queue_cond);
	pthread_mutex_destroy(&simulation->queue_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	if (simulation->queue.requests)
		free(simulation->queue.requests);
	if (simulation->dongles)
	{
		destroy_dongle_mutexes(simulation->dongles,
			simulation->inited_dongles);
		free(simulation->dongles);
	}
	if (simulation->threads)
		free(simulation->threads);
}
