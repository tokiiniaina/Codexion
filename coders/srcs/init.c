#include "codexion.h"

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;
	int	cond_count;

	cond_count = 0;
	simulation->stop_simulation = 0;
	simulation->start_time = 0;
	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->threads = NULL;
	simulation->finished_coders = 0;
	simulation->scheduler_thread = 0;
	simulation->request_counter = 0;
	simulation->queue.requests = NULL;
	simulation->queue.size = 0;
	simulation->queue.capacity = 0;
	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (pthread_mutex_init(&simulation->queue_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (pthread_cond_init(&simulation->queue_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (init_priority_queue(&simulation->queue,
			config->number_of_coders) != 0)
	{
		pthread_cond_destroy(&simulation->queue_cond);
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	simulation->coders = malloc(sizeof(t_coder_data)
			* config->number_of_coders);
	if (!simulation->coders)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	simulation->threads = malloc(sizeof(pthread_t)
			* config->number_of_coders);
	if (!simulation->threads)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].is_finished = 0;
		simulation->coders[i].has_permission = 0;
		if (pthread_cond_init(&simulation->coders[i].cond, NULL) != 0)
		{
			free_simulation(simulation, 0, cond_count);
			return (1);
		}
		cond_count++;
		i++;
	}
	simulation->dongles = malloc(sizeof(t_dongle_data)
			* config->number_of_coders);
	if (!simulation->dongles)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].is_available = 1;
		simulation->dongles[i].available_at = 0;
		simulation->dongles[i].is_reserved = 0;
		if (pthread_mutex_init(&simulation->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongle_mutexes(simulation->dongles, i);
			free_simulation(simulation, 0, cond_count);
			return (1);
		}
		i++;
	}
	return (0);
}
