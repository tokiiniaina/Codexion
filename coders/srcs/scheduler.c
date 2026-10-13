/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:39:23 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/02 22:11:15 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_for_queue(t_simulation_data *simulation)
{
	pthread_mutex_lock(&simulation->queue_mutex);
	while (simulation->queue.size == 0)
	{
		if (is_simulation_stopped(simulation))
		{
			pthread_mutex_unlock(&simulation->queue_mutex);
			return (1);
		}
		pthread_cond_wait(&simulation->queue_cond,
			&simulation->queue_mutex);
	}
	return (0);
}

static void	wait_for_retry(t_simulation_data *simulation)
{
	struct timespec	timeout;
	long			wakeup_time;

	pthread_mutex_lock(&simulation->queue_mutex);
	if (!is_simulation_stopped(simulation))
	{
		if (get_next_dongle_wakeup(simulation, &wakeup_time))
		{
			set_abs_timeout(wakeup_time, &timeout);
			pthread_cond_timedwait(&simulation->queue_cond,
				&simulation->queue_mutex, &timeout);
		}
		else
			pthread_cond_wait(&simulation->queue_cond,
				&simulation->queue_mutex);
	}
	pthread_mutex_unlock(&simulation->queue_mutex);
}

void	signal_scheduler(t_simulation_data *simulation)
{
	pthread_mutex_lock(&simulation->queue_mutex);
	pthread_cond_signal(&simulation->queue_cond);
	pthread_mutex_unlock(&simulation->queue_mutex);
}

void	*scheduler_routine(void *arg)
{
	t_simulation_data	*simulation;
	t_compile_request	*pending;
	int				pending_count;
	int				dispatched;

	simulation = (t_simulation_data *)arg;
	pending = malloc(sizeof(t_compile_request)
			* simulation->config->number_of_coders);
	if (!pending)
	{
		wake_everyone(simulation);
		return (NULL);
	}
	while (!is_simulation_stopped(simulation))
	{
		if (wait_for_queue(simulation))
			break ;
		pending_count = drain_queue(simulation, pending);
		pthread_mutex_unlock(&simulation->queue_mutex);
		dispatched = dispatch_pending(simulation, pending, pending_count);
		if (dispatched < pending_count && !is_simulation_stopped(simulation))
			wait_for_retry(simulation);
	}
	free(pending);
	return (NULL);
}
