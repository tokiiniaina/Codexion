#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_compile_request
{
	int		coder_id;
	long	deadline;
	int		arrival_order;
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
	int				id;
	int				compile_count;
	long			last_compile_start;
	int				is_finished;

	pthread_cond_t	cond;
	int				has_permission;	
}	t_coder_data;

typedef struct s_dongle_data
{
	int				id;
	int				is_available;
	int				is_reserved;
	long			available_at;
	pthread_mutex_t	mutex;
}	t_dongle_data;

typedef struct s_simulation_data
{
	t_simulation_config	*config;
	t_coder_data		*coders;
	t_dongle_data		*dongles;
	pthread_t			*threads;

	int					finished_coders;
	pthread_t			monitor_thread;

	pthread_mutex_t		state_mutex;

	int					stop_simulation;
	long				start_time;

	t_priority_queue	queue;
	pthread_mutex_t		queue_mutex;
	pthread_cond_t		queue_cond;

	pthread_t			scheduler_thread;

	int					request_counter;

	pthread_mutex_t		log_mutex;
}	t_simulation_data;

typedef struct s_coder_context
{
	t_coder_data		*coder;
	t_simulation_data	*simulation;
}	t_coder_context;

int		parse_positive_number(char *value, int *result);
int		parse_arguments(char **argv, t_simulation_config *config);

int		init_simulation(t_simulation_data *simulation,
			t_simulation_config *config);
int		start_simulation(t_simulation_data *simulation);
void	free_simulation(t_simulation_data *simulation,
			int mutex_count, int cond_count);

int		init_priority_queue(t_priority_queue *queue, int capacity);
int		push_request(t_priority_queue *queue,
			t_compile_request request, t_scheduler scheduler);
int		pop_request(t_priority_queue *queue,
			t_compile_request *request, t_scheduler scheduler);
int		peek_request(t_priority_queue *queue,
		t_compile_request *request);

long	get_time_ms(void);

#endif