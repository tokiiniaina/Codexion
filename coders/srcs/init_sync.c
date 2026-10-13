/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_sync.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/13 00:00:00 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/13 00:00:00 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_mutexes(t_simulation_data *simulation)
{
	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (pthread_mutex_init(&simulation->queue_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	return (0);
}

static void	destroy_mutexes(t_simulation_data *simulation)
{
	pthread_mutex_destroy(&simulation->queue_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
}

int	init_sync_primitives(t_simulation_data *simulation,
		t_simulation_config *config)
{
	if (init_mutexes(simulation) != 0)
		return (1);
	if (pthread_cond_init(&simulation->queue_cond, NULL) != 0)
	{
		destroy_mutexes(simulation);
		return (1);
	}
	if (pthread_cond_init(&simulation->monitor_cond, NULL) != 0)
	{
		pthread_cond_destroy(&simulation->queue_cond);
		destroy_mutexes(simulation);
		return (1);
	}
	if (init_priority_queue(&simulation->queue,
			config->number_of_coders) != 0)
	{
		pthread_cond_destroy(&simulation->monitor_cond);
		pthread_cond_destroy(&simulation->queue_cond);
		destroy_mutexes(simulation);
		return (1);
	}
	return (0);
}
