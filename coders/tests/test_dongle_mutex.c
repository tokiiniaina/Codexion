#include "codexion.h"
// #include <unistd.h>

void	*thread_function(void *arg)
{
	t_dongle_data	*dongle;

	dongle = (t_dongle_data *)arg;

	printf("Coder wants dongle %d\n", dongle->id);
	pthread_mutex_lock(&dongle->mutex);

	printf("Coder got dongle %d\n", dongle->id);
	usleep(5000000);

	printf("Coder releases dongle %d\n", dongle->id);
	pthread_mutex_unlock(&dongle->mutex);

	return (NULL);
}


int	main(void)
{
	t_dongle_data	dongle;
	pthread_t		thread_1;
	pthread_t		thread_2;

	dongle.id = 0;
	dongle.is_available = 1;
	pthread_mutex_init(&dongle.mutex, NULL);

	pthread_create(&thread_1, NULL, thread_function, &dongle);
	pthread_create(&thread_2, NULL, thread_function, &dongle);

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);

	pthread_mutex_destroy(&dongle.mutex);
	return (0);
}
