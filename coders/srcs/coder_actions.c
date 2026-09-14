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
	t_dongle_data		*dongles;
	t_simulation_data	*sim;
	int					cd;

	sim = context->simulation;
	dongles = sim->dongles;
	cd = sim->config->dongle_cooldown;
	pthread_mutex_lock(&dongles[first].mutex);
	log_event(sim, context->coder->id, "has taken a dongle");
	if (second == -1)
	{
		while (!is_simulation_stopped(sim))
			usleep(1000);
		unlock_dongle(&dongles[first], cd);
		return (1);
	}
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

void	compile_coder(t_coder_context *context)
{
	t_coder_data	*coder;

	coder = context->coder;
	pthread_mutex_lock(&context->simulation->state_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&context->simulation->state_mutex);
	log_event(context->simulation, coder->id, "is compiling");
	usleep(context->simulation->config->time_to_compile * 1000);
}

static int	coder_step(t_simulation_data *sim, t_coder_data *coder,
		char *msg, int duration)
{
	if (is_simulation_stopped(sim))
		return (1);
	log_event(sim, coder->id, msg);
	usleep(duration * 1000);
	return (0);
}

int	finish_compile_cycle(t_coder_context *context)
{
	t_coder_data		*coder;
	t_simulation_data	*sim;

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
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_lock(&sim->state_mutex);
	if (coder->compile_count >= sim->config->number_of_compiles_required)
	{
		pthread_mutex_unlock(&sim->state_mutex);
		mark_coder_finished(sim, coder);
	}
	else
		pthread_mutex_unlock(&sim->state_mutex);
	return (0);
}
