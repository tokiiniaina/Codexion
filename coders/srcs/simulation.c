#include "codexion.h"

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

void	free_simulation(t_simulation_data *simulation, int mutex_count)
{
	if (simulation->dongles)
	{
		destroy_dongle_mutexes(simulation->dongles, mutex_count);
		free(simulation->dongles);
	}
	if (simulation->coders)
		free(simulation->coders);
	if (simulation->threads)
		free(simulation->threads);
}

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;

	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->threads = NULL;
	simulation->finished_coders = 0;
	simulation->coders = malloc(sizeof(t_coder_data)
		* config->number_of_coders);
	if (!simulation->coders)
		return (1);
	simulation->threads = malloc(sizeof(pthread_t)
			* config->number_of_coders);
	if (!simulation->threads)
	{
		free_simulation(simulation, 0);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].is_finished = 0;
		i++;
	}
	simulation->dongles = malloc(sizeof(t_dongle_data)
			* config->number_of_coders);
	if (!simulation->dongles)
	{
		free_simulation(simulation, 0);
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
			free_simulation(simulation, 0);
			return (1);
		}
		i++;
	}
	return (0);
}

static void	*coder_routine(void *arg)
{
	t_coder_data	*coder;

	coder = (t_coder_data *)arg;
	printf("Coder %d is running\n", coder->id);
	return (NULL);
}

int	start_simulation(t_simulation_data *simulation)
{
	int	i;

	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		if (pthread_create(&simulation->threads[i], NULL,
				coder_routine, &simulation->coders[i]) != 0)
			return (1);
		i++;
	}
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_join(simulation->threads[i], NULL);
		i++;
	}
	return (0);
}
