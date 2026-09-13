#include "codexion.h"

void	*monitor_routine(void *arg)
{
	t_simulation_data	*simulation;
	int					i;
	int					j;
	long				current_time;
	int					all_finished;

	simulation = (t_simulation_data *)arg;
	while (1)
	{
		current_time = get_time_ms();
		pthread_mutex_lock(&simulation->state_mutex);
		if (simulation->stop_simulation)
		{
			pthread_mutex_unlock(&simulation->state_mutex);
			return (NULL);
		}
		all_finished = simulation->finished_coders
			== simulation->config->number_of_coders;
		if (all_finished)
		{
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
			return (NULL);
		}
		i = 0;
		while (i < simulation->config->number_of_coders)
		{
			if (!simulation->coders[i].is_finished
				&& current_time - simulation->coders[i].last_compile_start
				>= simulation->config->time_to_burnout)
			{
				simulation->stop_simulation = 1;
				j = 0;
				while (j < simulation->config->number_of_coders)
				{
					pthread_cond_signal(&simulation->coders[j].cond);
					j++;
				}
				pthread_mutex_unlock(&simulation->state_mutex);
				pthread_mutex_lock(&simulation->queue_mutex);
				pthread_cond_broadcast(&simulation->queue_cond);
				pthread_mutex_unlock(&simulation->queue_mutex);
				log_event(simulation, simulation->coders[i].id,
					"burned out");
				return (NULL);
			}
			i++;
		}
		pthread_mutex_unlock(&simulation->state_mutex);
		usleep(1000);
	}
}
