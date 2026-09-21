#include "codexion.h"

static void	print_config(t_simulation_config *config)
{
	printf("coders: %d\n", config->number_of_coders);
	printf("burnout: %d\n", config->time_to_burnout);
	printf("compile: %d\n", config->time_to_compile);
	printf("debug: %d\n", config->time_to_debug);
	printf("refactor: %d\n", config->time_to_refactor);
	printf("required: %d\n", config->number_of_compiles_required);
	printf("cooldown: %d\n", config->dongle_cooldown);
	printf("scheduler: %d\n", config->scheduler);
}

static void	print_coders(t_simulation_data *simulation, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		printf("coder %d: count=%d, last=%ld, finished=%d\n",
			simulation->coders[i].id,
			simulation->coders[i].compile_count,
			simulation->coders[i].last_compile_start,
			simulation->coders[i].is_finished);
		i++;
	}
}

static int	run_simulation(t_simulation_config *config)
{
	t_simulation_data	simulation;

	if (init_simulation(&simulation, config))
	{
		printf("Error: simulation initialization failed\n");
		return (1);
	}
	if (start_simulation(&simulation))
	{
		printf("Error: simulation start failed\n");
		free_simulation(&simulation, config->number_of_coders,
			config->number_of_coders);
		return (1);
	}
	print_coders(&simulation, config->number_of_coders);
	free_simulation(&simulation, config->number_of_coders,
		config->number_of_coders);
	return (0);
}

int	main(int argc, char **argv)
{
	t_simulation_config	config;

	if (argc != 9)
	{
		printf("Error: invalid number of arguments\n");
		return (1);
	}
	if (parse_arguments(argv, &config))
	{
		printf("Error: invalid arguments\n");
		return (1);
	}
	print_config(&config);
	return (run_simulation(&config));
}
