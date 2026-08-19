#include "codexion.h"

int	main(int argc, char **argv)
{
	t_simulation_config	config;
	t_simulation_data	simulation;

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
	printf("coders: %d\n", config.number_of_coders);
	printf("burnout: %d\n", config.time_to_burnout);
	printf("compile: %d\n", config.time_to_compile);
	printf("debug: %d\n", config.time_to_debug);
	printf("refactor: %d\n", config.time_to_refactor);
	printf("required: %d\n", config.number_of_compiles_required);
	printf("cooldown: %d\n", config.dongle_cooldown);
	printf("scheduler: %d\n", config.scheduler);

	if (init_simulation(&simulation, &config))
	{
		printf("Error: simulation initialization failed\n");
		return (1);
	}
	int	i;
	i = 0;
	while (i < config.number_of_coders)
	{
		printf("coder %d: count=%d, last=%d, finished=%d\n",
			simulation.coders[i].id,
			simulation.coders[i].compile_count,
			simulation.coders[i].last_compile_start,
			simulation.coders[i].is_finished);
		i++;
	}
	i = 0;
	while (i < config.number_of_coders)
	{
		printf("dongle %d: available=%d\n",
			simulation.dongles[i].id,
			simulation.dongles[i].is_available);
		i++;
	}
	free_simulation(&simulation);
	return (0);
}