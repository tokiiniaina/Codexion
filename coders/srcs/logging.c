#include "codexion.h"

void	log_event(t_simulation_data *simulation, int coder_id,
		char *message)
{
	long	timestamp;

	timestamp = get_time_ms() - simulation->start_time;
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ld %d %s\n", timestamp, coder_id + 1, message);
	pthread_mutex_unlock(&simulation->log_mutex);
}
