#include <pthread.h>
#include <stdio.h>

int	main(void)
{
	pthread_mutex_t	mutex;

	pthread_mutex_init(&mutex, NULL);

	printf("Lock\n");
	pthread_mutex_lock(&mutex);

	printf("First unlock\n");
	pthread_mutex_unlock(&mutex);

	printf("Second unlock\n");
	pthread_mutex_unlock(&mutex);

	printf("End\n");

	pthread_mutex_destroy(&mutex);
	return (0);
}