#include "codexion.h"
#include <unistd.h>

typedef struct s_coder_context
{
	t_coder_data	*coder;
	t_dongle_data	*first_dongle;
	t_dongle_data	*second_dongle;
}	t_coder_context;

void	*coder_routine(void *arg)
{
	t_coder_context	*context;

	context = (t_coder_context *)arg;
	printf("Coder %d wants dongle %d\n",
		context->coder->id, context->first_dongle->id);
	pthread_mutex_lock(&context->first_dongle->mutex);
	printf("Coder %d got dongle %d\n",
		context->coder->id, context->first_dongle->id);

	printf("Coder %d wants dongle %d\n",
		context->coder->id, context->second_dongle->id);
	pthread_mutex_lock(&context->second_dongle->mutex);
	printf("Coder %d got dongle %d\n",
		context->coder->id, context->second_dongle->id);

	printf("Coder %d is using both dongles\n",
		context->coder->id);
	usleep(1000000);

	pthread_mutex_unlock(&context->second_dongle->mutex);
	printf("Coder %d released dongle %d\n",
		context->coder->id, context->second_dongle->id);

	pthread_mutex_unlock(&context->first_dongle->mutex);
	printf("Coder %d released dongle %d\n",
		context->coder->id, context->first_dongle->id);

	return (NULL);
}

int	main(void)
{
	t_coder_data		coders[3];
	t_dongle_data		dongles[3];
	t_coder_context	contexts[3];
	pthread_t			threads[3];
	int					i;

	i = 0;
	while (i < 3)
	{
		coders[i].id = i;
		coders[i].compile_count = 0;
		coders[i].last_compile_start = 0;
		coders[i].is_finished = 0;

		dongles[i].id = i;
		dongles[i].is_available = 1;
		pthread_mutex_init(&dongles[i].mutex, NULL);
		i++;
	}

	i = 0;
	while (i < 3)
	{
		contexts[i].coder = &coders[i];
		contexts[i].first_dongle = &dongles[i];
		contexts[i].second_dongle = &dongles[(i + 1) % 3];

		pthread_create(&threads[i], NULL, coder_routine, &contexts[i]);
		i++;
	}

	i = 0;
	while (i < 3)
	{
		pthread_join(threads[i], NULL);
		i++;
	}

	i = 0;
	while (i < 3)
	{
		pthread_mutex_destroy(&dongles[i].mutex);
		i++;
	}

	return (0);
}
