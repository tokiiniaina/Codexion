#include "codexion.h"

void	*thread_function(void *arg)
{
	pthread_mutex_t	*mutex;

	mutex = (pthread_mutex_t *)arg;
	printf("Thread: before lock\n");
	pthread_mutex_lock(mutex);

	printf("Thread: inside critical section\n");
	usleep(5000000);
	pthread_mutex_unlock(mutex);
	printf("Thread: after unlock\n");
	return (NULL);
}

int	main(void)
{
	pthread_t		thread_1;
	pthread_t		thread_2;
	pthread_mutex_t	mutex;

	pthread_mutex_init(&mutex, NULL);

	pthread_create(&thread_1, NULL, thread_function, &mutex);
	pthread_create(&thread_2, NULL, thread_function, &mutex);
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	pthread_mutex_destroy(&mutex);

	return (0);
}
