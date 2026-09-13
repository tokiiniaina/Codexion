#include "codexion.h"

static void	destroy_coder_conditions(t_coder_data *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&coders[i].cond);
		i++;
	}
}

void	destroy_dongle_mutexes(t_dongle_data *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&dongles[i].mutex);
		i++;
	}
}

void	free_simulation(t_simulation_data *simulation, int mutex_count,
		int cond_count)
{
	if (simulation->coders)
	{
		destroy_coder_conditions(simulation->coders, cond_count);
		free(simulation->coders);
	}
	pthread_cond_destroy(&simulation->queue_cond);
	pthread_mutex_destroy(&simulation->queue_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	if (simulation->queue.requests)
		free(simulation->queue.requests);
	if (simulation->dongles)
	{
		destroy_dongle_mutexes(simulation->dongles, mutex_count);
		free(simulation->dongles);
	}
	if (simulation->threads)
		free(simulation->threads);
}
