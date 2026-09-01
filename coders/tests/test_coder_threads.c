#include "codexion.h"
#include <unistd.h>

void	*coder_routine(void *arg)
{
	t_coder_data	*coder;

	coder = (t_coder_data *)arg;
	printf("Coder %d is running\n", coder->id);
	usleep(500000);
	printf("Coder %d finished\n", coder->id);
	return (NULL);
}

int	main(void)
{
	t_coder_data	coders[3];
	pthread_t		threads[3];
	int				i;

	i = 0;
	while (i < 3)
	{
		coders[i].id = i;
		coders[i].compile_count = 0;
		coders[i].last_compile_start = 0;
		coders[i].is_finished = 0;
		i++;
	}

	i = 0;
	while (i < 3)
	{
		pthread_create(&threads[i], NULL, coder_routine, &coders[i]);
		i++;
	}

	i = 0;
	while (i < 3)
	{
		pthread_join(threads[i], NULL);
		i++;
	}

	return (0);
}
