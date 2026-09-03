#ifndef CODEXION_H
# define CODEXION_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>

typedef struct s_compile_request
{
	int	coder_id;
	int	deadline;
	int	arrival_order;
}	t_compile_request;

typedef struct s_priority_queue
{
	t_compile_request	*requests;
	int					size;
	int					capacity;
}	t_priority_queue;

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
	int				id;
	int				is_available;
	pthread_mutex_t mutex;
}	t_dongle_data;

typedef struct s_simulation_data
{
	t_simulation_config		*config;
	t_coder_data			*coders;
	t_dongle_data			*dongles;
	pthread_t				*threads;
	int						finished_coders;
}	t_simulation_data;


typedef struct s_coder_context
{
	t_coder_data		*coder;
	t_simulation_data	*simulation;
} t_coder_context;

int		parse_positive_number(char *value, int *result);
int		parse_arguments(char **argv, t_simulation_config *config);

int		init_simulation(t_simulation_data *simulation,
			t_simulation_config *config);
int		start_simulation(t_simulation_data *simulation);
void	free_simulation(t_simulation_data *simulation, int mutex_count);

int		init_priority_queue(t_priority_queue *queue, int capacity);
int		push_request(t_priority_queue *queue,
			t_compile_request request, t_scheduler scheduler);
int		pop_request(t_priority_queue *queue, t_compile_request *request,
			t_scheduler scheduler);

long	get_time_ms(void);

#endif
