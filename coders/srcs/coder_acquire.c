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

static void	assign_session_color(t_coder_context *context)
{
	t_simulation_data	*sim;

	sim = context->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	context->coder->color_index = sim->next_color_index;
	sim->next_color_index = 1 - sim->next_color_index;
	pthread_mutex_unlock(&sim->state_mutex);
}

static int	wait_alone_until_stop(t_simulation_data *sim,
		t_dongle_data *dongle, int cd)
{
	while (!is_simulation_stopped(sim))
		usleep(1000);
	unlock_dongle(dongle, cd);
	return (1);
}

int	take_dongles(t_coder_context *context, int first, int second)
{
	t_dongle_data		*dongles;
	t_simulation_data	*sim;
	int					cd;

	sim = context->simulation;
	dongles = sim->dongles;
	cd = sim->config->dongle_cooldown;
	assign_session_color(context);
	pthread_mutex_lock(&dongles[first].mutex);
	log_event(sim, context->coder->id, "has taken a dongle");
	if (second == -1)
		return (wait_alone_until_stop(sim, &dongles[first], cd));
	pthread_mutex_lock(&dongles[second].mutex);
	log_event(sim, context->coder->id, "has taken a dongle");
	if (is_simulation_stopped(sim))
	{
		unlock_dongle(&dongles[second], cd);
		unlock_dongle(&dongles[first], cd);
		return (1);
	}
	return (0);
}
