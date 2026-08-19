#include "codexion.h"

int	parse_positive_number(char *value, int *result)
{
	int	i;
	int	number;

	if (!value || !result || value[0] == '\0')
		return (1);
	i = 0;
	number = 0;
	while (value[i])
	{
		if (value[i] < '0' || value[i] > '9')
			return (1);
		if (number > (INT_MAX - (value[i] - '0')) / 10)
			return (1);
		number = number * 10 + (value[i] - '0');
		i++;
	}
	*result = number;
	return (0);
}

static int	parse_scheduler(char *value, t_scheduler *scheduler)
{
	if (strcmp(value, "fifo") == 0)
		*scheduler = SCHEDULER_FIFO;
	else if (strcmp(value, "edf") == 0)
		*scheduler = SCHEDULER_EDF;
	else
		return (1);
	return (0);
}

static int	parse_config_numbers(char **argv, t_simulation_config *config)
{
	if (parse_positive_number(argv[1], &config->number_of_coders))
		return (1);
	if (parse_positive_number(argv[2], &config->time_to_burnout))
		return (1);
	if (parse_positive_number(argv[3], &config->time_to_compile))
		return (1);
	if (parse_positive_number(argv[4], &config->time_to_debug))
		return (1);
	if (parse_positive_number(argv[5], &config->time_to_refactor))
		return (1);
	if (parse_positive_number(argv[6], &config->number_of_compiles_required))
		return (1);
	if (parse_positive_number(argv[7], &config->dongle_cooldown))
		return (1);
	return (0);
}

int	parse_arguments(char **argv, t_simulation_config *config)
{
	if (parse_config_numbers(argv, config))
		return (1);
	if (parse_scheduler(argv[8], &config->scheduler))
		return (1);
	if (config->number_of_coders < 1)
		return (1);
	return (0);
}
