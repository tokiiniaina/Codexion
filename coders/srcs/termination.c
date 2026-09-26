#include "codexion.h"

void	wake_everyone_locked(t_simulation_data *sim)
{
	int	i;

	sim->stop_simulation = 1;
	i = 0;
	while (i < sim->config->number_of_coders)
	{
		pthread_cond_signal(&sim->coders[i].cond);
		i++;
	}
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_lock(&sim->queue_mutex);
	pthread_cond_broadcast(&sim->queue_cond);
	pthread_mutex_unlock(&sim->queue_mutex);
}

void	wake_everyone(t_simulation_data *sim)
{
	pthread_mutex_lock(&sim->state_mutex);
	wake_everyone_locked(sim);
}
