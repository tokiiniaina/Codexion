#include "codexion.h"

int	is_simulation_stopped(t_simulation_data *simulation)
{
	int	stopped;

	pthread_mutex_lock(&simulation->state_mutex);
	stopped = simulation->stop_simulation;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

void	mark_coder_finished(t_simulation_data *simulation,
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

int	get_compile_count(t_simulation_data *simulation,
		t_coder_data *coder)
{
	int	compile_count;

	pthread_mutex_lock(&simulation->state_mutex);
	compile_count = coder->compile_count;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (compile_count);
}

int	enqueue_compile_request(t_coder_data *coder,
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

int	start_simulation(t_simulation_data *simulation)
{
	t_coder_context	*contexts;
	int				i;
	int				created_threads;

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
