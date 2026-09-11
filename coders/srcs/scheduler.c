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

void	*scheduler_routine(void *arg)
{
	t_simulation_data	*simulation;
	t_compile_request	*pending;
	int					pending_count;
	int					dispatched;

	simulation = (t_simulation_data *)arg;
	pending = malloc(sizeof(t_compile_request)
			* simulation->config->number_of_coders);
	if (!pending)
		return (NULL);
	while (!is_simulation_stopped(simulation))
	{
		if (wait_for_queue(simulation))
			break ;
		pending_count = drain_queue(simulation, pending);
		pthread_mutex_unlock(&simulation->queue_mutex);
		dispatched = dispatch_pending(simulation, pending, pending_count);
		if (dispatched < pending_count)
			usleep(1000);
	}
	free(pending);
	return (NULL);
}
