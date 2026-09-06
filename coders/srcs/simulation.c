#include "codexion.h"


static	void destroy_coder_conditions(t_coder_data *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&coders[i].cond);
		i++;
	}
}


static	int	is_simulation_stopped(t_simulation_data *simulation)
{
	int	stopped;

	pthread_mutex_lock(&simulation->state_mutex);
	stopped = simulation->stop_simulation;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

static void	destroy_dongle_mutexes(t_dongle_data *dongles, int count)
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
			int	cond_count)
{
	if (simulation->coders)
	{
		destroy_coder_conditions(simulation->coders, cond_count);
		free(simulation->coders);
	}

	pthread_cond_destroy(&simulation->queue_cond);
	pthread_mutex_destroy(&simulation->queue_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);

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
	simulation->queue.requests = NULL;
	simulation->queue.size = 0;
	simulation->queue.capacity = 0;

	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->queue_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	if (pthread_cond_init(&simulation->queue_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	if (init_priority_queue(&simulation->queue,
			config->number_of_coders) != 0)
	{
		pthread_cond_destroy(&simulation->queue_cond);
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
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

static void	mark_coder_finished(t_simulation_data *simulation,
		t_coder_data *coder)
{
	pthread_mutex_lock(&simulation->state_mutex);
	if (!coder->is_finished)
	{
		coder->is_finished = 1;
		simulation->finished_coders++;
	}
	pthread_mutex_unlock(&simulation->state_mutex);
}

static void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	t_dongle_data	*dongles;
	int				first;
	int				second;
	int				temp;

	context = (t_coder_context *)arg;
	coder = context->coder;
	dongles = context->simulation->dongles;
	first = coder->id;
	second = (coder->id + 1)
		% context->simulation->config->number_of_coders;
	if (first > second)
	{
		temp = first;
		first = second;
		second = temp;
	}
	while (!is_simulation_stopped(context->simulation)
		&& coder->compile_count
		< context->simulation->config->number_of_compiles_required)
	{
		printf("Coder %d wants dongles %d and %d\n",
			coder->id, first, second);
		pthread_mutex_lock(&dongles[first].mutex);
		printf("Coder %d got dongle %d\n", coder->id, first);
		pthread_mutex_lock(&dongles[second].mutex);
		printf("Coder %d got dongle %d\n", coder->id, second);

		pthread_mutex_lock(&context->simulation->state_mutex);
		coder->last_compile_start = get_time_ms();
		pthread_mutex_unlock(&context->simulation->state_mutex);

		printf("Coder %d is using both dongles\n", coder->id);
		usleep(context->simulation->config->time_to_compile * 1000);

		pthread_mutex_lock(&context->simulation->state_mutex);
		coder->compile_count++;
		pthread_mutex_unlock(&context->simulation->state_mutex);

		pthread_mutex_unlock(&dongles[second].mutex);
		pthread_mutex_unlock(&dongles[first].mutex);

		if (coder->compile_count
			>= context->simulation->config->number_of_compiles_required)
			mark_coder_finished(context->simulation, coder);
	}
	return (NULL);
}

static void	*monitor_routine(void *arg)
{
	t_simulation_data	*simulation;
	int					i;
	long				current_time;
	int					all_finished;

	simulation = (t_simulation_data *)arg;
	while (1)
	{
		current_time = get_time_ms();
		pthread_mutex_lock(&simulation->state_mutex);
		all_finished = simulation->finished_coders
			== simulation->config->number_of_coders;
		if (all_finished)
		{
			simulation->stop_simulation = 1;
			pthread_mutex_unlock(&simulation->state_mutex);
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
				pthread_mutex_unlock(&simulation->state_mutex);
				printf("Coder %d burned out\n", simulation->coders[i].id);
				return (NULL);
			}
			i++;
		}
		pthread_mutex_unlock(&simulation->state_mutex);
		usleep(1000);
	}
}

int	start_simulation(t_simulation_data *simulation)
{
	t_coder_context	*contexts;
	int				i;

	contexts = malloc(sizeof(t_coder_context)
			* simulation->config->number_of_coders);
	if (!contexts)
		return (1);
	simulation->start_time = get_time_ms();
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		simulation->coders[i].last_compile_start = simulation->start_time;
		i++;
	}
	if (pthread_create(&simulation->monitor_thread, NULL,
			monitor_routine, simulation) != 0)
	{
		free(contexts);
		return (1);
	}
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		contexts[i].coder = &simulation->coders[i];
		contexts[i].simulation = simulation;
		if (pthread_create(&simulation->threads[i], NULL,
				coder_routine, &contexts[i]) != 0)
		{
			free(contexts);
			return (1);
		}
		i++;
	}
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_join(simulation->threads[i], NULL);
		i++;
	}
	pthread_join(simulation->monitor_thread, NULL);
	free(contexts);
	return (0);
}
