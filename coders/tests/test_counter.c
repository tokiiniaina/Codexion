#include "codexion.h"


typedef struct s_shared_data
{
	int				counter;
	pthread_mutex_t	mutex;
} t_shared_data;


void	*increment_counter(void *arg)
{
	t_shared_data	*data;
	int	i;

	data = (t_shared_data *)arg;
	i = 0;
	while (i < 100000)
	{
		pthread_mutex_lock(&data->mutex);
		data->counter++;
		pthread_mutex_unlock(&data->mutex);
		i++;
	}
	return (0);
}


int	main(void)
{
	t_shared_data	data;
	pthread_t		thread_1;
	pthread_t		thread_2;

	data.counter = 0;
	pthread_mutex_init(&data.mutex, NULL);
	pthread_create(&thread_1, NULL, increment_counter, &data);
	pthread_create(&thread_2, NULL, increment_counter, &data);
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	printf("Counter: %d\n", data.counter);
	pthread_mutex_destroy(&data.mutex);
	return (0);
}
