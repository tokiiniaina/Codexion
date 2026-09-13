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
	t_dongle_data	*dongles;

	dongles = context->simulation->dongles;
	pthread_mutex_lock(&dongles[first].mutex);
	log_event(context->simulation, context->coder->id,
		"has taken a dongle");
	if (second != -1)
	{
		pthread_mutex_lock(&dongles[second].mutex);
		log_event(context->simulation, context->coder->id,
			"has taken a dongle");
	}
	else
	{
		while (!is_simulation_stopped(context->simulation))
			usleep(1000);
		unlock_dongle(&dongles[first],
			context->simulation->config->dongle_cooldown);
		return (1);
	}
	if (is_simulation_stopped(context->simulation))
	{
		if (second != -1)
			unlock_dongle(&dongles[second],
				context->simulation->config->dongle_cooldown);
		unlock_dongle(&dongles[first],
			context->simulation->config->dongle_cooldown);
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

void	release_dongles(t_coder_context *context, int first, int second)
{
	t_dongle_data	*dongles;

	dongles = context->simulation->dongles;
	if (second != -1)
		unlock_dongle(&dongles[second],
			context->simulation->config->dongle_cooldown);
	unlock_dongle(&dongles[first],
		context->simulation->config->dongle_cooldown);
}

int	finish_compile_cycle(t_coder_context *context)
{
	t_coder_data	*coder;

	coder = context->coder;
	if (is_simulation_stopped(context->simulation))
		return (1);
	log_event(context->simulation, coder->id, "is debugging");
	usleep(context->simulation->config->time_to_debug * 1000);
	if (is_simulation_stopped(context->simulation))
		return (1);
	log_event(context->simulation, coder->id, "is refactoring");
	usleep(context->simulation->config->time_to_refactor * 1000);
	if (is_simulation_stopped(context->simulation))
		return (1);
	pthread_mutex_lock(&context->simulation->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&context->simulation->state_mutex);
	pthread_mutex_lock(&context->simulation->state_mutex);
	if (coder->compile_count
		>= context->simulation->config->number_of_compiles_required)
	{
		pthread_mutex_unlock(&context->simulation->state_mutex);
		mark_coder_finished(context->simulation, coder);
	}
	else
		pthread_mutex_unlock(&context->simulation->state_mutex);
	return (0);
}
