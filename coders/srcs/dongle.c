#include "codexion.h"

void	unlock_dongle(t_dongle_data *dongle, int cooldown)
{
	dongle->is_available = 0;
	dongle->is_reserved = 0;
	dongle->available_at = get_time_ms() + cooldown;
	pthread_mutex_unlock(&dongle->mutex);
}

void	get_coder_dongles(t_simulation_data *simulation,
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

static int	try_reserve_dongle(t_simulation_data *sim, int idx,
		long current_time)
{
	pthread_mutex_lock(&sim->dongles[idx].mutex);
	if (sim->dongles[idx].is_reserved)
	{
		pthread_mutex_unlock(&sim->dongles[idx].mutex);
		return (0);
	}
	if (!sim->dongles[idx].is_available)
	{
		if (current_time < sim->dongles[idx].available_at)
		{
			pthread_mutex_unlock(&sim->dongles[idx].mutex);
			return (0);
		}
		sim->dongles[idx].is_available = 1;
	}
	sim->dongles[idx].is_reserved = 1;
	sim->dongles[idx].is_available = 0;
	pthread_mutex_unlock(&sim->dongles[idx].mutex);
	return (1);
}

int	reserve_dongles(t_simulation_data *simulation,
		int first, int second)
{
	long	current_time;

	current_time = get_time_ms();
	if (!try_reserve_dongle(simulation, first, current_time))
		return (0);
	if (second != -1)
	{
		if (!try_reserve_dongle(simulation, second, current_time))
		{
			pthread_mutex_lock(&simulation->dongles[first].mutex);
			simulation->dongles[first].is_reserved = 0;
			simulation->dongles[first].is_available = 1;
			pthread_mutex_unlock(&simulation->dongles[first].mutex);
			return (0);
		}
	}
	return (1);
}

void	release_dongles(t_coder_context *context, int first, int second)
{
	t_dongle_data	*dongles;

	dongles = context->simulation->dongles;
	if (second != -1)
		unlock_dongle(&dongles[second],
			context->simulation->config->dongle_cooldown);
	unlock_dongle(&dongles[first],
		context->simulation->config->dongle_cooldown);
}
