/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:06 by tokinira          #+#    #+#             */
/*   Updated: 2026/09/29 08:39:07 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	alloc_simulation_arrays(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	n;

	n = config->number_of_coders;
	simulation->coders = malloc(sizeof(t_coder_data) * n);
	simulation->threads = malloc(sizeof(pthread_t) * n);
	simulation->dongles = malloc(sizeof(t_dongle_data) * n);
	if (!simulation->coders || !simulation->threads
		|| !simulation->dongles)
	{
		free_simulation(simulation);
		return (1);
	}
	return (0);
}

static int	init_coders(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].burnout_deadline = 0;
		simulation->coders[i].is_finished = 0;
		simulation->coders[i].has_permission = 0;
		if (pthread_cond_init(&simulation->coders[i].cond, NULL) != 0)
		{
			free_simulation(simulation);
			return (1);
		}
		simulation->inited_coders++;
		i++;
	}
	return (0);
}

static int	init_dongles(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].is_available = 1;
		simulation->dongles[i].available_at = 0;
		simulation->dongles[i].is_reserved = 0;
		if (pthread_mutex_init(&simulation->dongles[i].mutex, NULL) != 0)
		{
			free_simulation(simulation);
			return (1);
		}
		simulation->inited_dongles++;
		i++;
	}
	return (0);
}

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	simulation->stop_simulation = 0;
	simulation->start_time = 0;
	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->threads = NULL;
	simulation->finished_coders = 0;
	simulation->inited_coders = 0;
	simulation->inited_dongles = 0;
	simulation->scheduler_thread = 0;
	simulation->request_counter = 0;
	simulation->queue.requests = NULL;
	simulation->queue.size = 0;
	simulation->queue.capacity = 0;
	if (init_sync_primitives(simulation, config) != 0)
		return (1);
	if (alloc_simulation_arrays(simulation, config) != 0)
		return (1);
	if (init_coders(simulation, config) != 0)
		return (1);
	if (init_dongles(simulation, config) != 0)
		return (1);
	return (0);
}
