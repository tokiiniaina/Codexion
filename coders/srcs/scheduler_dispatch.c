#include "codexion.h"

int	peek_request(t_priority_queue *queue, t_compile_request *request)
{
	if (queue->size == 0)
		return (1);
	*request = queue->requests[0];
	return (0);
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

int	drain_queue(t_simulation_data *simulation,
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

int	dispatch_pending(t_simulation_data *simulation,
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
