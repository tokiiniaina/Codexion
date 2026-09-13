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

int	reserve_dongles(t_simulation_data *simulation,
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
