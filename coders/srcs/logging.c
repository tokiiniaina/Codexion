#include "codexion.h"

static int	get_coder_color(t_simulation_data *simulation, int coder_id)
{
	static int	colors[] = {
		15,
		39
	};
	int			nb_colors;

	nb_colors = sizeof(colors) / sizeof(colors[0]);
	return (colors[simulation->coders[coder_id].color_index % nb_colors]);
}

void	log_event(t_simulation_data *simulation, int coder_id, char *message)
{
	long	timestamp;
	int		color;

	timestamp = get_time_ms() - simulation->start_time;
	if (strcmp(message, "burned out") == 0)
		color = 196;
	else
		color = get_coder_color(simulation, coder_id);
	pthread_mutex_lock(&simulation->log_mutex);
	printf("\033[38;5;%dm%ld %d %s\033[0m\n",
		color, timestamp, coder_id + 1, message);
	pthread_mutex_unlock(&simulation->log_mutex);
}
