#include "codexion.h"

static void	stop_and_wake(t_simulation_data *sim)
{
	int	i;

	pthread_mutex_lock(&sim->state_mutex);
	sim->stop_simulation = 1;
	i = -1;
	while (++i < sim->config->number_of_coders)
		pthread_cond_signal(&sim->coders[i].cond);
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_lock(&sim->queue_mutex);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_mutex);
}

static void	init_contexts(t_simulation_data *sim, t_coder_context *ctx)
{
	int	i;

	sim->start_time = get_time_ms();
	i = -1;
	while (++i < sim->config->number_of_coders)
	{
		sim->coders[i].last_compile_start = sim->start_time;
		ctx[i].coder = &sim->coders[i];
		ctx[i].simulation = sim;
	}
}

static void	join_threads(t_simulation_data *sim, int count, int stop_first)
{
	if (stop_first)
		stop_and_wake(sim);
	while (count > 0)
	{
		count--;
		pthread_join(sim->threads[count], NULL);
	}
	pthread_join(sim->scheduler_thread, NULL);
	pthread_join(sim->monitor_thread, NULL);
}

static int	spawn_all(t_simulation_data *sim, t_coder_context *ctx)
{
	int	i;

	if (pthread_create(&sim->monitor_thread, NULL,
			monitor_routine, sim) != 0)
		return (1);
	if (pthread_create(&sim->scheduler_thread, NULL,
			scheduler_routine, sim) != 0)
	{
		stop_and_wake(sim);
		pthread_join(sim->monitor_thread, NULL);
		return (1);
	}
	i = -1;
	while (++i < sim->config->number_of_coders)
	{
		if (pthread_create(&sim->threads[i], NULL,
				coder_routine, &ctx[i]) != 0)
		{
			join_threads(sim, i, 1);
			return (1);
		}
	}
	return (0);
}

int	start_simulation(t_simulation_data *simulation)
{
	t_coder_context	*contexts;

	contexts = malloc(sizeof(t_coder_context)
			* simulation->config->number_of_coders);
	if (!contexts)
		return (1);
	init_contexts(simulation, contexts);
	if (spawn_all(simulation, contexts))
	{
		free(contexts);
		return (1);
	}
	join_threads(simulation, simulation->config->number_of_coders, 0);
	free(contexts);
	return (0);
}
