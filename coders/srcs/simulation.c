#include "codexion.h"

void	free_simulation(t_simulation_data *simulation)
{
	if (simulation->coders)
		free(simulation->coders);
	if (simulation->dongles)
		free(simulation->dongles);
}

int	init_simulation(t_simulation_data *simulation, t_simulation_config *config)
{
	int	i;

	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->finished_coders = 0;
	simulation->coders = malloc(sizeof(t_coder_data)
			* config->number_of_coders);
	if (!simulation->coders)
		return (1);
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
		free_simulation(simulation);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].is_available = 1;
		i++;
	}
	return (0);
}
