/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_dispatch.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:21 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/13 00:00:00 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	grant_permission(t_simulation_data *simulation, int coder_id)
{
	t_coder_data	*coder;
	int				granted;

	coder = &simulation->coders[coder_id];
	pthread_mutex_lock(&simulation->state_mutex);
	granted = !simulation->stop_simulation;
	if (granted)
	{
		coder->has_permission = 1;
		pthread_cond_signal(&coder->cond);
	}
	pthread_mutex_unlock(&simulation->state_mutex);
	return (granted);
}

int	get_next_dongle_wakeup(t_simulation_data *simulation,
		long *wakeup_time)
{
	long	candidate;
	long	now;
	int		i;
	int		found;

	now = get_time_ms();
	i = 0;
	found = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_mutex_lock(&simulation->dongles[i].mutex);
		candidate = simulation->dongles[i].available_at;
		if (!simulation->dongles[i].is_reserved && candidate > now
			&& (!found || candidate < *wakeup_time))
		{
			*wakeup_time = candidate;
			found = 1;
		}
		pthread_mutex_unlock(&simulation->dongles[i].mutex);
		i++;
	}
	return (found);
}

int	dispatch_request(t_simulation_data *simulation,
		t_compile_request *request)
{
	int	first;
	int	second;

	get_coder_dongles(simulation, request->coder_id, &first, &second);
	if (!reserve_dongles(simulation, first, second))
		return (0);
	if (!grant_permission(simulation, request->coder_id))
	{
		cancel_dongles(simulation, first, second);
		return (0);
	}
	return (1);
}

int	drain_queue(t_simulation_data *simulation,
		t_compile_request *pending)
{
	int					pos;
	t_compile_request	request;

	pos = 0;
	while (pop_request(&simulation->queue, &request,
			simulation->config->scheduler) == 0)
	{
		pending[pos] = request;
		pos++;
	}
	return (pos);
}

int	dispatch_pending(t_simulation_data *simulation,
		t_compile_request *pending, int pending_count)
{
	int	i;
	int	dispatched;

	i = 0;
	dispatched = 0;
	while (i < pending_count)
	{
		if (dispatch_request(simulation, &pending[i]))
			dispatched++;
		else if (!is_simulation_stopped(simulation))
		{
			pthread_mutex_lock(&simulation->queue_mutex);
			if (push_request(&simulation->queue, pending[i],
					simulation->config->scheduler) != 0)
			{
				pthread_mutex_unlock(&simulation->queue_mutex);
				wake_everyone(simulation);
				i++;
				continue ;
			}
			pthread_mutex_unlock(&simulation->queue_mutex);
		}
		i++;
	}
	return (dispatched);
}