/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:38:49 by tokinira          #+#    #+#             */
/*   Updated: 2026/09/29 08:38:49 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	run_coder_task(t_coder_context *context, int first, int second)
{
	if (enqueue_compile_request(context->coder, context->simulation) != 0)
		return (1);
	if (wait_for_permission(context))
		return (1);
	if (take_dongles(context, first, second))
		return (1);
	if (compile_coder(context))
	{
		release_dongles(context, first, second);
		return (1);
	}
	release_dongles(context, first, second);
	if (finish_compile_cycle(context))
		return (1);
	return (0);
}

static void	*run_lonely_coder(t_coder_context *context)
{
	t_simulation_data	*simulation;
	t_coder_data		*coder;

	simulation = context->simulation;
	coder = context->coder;
	log_event(simulation, coder->id, "has taken a dongle");
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->stop_simulation)
		pthread_cond_wait(&coder->cond, &simulation->state_mutex);
	pthread_mutex_unlock(&simulation->state_mutex);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	int				first;
	int				second;

	context = (t_coder_context *)arg;
	coder = context->coder;
	get_coder_dongles(context->simulation, coder->id, &first, &second);
	if (get_compile_count(context->simulation, coder)
		>= context->simulation->config->number_of_compiles_required)
	{
		mark_coder_finished(context->simulation, coder);
		return (NULL);
	}
	if (context->simulation->config->number_of_coders == 1)
		return (run_lonely_coder(context));
	while (!is_simulation_stopped(context->simulation)
		&& get_compile_count(context->simulation, coder)
		< context->simulation->config->number_of_compiles_required)
	{
		if (run_coder_task(context, first, second))
			return (NULL);
	}
	return (NULL);
}
