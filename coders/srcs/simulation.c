#include "codexion.h"

static void	log_state(t_simulation_data *sim, int coder_id, char *msg)
{
	long	elapsed;

	elapsed = get_time_ms() - sim->start_time;
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", elapsed, coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

static int	is_simulation_stopped(t_simulation_data *simulation)
{
	int	stopped;

	pthread_mutex_lock(&simulation->state_mutex);
	stopped = simulation->stop_simulation;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

static int	all_coders_finished(t_simulation_data *sim)
{
	int	i;
	int	required;

	required = sim->config->number_of_compiles_required;
	i = 0;
	while (i < sim->config->number_of_coders)
	{
		if (sim->coders[i].compile_count < required)
			return (0);
		i++;
	}
	return (1);
}

static void	cleanup_dongles(t_dongle_data *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&dongles[i].mutex);
		pthread_cond_destroy(&dongles[i].cond);
		if (dongles[i].queue.requests)
			free(dongles[i].queue.requests);
		i++;
	}
}

void	free_simulation(t_simulation_data *simulation, int mutex_count)
{
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	if (simulation->dongles)
	{
		cleanup_dongles(simulation->dongles, mutex_count);
		free(simulation->dongles);
	}
	if (simulation->coders)
		free(simulation->coders);
	if (simulation->threads)
		free(simulation->threads);
}

int	init_simulation(t_simulation_data *simulation,
		t_simulation_config *config)
{
	int	i;

	simulation->stop_simulation = 0;
	simulation->request_counter = 0;
	simulation->config = config;
	simulation->coders = NULL;
	simulation->dongles = NULL;
	simulation->threads = NULL;
	simulation->finished_coders = 0;
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	simulation->coders = malloc(sizeof(t_coder_data)
			* config->number_of_coders);
	simulation->threads = malloc(sizeof(pthread_t)
			* config->number_of_coders);
	simulation->dongles = malloc(sizeof(t_dongle_data)
			* config->number_of_coders);
	if (!simulation->coders || !simulation->threads || !simulation->dongles)
	{
		free_simulation(simulation, 0);
		return (1);
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->coders[i].id = i + 1;
		simulation->coders[i].compile_count = 0;
		simulation->coders[i].last_compile_start = 0;
		simulation->coders[i].is_finished = 0;
		i++;
	}
	i = 0;
	while (i < config->number_of_coders)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].is_available = 1;
		simulation->dongles[i].available_at = 0;
		if (pthread_mutex_init(&simulation->dongles[i].mutex, NULL) != 0
			|| pthread_cond_init(&simulation->dongles[i].cond, NULL) != 0
			|| init_priority_queue(&simulation->dongles[i].queue, 2))
		{
			cleanup_dongles(simulation->dongles, i);
			free_simulation(simulation, 0);
			return (1);
		}
		i++;
	}
	return (0);
}

