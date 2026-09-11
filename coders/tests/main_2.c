#include <pthread.h>
#include <stdio.h>

int	main(void)
{
	pthread_mutex_t	mutex;
	pthread_mutexattr_t	attr;
	int				result;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(&mutex, &attr);
	pthread_mutexattr_destroy(&attr);

	printf("Lock\n");
	pthread_mutex_lock(&mutex);

	printf("First unlock\n");
	result = pthread_mutex_unlock(&mutex);
	printf("Result: %d\n", result);

	printf("Second unlock\n");
	result = pthread_mutex_unlock(&mutex);
	printf("Result: %d\n", result);

	pthread_mutex_destroy(&mutex);
	return (0);
}