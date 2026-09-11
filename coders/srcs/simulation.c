#include "codexion.h"


int	peek_request(t_priority_queue *queue, t_compile_request *request)
{
	if (queue->size == 0)
		return (1);
	*request = queue->requests[0];
	return (0);
}


static void	destroy_coder_conditions(t_coder_data *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&coders[i].cond);
		i++;
	}
}

static int	is_simulation_stopped(t_simulation_data *simulation)
{
	int	stopped;

	pthread_mutex_lock(&simulation->state_mutex);
	stopped = simulation->stop_simulation;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

static void	destroy_dongle_mutexes(t_dongle_data *dongles, int count)
{
	int	i;

	i = 0;
 	while (i < count)
   	{
		pthread_mutex_destroy(&dongles[i].mutex);
		i++;
	}
}

void	free_simulation(t_simulation_data *simulation, int mutex_count,
		int cond_count)
{
	if (simulation->coders)
	{
		destroy_coder_conditions(simulation->coders, cond_count);
		free(simulation->coders);
	}
	pthread_cond_destroy(&simulation->queue_cond);
	pthread_mutex_destroy(&simulation->queue_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	if (simulation->queue.requests)
		free(simulation->queue.requests);
	if (simulation->dongles)
	{
		destroy_dongle_mutexes(simulation->dongles, mutex_count);
		free(simulation->dongles);
	}
	if (simulation->threads)
		free(simulation->threads);
}

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;
	int	cond_count;

	cond_count = 0;
	simulation->stop_simulation = 0;
	simulation->start_time = 0;
	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->threads = NULL;
	simulation->finished_coders = 0;
	simulation->scheduler_thread = 0;
	simulation->request_counter = 0;
	simulation->queue.requests = NULL;
	simulation->queue.size = 0;
	simulation->queue.capacity = 0;

	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (pthread_mutex_init(&simulation->queue_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (pthread_cond_init(&simulation->queue_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	if (init_priority_queue(&simulation->queue,
			config->number_of_coders) != 0)
	{
		pthread_cond_destroy(&simulation->queue_cond);
		pthread_mutex_destroy(&simulation->queue_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (1);
	}
	simulation->coders = malloc(sizeof(t_coder_data)
			* config->number_of_coders);
	if (!simulation->coders)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	simulation->threads = malloc(sizeof(pthread_t)
			* config->number_of_coders);
	if (!simulation->threads)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].is_finished = 0;
		simulation->coders[i].has_permission = 0;
		if (pthread_cond_init(&simulation->coders[i].cond, NULL) != 0)
		{
			free_simulation(simulation, 0, cond_count);
			return (1);
		}
		cond_count++;
		i++;
	}
	simulation->dongles = malloc(sizeof(t_dongle_data)
			* config->number_of_coders);
	if (!simulation->dongles)
	{
		free_simulation(simulation, 0, cond_count);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].is_available = 1;
		simulation->dongles[i].available_at = 0;
		simulation->dongles[i].is_reserved = 0;
		if (pthread_mutex_init(&simulation->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongle_mutexes(simulation->dongles, i);
			free_simulation(simulation, 0, cond_count);
			return (1);
		}
		i++;
	}
	return (0);
}

static void	mark_coder_finished(t_simulation_data *simulation,
		t_coder_data *coder)
{
	pthread_mutex_lock(&simulation->state_mutex);
	if (!coder->is_finished)
	{
		coder->is_finished = 1;
		simulation->finished_coders++;
	}
	pthread_mutex_unlock(&simulation->state_mutex);
}

static void unlock_dongle(t_dongle_data *dongle, int cooldown)
{
	dongle->is_available = 0;
	dongle->is_reserved = 0;
	dongle->available_at = get_time_ms() + cooldown;
	pthread_mutex_unlock(&dongle->mutex);
}

static int get_compile_count(t_simulation_data *simulation,
		t_coder_data *coder)
{
	int	compile_count;

	pthread_mutex_lock(&simulation->state_mutex);
	compile_count = coder->compile_count;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (compile_count);
}

static int enqueue_compile_request(t_coder_data *coder, 
	t_simulation_data *simulation)
{
	t_compile_request	request;
	long				last_compile_start;

	pthread_mutex_lock(&simulation->state_mutex);
	last_compile_start = coder->last_compile_start;
	pthread_mutex_unlock(&simulation->state_mutex);
	request.coder_id = coder->id;
	request.deadline = last_compile_start
		+ simulation->config->time_to_burnout;

	pthread_mutex_lock(&simulation->queue_mutex);
	request.arrival_order = simulation->request_counter;
	simulation->request_counter++;
	if (push_request(&simulation->queue, request,
			simulation->config->scheduler) != 0)
	{
		pthread_mutex_unlock(&simulation->queue_mutex);
		return (1);
	}
	pthread_cond_signal(&simulation->queue_cond);
	pthread_mutex_unlock(&simulation->queue_mutex);
	return (0);
}


static void	get_coder_dongles(t_simulation_data *simulation,
		int coder_id, int *first, int *second)
{
	int	temp;

	*first = coder_id;
	if (simulation->config->number_of_coders == 1)
	{
		*second = -1;
		return ;
	}
	*second = (coder_id + 1)
		% simulation->config->number_of_coders;
	if (*first > *second)
	{
		temp = *first;
		*first = *second;
		*second = temp;
	}
}


static int	reserve_dongles(t_simulation_data *simulation,
				int first, int second)
{
	long	current_time;

	current_time = get_time_ms();

	pthread_mutex_lock(&simulation->dongles[first].mutex);
	if (simulation->dongles[first].is_reserved)
	{
		pthread_mutex_unlock(&simulation->dongles[first].mutex);
		return (0);
	}
	if (!simulation->dongles[first].is_available)
	{
		if (current_time < simulation->dongles[first].available_at)
		{
			pthread_mutex_unlock(&simulation->dongles[first].mutex);
			return (0);
		}
		simulation->dongles[first].is_available = 1;
	}
	simulation->dongles[first].is_reserved = 1;
	simulation->dongles[first].is_available = 0;
	pthread_mutex_unlock(&simulation->dongles[first].mutex);

	if (second != -1)
	{
		pthread_mutex_lock(&simulation->dongles[second].mutex);
		if (simulation->dongles[second].is_reserved)
		{
			pthread_mutex_unlock(&simulation->dongles[second].mutex);
			pthread_mutex_lock(&simulation->dongles[first].mutex);
			simulation->dongles[first].is_reserved = 0;
			simulation->dongles[first].is_available = 1;
			pthread_mutex_unlock(&simulation->dongles[first].mutex);
			return (0);
		}
		if (!simulation->dongles[second].is_available)
		{
			if (current_time < simulation->dongles[second].available_at)
			{
				pthread_mutex_unlock(&simulation->dongles[second].mutex);
				pthread_mutex_lock(&simulation->dongles[first].mutex);
				simulation->dongles[first].is_reserved = 0;
				simulation->dongles[first].is_available = 1;
				pthread_mutex_unlock(&simulation->dongles[first].mutex);
				return (0);
			}
			simulation->dongles[second].is_available = 1;
		}
		simulation->dongles[second].is_reserved = 1;
		simulation->dongles[second].is_available = 0;
		pthread_mutex_unlock(&simulation->dongles[second].mutex);
	}
	return (1);
}


static void	log_event(t_simulation_data *simulation, int coder_id,
		char *message)
{
	long	timestamp;

	timestamp = get_time_ms() - simulation->start_time;
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ld %d %s\n", timestamp, coder_id + 1, message);
	pthread_mutex_unlock(&simulation->log_mutex);
}

static void	grant_permission(t_simulation_data *simulation, int coder_id)
{
	t_coder_data	*coder;

	coder = &simulation->coders[coder_id];
	pthread_mutex_lock(&simulation->state_mutex);
	if (!simulation->stop_simulation)
	{
		coder->last_compile_start = get_time_ms();
		coder->has_permission = 1;
		pthread_cond_signal(&coder->cond);
	}
	pthread_mutex_unlock(&simulation->state_mutex);
}

static int	try_dispatch_request(t_simulation_data *simulation,
		t_compile_request *request)
{
	int	first;
	int	second;

	get_coder_dongles(simulation, request->coder_id, &first, &second);
	if (!reserve_dongles(simulation, first, second))
		return (0);
	grant_permission(simulation, request->coder_id);
	return (1);
}

static int	drain_queue(t_simulation_data *simulation,
		t_compile_request *pending)
{
	int					count;
	t_compile_request	request;

	count = 0;
	while (pop_request(&simulation->queue, &request,
			simulation->config->scheduler) == 0)
	{
		pending[count] = request;
		count++;
	}
	return (count);
}

static int	dispatch_pending(t_simulation_data *simulation,
		t_compile_request *pending, int pending_count)
{
	int	i;
	int	dispatched;

	i = 0;
	dispatched = 0;
	while (i < pending_count)
	{
		if (try_dispatch_request(simulation, &pending[i]))
			dispatched++;
		else
		{
			pthread_mutex_lock(&simulation->queue_mutex);
			push_request(&simulation->queue, pending[i],
				simulation->config->scheduler);
			pthread_mutex_unlock(&simulation->queue_mutex);
		}
		i++;
	}
	return (dispatched);
}

static int	wait_for_queue(t_simulation_data *simulation)
{
	pthread_mutex_lock(&simulation->queue_mutex);
	while (simulation->queue.size == 0)
	{
		if (is_simulation_stopped(simulation))
		{
			pthread_mutex_unlock(&simulation->queue_mutex);
			return (1);
		}
		pthread_cond_wait(&simulation->queue_cond,
			&simulation->queue_mutex);
	}
	return (0);
}

static void	*scheduler_routine(void *arg)
{
	t_simulation_data	*simulation;
	t_compile_request	*pending;
	int					pending_count;
	int					dispatched;

	simulation = (t_simulation_data *)arg;
	pending = malloc(sizeof(t_compile_request)
			* simulation->config->number_of_coders);
	if (!pending)
		return (NULL);
	while (!is_simulation_stopped(simulation))
	{
		if (wait_for_queue(simulation))
			break ;
		pending_count = drain_queue(simulation, pending);
		pthread_mutex_unlock(&simulation->queue_mutex);
		dispatched = dispatch_pending(simulation, pending, pending_count);
		if (dispatched < pending_count)
			usleep(1000);
	}
	free(pending);
	return (NULL);
}


static void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	t_dongle_data	*dongles;
	int				first;
	int				second;
	int				temp;

	context = (t_coder_context *)arg;
	coder = context->coder;
	dongles = context->simulation->dongles;
	first = coder->id;

	if (context->simulation->config->number_of_coders == 1)
		second = -1;
	else
	{
		second = (coder->id + 1)
			% context->simulation->config->number_of_coders;
		if (first > second)
		{
			temp = first;						
			first = second;
			second = temp;
		}
	}

	while (!is_simulation_stopped(context->simulation)
		&& get_compile_count(context->simulation, coder)
		< context->simulation->config->number_of_compiles_required)
	{
		if (enqueue_compile_request(coder, context->simulation) != 0)
			return (NULL);

		pthread_mutex_lock(&context->simulation->state_mutex);
		while (!coder->has_permission
			&& !context->simulation->stop_simulation)
			pthread_cond_wait(&coder->cond,
				&context->simulation->state_mutex);
		if (context->simulation->stop_simulation)
		{
			pthread_mutex_unlock(&context->simulation->state_mutex);
			return (NULL);
		}
		coder->has_permission = 0;
		pthread_mutex_unlock(&context->simulation->state_mutex);

		pthread_mutex_lock(&dongles[first].mutex);
		log_event(context->simulation, coder->id, "has taken a dongle");

		if (second != -1)
		{
			pthread_mutex_lock(&dongles[second].mutex);
			log_event(context->simulation, coder->id, "has taken a dongle");
		}
		else
		{
			/* Cas 1 codeur : un seul dongle sur la table, mais compiler
			   en requiert deux. On ne peut jamais compiler : on attend
			   ici que le moniteur detecte le burnout et stoppe la simu. */
			while (!is_simulation_stopped(context->simulation))
				usleep(1000);
			unlock_dongle(&dongles[first],
				context->simulation->config->dongle_cooldown);
			return (NULL);
		}
		if (is_simulation_stopped(context->simulation))
		{
			if (second != -1)
				unlock_dongle(&dongles[second],
					context->simulation->config->dongle_cooldown);
			unlock_dongle(&dongles[first],
				context->simulation->config->dongle_cooldown);
			return (NULL);
		}

		pthread_mutex_lock(&context->simulation->state_mutex);
		coder->last_compile_start = get_time_ms();
		pthread_mutex_unlock(&context->simulation->state_mutex);

		log_event(context->simulation, coder->id, "is compiling");
		usleep(context->simulation->config->time_to_compile * 1000);


		if (second != -1)
			unlock_dongle(&dongles[second],
				context->simulation->config->dongle_cooldown);
		unlock_dongle(&dongles[first],
			context->simulation->config->dongle_cooldown);

		if (is_simulation_stopped(context->simulation))
			return (NULL);
		log_event(context->simulation, coder->id, "is debugging");
		usleep(context->simulation->config->time_to_debug * 1000);

		if (is_simulation_stopped(context->simulation))
			return (NULL);
		log_event(context->simulation, coder->id, "is refactoring");
		usleep(context->simulation->config->time_to_refactor * 1000);

		if (is_simulation_stopped(context->simulation))
			return (NULL);
		pthread_mutex_lock(&context->simulation->state_mutex);
		coder->compile_count++;
		pthread_mutex_unlock(&context->simulation->state_mutex);


		// if (second != -1)
		// 	pthread_mutex_unlock(&dongles[second].mutex);
		// pthread_mutex_unlock(&dongles[first].mutex);

		pthread_mutex_lock(&context->simulation->state_mutex);
		if (coder->compile_count
			>= context->simulation->config->number_of_compiles_required)
		{
			pthread_mutex_unlock(&context->simulation->state_mutex);
			mark_coder_finished(context->simulation, coder);
		}
		else
			pthread_mutex_unlock(&context->simulation->state_mutex);	
	}
	return (NULL);
}

static void	*monitor_routine(void *arg)
{
	t_simulation_data	*simulation;
	int					i;
	int					j;
	long				current_time;
	int					all_finished;

	simulation = (t_simulation_data *)arg;
	while (1)
	{
		current_time = get_time_ms();
		pthread_mutex_lock(&simulation->state_mutex);
		if (simulation->stop_simulation)
		{
			pthread_mutex_unlock(&simulation->state_mutex);
			return (NULL);
		}
		all_finished = simulation->finished_coders
			== simulation->config->number_of_coders;
		if (all_finished)
		{
			simulation->stop_simulation = 1;
			i = 0;
			while (i < simulation->config->number_of_coders)
			{
				pthread_cond_signal(&simulation->coders[i].cond);
				i++;
			}
			pthread_mutex_unlock(&simulation->state_mutex);
			pthread_mutex_lock(&simulation->queue_mutex);
			pthread_cond_broadcast(&simulation->queue_cond);
			pthread_mutex_unlock(&simulation->queue_mutex);
			return (NULL);
		}
		i = 0;
		while (i < simulation->config->number_of_coders)
		{
			if (!simulation->coders[i].is_finished
				&& current_time - simulation->coders[i].last_compile_start
				>= simulation->config->time_to_burnout)
			{
				simulation->stop_simulation = 1;
				j = 0;
				while (j < simulation->config->number_of_coders)
				{
					pthread_cond_signal(&simulation->coders[j].cond);
					j++;
				}
				pthread_mutex_unlock(&simulation->state_mutex);
				pthread_mutex_lock(&simulation->queue_mutex);
				pthread_cond_broadcast(&simulation->queue_cond);
				pthread_mutex_unlock(&simulation->queue_mutex);
				log_event(simulation, simulation->coders[i].id, "burned out");
				return (NULL);
			}
			i++;
		}
		pthread_mutex_unlock(&simulation->state_mutex);
		usleep(1000);
	}
}

int	start_simulation(t_simulation_data *simulation)
{
	t_coder_context	*contexts;
	int				i;
	int created_threads;

	created_threads = 0;


	contexts = malloc(sizeof(t_coder_context)
			* simulation->config->number_of_coders);
	if (!contexts)
		return (1);
	simulation->start_time = get_time_ms();
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		simulation->coders[i].last_compile_start = simulation->start_time;
		i++;
	}
	if (pthread_create(&simulation->monitor_thread, NULL,
			monitor_routine, simulation) != 0)
	{
		free(contexts);
		return (1);
	}
	if (pthread_create(&simulation->scheduler_thread, NULL,
		scheduler_routine, simulation) != 0)
	{
		pthread_mutex_lock(&simulation->state_mutex);
		simulation->stop_simulation = 1;
		pthread_mutex_unlock(&simulation->state_mutex);
		pthread_mutex_lock(&simulation->queue_mutex);
		pthread_cond_broadcast(&simulation->queue_cond);
		pthread_mutex_unlock(&simulation->queue_mutex);
		pthread_join(simulation->monitor_thread, NULL);
		free(contexts);
		return (1);
	}	

	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		contexts[i].coder = &simulation->coders[i];
		contexts[i].simulation = simulation;
		if (pthread_create(&simulation->threads[i], NULL,
				coder_routine, &contexts[i]) != 0)
		{
			pthread_mutex_lock(&simulation->state_mutex);
			simulation->stop_simulation = 1;
			i = 0;
			while (i < simulation->config->number_of_coders)
			{
				pthread_cond_signal(&simulation->coders[i].cond);
				i++;
			}
			pthread_mutex_unlock(&simulation->state_mutex);
			pthread_mutex_lock(&simulation->queue_mutex);
			pthread_cond_broadcast(&simulation->queue_cond);
			pthread_mutex_unlock(&simulation->queue_mutex);
			while (created_threads > 0)
			{
				created_threads--;
				pthread_join(simulation->threads[created_threads], NULL);
			}
			pthread_join(simulation->scheduler_thread, NULL);
			pthread_join(simulation->monitor_thread, NULL);
			free(contexts);
			return (1);
		}
		created_threads++;
		i++;
	}
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_join(simulation->threads[i], NULL);
		i++;
	}
	pthread_join(simulation->scheduler_thread, NULL);
	pthread_join(simulation->monitor_thread, NULL);
	free(contexts);
	return (0);
}