/*
** Verrouille le dongle, s'enregistre dans sa file d'attente,
** et attend d'être 1) en tête de file selon le scheduler
** et 2) disponible (ni tenu, ni en cooldown).
** On ne tient jamais deux mutex de dongle en même temps,
** ce qui élimine tout risque de deadlock circulaire.
*/
static void	acquire_dongle(t_dongle_data *dongle,
		t_compile_request request, t_scheduler scheduler)
{
	pthread_mutex_lock(&dongle->mutex);
	push_request(&dongle->queue, request, scheduler);
	while (1)
	{
		if (dongle->is_available
			&& get_time_ms() >= dongle->available_at
			&& dongle->queue.size > 0
			&& dongle->queue.requests[0].coder_id == request.coder_id)
			break ;
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	pop_request(&dongle->queue, &request, scheduler);
	dongle->is_available = 0;
	pthread_mutex_unlock(&dongle->mutex);
}

static void	release_dongle(t_dongle_data *dongle, int cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_available = 1;
	dongle->available_at = get_time_ms() + cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

static t_compile_request	build_request(t_simulation_data *sim,
		t_coder_data *coder)
{
	t_compile_request	request;

	pthread_mutex_lock(&sim->state_mutex);
	request.coder_id = coder->id;
	request.arrival_order = sim->request_counter++;
	request.deadline = coder->last_compile_start
		+ sim->config->time_to_burnout;
	pthread_mutex_unlock(&sim->state_mutex);
	return (request);
}

static void	do_compile_cycle(t_coder_context *context,
		int first, int second)
{
	t_simulation_data	*sim;
	t_coder_data		*coder;
	t_compile_request	request;

	sim = context->simulation;
	coder = context->coder;
	request = build_request(sim, coder);
	acquire_dongle(&sim->dongles[first], request, sim->config->scheduler);
	log_state(sim, coder->id, "has taken a dongle");
	acquire_dongle(&sim->dongles[second], request, sim->config->scheduler);
	log_state(sim, coder->id, "has taken a dongle");
	pthread_mutex_lock(&sim->state_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&sim->state_mutex);
	log_state(sim, coder->id, "is compiling");
	usleep(sim->config->time_to_compile * 1000);
	pthread_mutex_lock(&sim->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->state_mutex);
	release_dongle(&sim->dongles[first], sim->config->dongle_cooldown);
	release_dongle(&sim->dongles[second], sim->config->dongle_cooldown);
	log_state(sim, coder->id, "is debugging");
	usleep(sim->config->time_to_debug * 1000);
	log_state(sim, coder->id, "is refactoring");
	usleep(sim->config->time_to_refactor * 1000);
}

static void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	int				first;
	int				second;
	int				temp;

	context = (t_coder_context *)arg;
	coder = context->coder;
	first = coder->id - 1;
	second = (coder->id) % context->simulation->config->number_of_coders;
	if (first > second)
	{
		temp = first;
		first = second;
		second = temp;
	}
	if (first == second)
	{
		/* 1 seul coder : impossible de compiler (il faut 2 dongles),
		** on laisse le monitor détecter le burnout inévitable. */
		while (!is_simulation_stopped(context->simulation))
			usleep(500);
		return (NULL);
	}
	while (!is_simulation_stopped(context->simulation)
		&& coder->compile_count
		< context->simulation->config->number_of_compiles_required)
		do_compile_cycle(context, first, second);
	pthread_mutex_lock(&context->simulation->state_mutex);
	coder->is_finished = 1;
	pthread_mutex_unlock(&context->simulation->state_mutex);
	return (NULL);
}

static void	*monitor_routine(void *arg)
{
	t_simulation_data	*sim;
	int					i;
	long				elapsed;

	sim = (t_simulation_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&sim->state_mutex);
		if (all_coders_finished(sim))
		{
			sim->stop_simulation = 1;
			pthread_mutex_unlock(&sim->state_mutex);
			return (NULL);
		}
		i = 0;
		while (i < sim->config->number_of_coders)
		{
			elapsed = get_time_ms() - sim->coders[i].last_compile_start;
			if (elapsed >= sim->config->time_to_burnout)
			{
				sim->stop_simulation = 1;
				pthread_mutex_unlock(&sim->state_mutex);
				log_state(sim, sim->coders[i].id, "burned out");
				return (NULL);
			}
			i++;
		}
		pthread_mutex_unlock(&sim->state_mutex);
		usleep(500);
	}
}

int	start_simulation(t_simulation_data *simulation)
{
	t_coder_context	*contexts;
	int				i;

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
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		contexts[i].coder = &simulation->coders[i];
		contexts[i].simulation = simulation;
		if (pthread_create(&simulation->threads[i], NULL,
				coder_routine, &contexts[i]) != 0)
		{
			free(contexts);
			return (1);
		}
		i++;
	}
	i = 0;
	while (i < simulation->config->number_of_coders)
	{
		pthread_join(simulation->threads[i], NULL);
		i++;
	}
	pthread_join(simulation->monitor_thread, NULL);
	free(contexts);
	return (0);
}