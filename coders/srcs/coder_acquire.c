/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_acquire.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:37:09 by tokinira          #+#    #+#             */
/*   Updated: 2026/09/29 08:37:09 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	wait_for_permission(t_coder_context *context)
{
	t_coder_data	*coder;

	coder = context->coder;
	pthread_mutex_lock(&context->simulation->state_mutex);
	while (!coder->has_permission
		&& !context->simulation->stop_simulation)
		pthread_cond_wait(&coder->cond,
			&context->simulation->state_mutex);
	if (context->simulation->stop_simulation)
	{
		pthread_mutex_unlock(&context->simulation->state_mutex);
		return (1);
	}
	coder->has_permission = 0;
	pthread_mutex_unlock(&context->simulation->state_mutex);
	return (0);
}

int	take_dongles(t_coder_context *context, int first, int second)
{
	t_simulation_data	*sim;

	sim = context->simulation;
	log_event(sim, context->coder->id, "has taken a dongle");
	log_event(sim, context->coder->id, "has taken a dongle");
	if (is_simulation_stopped(sim))
	{
		release_dongles(context, first, second);
		return (1);
	}
	return (0);
}
