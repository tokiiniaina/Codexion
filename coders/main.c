#include "codexion.h"

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
	printf("number_of_coders: %d\n", config.number_of_coders);
	printf("time_to_burnout: %d\n", config.time_to_burnout);
	printf("time_to_compile: %d\n", config.time_to_compile);
	printf("time_to_debug: %d\n", config.time_to_debug);
	printf("time_to_refactor: %d\n", config.time_to_refactor);
	printf("number_of_compiles_required: %d\n",
		config.number_of_compiles_required);
	printf("dongle_cooldown: %d\n", config.dongle_cooldown);
	if (config.scheduler == SCHEDULER_FIFO)
		printf("scheduler: fifo\n");
	else
		printf("scheduler: edf\n");
	return (0);
}