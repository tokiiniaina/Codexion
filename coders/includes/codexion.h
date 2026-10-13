/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tokinira <tokinira@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 08:41:35 by tokinira          #+#    #+#             */
/*   Updated: 2026/10/02 22:11:15 by tokinira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
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
	long			burnout_deadline;
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

	int					inited_coders;
	int					inited_dongles;
	pthread_mutex_t		state_mutex;

	int					stop_simulation;
	long				start_time;

	t_priority_queue	queue;
	pthread_mutex_t		queue_mutex;
	pthread_cond_t		queue_cond;

	pthread_cond_t		monitor_cond;


	pthread_t			scheduler_thread;

	int					request_counter;

	pthread_mutex_t		log_mutex;
}	t_simulation_data;

typedef struct s_coder_context
{
	t_coder_data		*coder;
	t_simulation_data	*simulation;
}	t_coder_context;

void	wake_everyone(t_simulation_data *simulation);

int		parse_arguments(char **argv, t_simulation_config *config);

int		init_simulation(t_simulation_data *simulation,
			t_simulation_config *config);
int		init_sync_primitives(t_simulation_data *simulation,
			t_simulation_config *config);
int		start_simulation(t_simulation_data *simulation);
void	free_simulation(t_simulation_data *simulation);

int		is_simulation_stopped(t_simulation_data *simulation);
void	mark_coder_finished(t_simulation_data *simulation,
			t_coder_data *coder);
int		get_compile_count(t_simulation_data *simulation,
			t_coder_data *coder);
int		enqueue_compile_request(t_coder_data *coder,
			t_simulation_data *simulation);
void	unlock_dongle(t_dongle_data *dongle, int cooldown);
void	cancel_dongles(t_simulation_data *simulation, int first, int second);
int		get_next_dongle_wakeup(t_simulation_data *simulation,
			long *wakeup_time);
void	get_coder_dongles(t_simulation_data *simulation,
			int coder_id, int *first, int *second);
int		reserve_dongles(t_simulation_data *simulation,
			int first, int second);
void	log_event(t_simulation_data *simulation, int coder_id,
			char *message);
int		dispatch_request(t_simulation_data *simulation,
			t_compile_request *request);
int		drain_queue(t_simulation_data *simulation,
			t_compile_request *pending);
int		dispatch_pending(t_simulation_data *simulation,
			t_compile_request *pending, int pending_count);
void	*scheduler_routine(void *arg);
void	signal_scheduler(t_simulation_data *simulation);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
int		wait_for_permission(t_coder_context *context);
int		take_dongles(t_coder_context *context, int first, int second);
int		compile_coder(t_coder_context *context);
void	release_dongles(t_coder_context *context, int first, int second);
int		finish_compile_cycle(t_coder_context *context);

int		init_priority_queue(t_priority_queue *queue, int capacity);
int		push_request(t_priority_queue *queue,
			t_compile_request request, t_scheduler scheduler);
int		pop_request(t_priority_queue *queue,
			t_compile_request *request, t_scheduler scheduler);
void	heapify_up(t_priority_queue *queue, int index,
			t_scheduler scheduler);
void	heapify_down(t_priority_queue *queue, int index,
			t_scheduler scheduler);

long	get_time_ms(void);
void	set_abs_timeout(long target_ms, struct timespec *ts);

#endif