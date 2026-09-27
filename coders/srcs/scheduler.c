#include "codexion.h"

#define SCHEDULER_WAIT_MS 50

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

static void	set_scheduler_timeout(struct timespec *timeout)
{
	clock_gettime(CLOCK_REALTIME, timeout);
	timeout->tv_nsec += SCHEDULER_WAIT_MS * 1000000L;
	if (timeout->tv_nsec >= 1000000000L)
	{
		timeout->tv_sec++;
		timeout->tv_nsec -= 1000000000L;
	}
}

static void	wait_for_retry(t_simulation_data *simulation)
{
	struct timespec	timeout;

	pthread_mutex_lock(&simulation->queue_mutex);
	if (!is_simulation_stopped(simulation))
	{
		set_scheduler_timeout(&timeout);
		pthread_cond_timedwait(&simulation->queue_cond,
			&simulation->queue_mutex, &timeout);
	}
	pthread_mutex_unlock(&simulation->queue_mutex);
}

void	signal_scheduler(t_simulation_data *simulation)
{
	pthread_mutex_lock(&simulation->queue_mutex);
	pthread_cond_signal(&simulation->queue_cond);
	pthread_mutex_unlock(&simulation->queue_mutex);
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
			wait_for_retry(simulation);
	}
	free(pending);
	return (NULL);
}
