#include "codexion.h"

static void	wake_everyone(t_simulation_data *simulation)
{
	int	i;

	simulation->stop_simulation = 1;
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_cond_signal(&simulation->coders[i].cond);
		i++;
	}
	pthread_mutex_unlock(&simulation->state_mutex);
	pthread_mutex_lock(&simulation->queue_mutex);
	pthread_cond_broadcast(&simulation->queue_cond);
	pthread_mutex_unlock(&simulation->queue_mutex);
}

static int	find_burned_out(t_simulation_data *simulation,
		long current_time)
{
	int	i;

	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		if (!simulation->coders[i].is_finished
			&& current_time - simulation->coders[i].last_compile_start
			>= simulation->config->time_to_burnout)
			return (i);
		i++;
	}
	return (-1);
}

static int	monitor_tick(t_simulation_data *simulation, long current_time)
{
	int	burned;

	pthread_mutex_lock(&simulation->state_mutex);
	if (simulation->stop_simulation)
	{
		pthread_mutex_unlock(&simulation->state_mutex);
		return (1);
	}
	if (simulation->finished_coders == simulation->config->number_of_coders)
	{
		wake_everyone(simulation);
		return (1);
	}
	burned = find_burned_out(simulation, current_time);
	if (burned >= 0)
	{
		wake_everyone(simulation);
		log_event(simulation, simulation->coders[burned].id, "burned out");
		return (1);
	}
	pthread_mutex_unlock(&simulation->state_mutex);
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_simulation_data	*simulation;

	simulation = (t_simulation_data *)arg;
	while (1)
	{
		if (monitor_tick(simulation, get_time_ms()))
			return (NULL);
		usleep(1000);
	}
}
