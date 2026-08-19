#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <limits.h>
# include <string.h>

typedef enum e_scheduler
{
	SCHEDULER_FIFO,
	SCHEDULER_EDF
}	t_scheduler;

typedef struct s_simulation_config
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	t_scheduler	scheduler;
}	t_simulation_config;

typedef struct s_coder_data
{
	int	id;
	int	compile_count;
	int	last_compile_start;
	int	is_finished;
}	t_coder_data;

typedef struct s_dongle_data
{
	int	id;
	int	is_available;
}	t_dongle_data;

typedef struct s_simulation_data
{
	t_simulation_config	*config;
	t_coder_data			*coders;
	t_dongle_data			*dongles;
	int						finished_coders;
}	t_simulation_data;

int		parse_arguments(char **argv, t_simulation_config *config);
int		init_simulation(t_simulation_data *simulation,
			t_simulation_config *config);
void	free_simulation(t_simulation_data *simulation);

#endif