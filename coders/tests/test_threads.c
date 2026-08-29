#include "codexion.h"
#include <unistd.h>

void	*thread_function(void *arg)
{
	int	id;

	id = *(int *)arg;
	printf("Thread %d: start\n", id);
	usleep(1000000);
	printf("Thread %d: end\n", id);
	return (NULL);
}


int	main(void)
{
	int	thread_1_id;
	int	thread_2_id;
	int	thread_3_id;

	thread_1_id = 1;
	thread_2_id = 2;
	thread_3_id = 3;

	pthread_t	thread_1;
	pthread_t	thread_2;
	pthread_t	thread_3;

	pthread_create(&thread_1, NULL, thread_function, &thread_1_id);
	pthread_create(&thread_2, NULL, thread_function, &thread_2_id);
	pthread_create(&thread_3, NULL, thread_function, &thread_3_id);

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	pthread_join(thread_3, NULL);
	printf("Main finished\n");

	return (0);
}
