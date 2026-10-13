/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:03 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/02 22:07:52 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	put_back_dongle(t_dongle_data *dongle, int cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_available = 0;
	dongle->is_reserved = 0;
	dongle->available_at = get_time_ms() + cooldown;
	pthread_mutex_unlock(&dongle->mutex);
}

void	get_coder_dongles(t_simulation_data *simulation,
		int coder_id, int *first, int *second)
{
	int	temp;

	*first = coder_id;
	if (simulation->config->number_of_coders == 1)
	{
		*second = -1;
		return ;
	}
	*second = (coder_id + 1)
		% simulation->config->number_of_coders;
	if (*first > *second)
	{
		temp = *first;
		*first = *second;
		*second = temp;
	}
}

static int	reserve_dongle(t_simulation_data *sim, int idx,
		long current_time)
{
	pthread_mutex_lock(&sim->dongles[idx].mutex);
	if (sim->dongles[idx].is_reserved)
	{
		pthread_mutex_unlock(&sim->dongles[idx].mutex);
		return (0);
	}
	if (!sim->dongles[idx].is_available)
	{
		if (current_time < sim->dongles[idx].available_at)
		{
			pthread_mutex_unlock(&sim->dongles[idx].mutex);
			return (0);
		}
		sim->dongles[idx].is_available = 1;
	}
	sim->dongles[idx].is_reserved = 1;
	sim->dongles[idx].is_available = 0;
	pthread_mutex_unlock(&sim->dongles[idx].mutex);
	return (1);
}

int	reserve_dongles(t_simulation_data *simulation,
		int first, int second)
{
	long	current_time;

	current_time = get_time_ms();
	if (!reserve_dongle(simulation, first, current_time))
		return (0);
	if (second != -1)
	{
		if (!reserve_dongle(simulation, second, current_time))
		{
			pthread_mutex_lock(&simulation->dongles[first].mutex);
			simulation->dongles[first].is_reserved = 0;
			simulation->dongles[first].is_available = 1;
			pthread_mutex_unlock(&simulation->dongles[first].mutex);
			return (0);
		}
	}
	return (1);
}

void	cancel_dongles(t_simulation_data *simulation, int first, int second)
{
	pthread_mutex_lock(&simulation->dongles[first].mutex);
	simulation->dongles[first].is_reserved = 0;
	simulation->dongles[first].is_available = 1;
	pthread_mutex_unlock(&simulation->dongles[first].mutex);
	if (second != -1)
	{
		pthread_mutex_lock(&simulation->dongles[second].mutex);
		simulation->dongles[second].is_reserved = 0;
		simulation->dongles[second].is_available = 1;
		pthread_mutex_unlock(&simulation->dongles[second].mutex);
	}
}

void	release_dongles(t_coder_context *context, int first, int second)
{
	t_dongle_data	*dongles;

	dongles = context->simulation->dongles;
	if (second != -1)
		put_back_dongle(&dongles[second],
			context->simulation->config->dongle_cooldown);
	put_back_dongle(&dongles[first],
		context->simulation->config->dongle_cooldown);
	signal_scheduler(context->simulation);
}
