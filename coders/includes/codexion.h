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
	int				color_index;

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
	int					next_color_index;

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
void	destroy_dongle_mutexes(t_dongle_data *dongles, int count);

int		is_simulation_stopped(t_simulation_data *simulation);
void	mark_coder_finished(t_simulation_data *simulation,
			t_coder_data *coder);
int		get_compile_count(t_simulation_data *simulation,
			t_coder_data *coder);
int		enqueue_compile_request(t_coder_data *coder,
			t_simulation_data *simulation);
void	unlock_dongle(t_dongle_data *dongle, int cooldown);
void	get_coder_dongles(t_simulation_data *simulation,
			int coder_id, int *first, int *second);
int		reserve_dongles(t_simulation_data *simulation,
			int first, int second);
void	log_event(t_simulation_data *simulation, int coder_id,
			char *message);
int		drain_queue(t_simulation_data *simulation,
			t_compile_request *pending);
int		dispatch_pending(t_simulation_data *simulation,
			t_compile_request *pending, int pending_count);
void	*scheduler_routine(void *arg);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
int		wait_for_permission(t_coder_context *context);
int		take_dongles(t_coder_context *context, int first, int second);
void	compile_coder(t_coder_context *context);
void	release_dongles(t_coder_context *context, int first, int second);
int		finish_compile_cycle(t_coder_context *context);

int		init_priority_queue(t_priority_queue *queue, int capacity);
int		push_request(t_priority_queue *queue,
			t_compile_request request, t_scheduler scheduler);
int		pop_request(t_priority_queue *queue,
			t_compile_request *request, t_scheduler scheduler);
int		peek_request(t_priority_queue *queue,
			t_compile_request *request);
int		compare_requests(t_compile_request *first,
			t_compile_request *second, t_scheduler scheduler);
void	heapify_up(t_priority_queue *queue, int index,
			t_scheduler scheduler);
void	heapify_down(t_priority_queue *queue, int index,
			t_scheduler scheduler);

long	get_time_ms(void);

#endif