/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:38:45 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/03 06:53:47 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	sleep_or_stop(t_simulation_data *simulation,
		t_coder_data *coder, int duration)
{
	long			end_time;
	struct timespec	ts;
	int				stopped;

	end_time = get_time_ms() + duration;
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->stop_simulation && get_time_ms() < end_time)
	{
		set_abs_timeout(end_time, &ts);
		pthread_cond_timedwait(&coder->cond, &simulation->state_mutex, &ts);
	}
	stopped = simulation->stop_simulation;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

int	compile_coder(t_coder_context *context)
{
	t_coder_data	*coder;

	coder = context->coder;
	pthread_mutex_lock(&context->simulation->state_mutex);
	coder->last_compile_start = get_time_ms();
	coder->burnout_deadline = coder->last_compile_start
		+ context->simulation->config->time_to_burnout;
	pthread_cond_signal(&context->simulation->monitor_cond);
	pthread_mutex_unlock(&context->simulation->state_mutex);
	log_event(context->simulation, coder->id, "is compiling");
	return (sleep_or_stop(context->simulation, coder,
			context->simulation->config->time_to_compile));
}

static int	coder_step(t_simulation_data *sim, t_coder_data *coder,
		char *msg, int duration)
{
	if (is_simulation_stopped(sim))
		return (1);
	log_event(sim, coder->id, msg);
	return (sleep_or_stop(sim, coder, duration));
}

int	finish_compile_cycle(t_coder_context *context)
{
	t_coder_data		*coder;
	t_simulation_data	*sim;
	int					finished;

	coder = context->coder;
	sim = context->simulation;
	if (coder_step(sim, coder, "is debugging", sim->config->time_to_debug))
		return (1);
	if (coder_step(sim, coder, "is refactoring",
			sim->config->time_to_refactor))
		return (1);
	if (is_simulation_stopped(sim))
		return (1);
	pthread_mutex_lock(&sim->state_mutex);
	coder->compile_count++;
	finished = coder->compile_count
		>= sim->config->number_of_compiles_required;
	pthread_cond_signal(&sim->monitor_cond);
	pthread_mutex_unlock(&sim->state_mutex);
	if (finished)
		mark_coder_finished(sim, coder);
	return (0);
}
