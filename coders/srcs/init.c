#include "codexion.h"

static int	init_sync_primitives(t_simulation_data *simulation,
		t_simulation_config *config)
{
	pthread_mutex_t	*mtx[3];
	int				i;

	mtx[0] = &simulation->log_mutex;
	mtx[1] = &simulation->state_mutex;
	mtx[2] = &simulation->queue_mutex;
	i = -1;
	while (++i < 3 && pthread_mutex_init(mtx[i], NULL) == 0)
		;
	if (i == 3 && pthread_cond_init(&simulation->queue_cond, NULL) == 0)
	{
		if (init_priority_queue(&simulation->queue,
				config->number_of_coders) == 0)
			return (0);
		pthread_cond_destroy(&simulation->queue_cond);
	}
	while (--i >= 0)
		pthread_mutex_destroy(mtx[i]);
	return (1);
}

static int	alloc_simulation_arrays(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	n;

	n = config->number_of_coders;
	simulation->coders = malloc(sizeof(t_coder_data) * n);
	if (!simulation->coders)
	{
		free_simulation(simulation, 0, 0);
		return (1);
	}
	simulation->threads = malloc(sizeof(pthread_t) * n);
	if (!simulation->threads)
	{
		free_simulation(simulation, 0, 0);
		return (1);
	}
	simulation->dongles = malloc(sizeof(t_dongle_data) * n);
	if (!simulation->dongles)
	{
		free_simulation(simulation, 0, 0);
		return (1);
	}
	return (0);
}

static int	init_coders(t_simulation_data *simulation,
		t_simulation_config *config, int *cond_count)
{
	int	i;

	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].is_finished = 0;
		simulation->coders[i].has_permission = 0;
		simulation->coders[i].color_index = 0;
		if (pthread_cond_init(&simulation->coders[i].cond, NULL) != 0)
		{
			free_simulation(simulation, 0, *cond_count);
			return (1);
		}
		(*cond_count)++;
		i++;
	}
	return (0);
}

static int	init_dongles(t_simulation_data *simulation,
		t_simulation_config *config, int cond_count)
{
	int	i;

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

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	cond_count;

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
	simulation->next_color_index = 0;
	simulation->queue.capacity = 0;
	cond_count = 0;
	if (init_sync_primitives(simulation, config) != 0)
		return (1);
	if (alloc_simulation_arrays(simulation, config) != 0)
		return (1);
	if (init_coders(simulation, config, &cond_count) != 0)
		return (1);
	if (init_dongles(simulation, config, cond_count) != 0)
		return (1);
	return (0);
}
