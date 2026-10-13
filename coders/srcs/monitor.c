/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:15 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/13 00:00:00 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	find_burned_out(t_simulation_data *simulation, long current_time)
{
	int	i;

	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		if (!simulation->coders[i].is_finished
			&& current_time >= simulation->coders[i].burnout_deadline)
			return (i);
		i++;
	}
	return (-1);
}

static long	find_next_deadline(t_simulation_data *simulation)
{
	long	deadline;
	int		i;

	deadline = LONG_MAX;
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		if (!simulation->coders[i].is_finished
			&& simulation->coders[i].burnout_deadline < deadline)
			deadline = simulation->coders[i].burnout_deadline;
		i++;
	}
	return (deadline);
}

void	*monitor_routine(void *arg)
{
	t_simulation_data	*simulation;
	struct timespec	timeout;
	long				current_time;
	long				next_deadline;
	int				burned;
	int				burned_id;

	simulation = (t_simulation_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&simulation->state_mutex);
		if (simulation->stop_simulation)
		{
			pthread_mutex_unlock(&simulation->state_mutex);
			return (NULL);
		}
		if (simulation->finished_coders
			== simulation->config->number_of_coders)
		{
			pthread_mutex_unlock(&simulation->state_mutex);
			wake_everyone(simulation);
			return (NULL);
		}
		current_time = get_time_ms();
		burned = find_burned_out(simulation, current_time);
		if (burned >= 0)
		{
			burned_id = simulation->coders[burned].id;
			simulation->stop_simulation = 1;
			pthread_mutex_unlock(&simulation->state_mutex);
			wake_everyone(simulation);
			log_event(simulation, burned_id, "burned out");
			return (NULL);
		}
		next_deadline = find_next_deadline(simulation);
		set_abs_timeout(next_deadline, &timeout);
		pthread_cond_timedwait(&simulation->monitor_cond,
			&simulation->state_mutex, &timeout);
		pthread_mutex_unlock(&simulation->state_mutex);
	}
}