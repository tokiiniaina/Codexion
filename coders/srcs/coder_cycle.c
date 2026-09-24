#include "codexion.h"

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
