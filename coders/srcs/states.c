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
