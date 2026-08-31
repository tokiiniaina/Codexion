#include "codexion.h"
#include <unistd.h>

typedef struct s_coder_test
{
	int				id;
	t_dongle_data	*first_dongle;
	t_dongle_data	*second_dongle;
}	t_coder_test;

void	*thread_function(void *arg)
{
	t_coder_test	*coder;

	coder = (t_coder_test *)arg;
	printf("Coder %d wants dongle %d\n",
		coder->id, coder->first_dongle->id);
	pthread_mutex_lock(&coder->first_dongle->mutex);
	printf("Coder %d got dongle %d\n",
		coder->id, coder->first_dongle->id);

	printf("Coder %d wants dongle %d\n",
		coder->id, coder->second_dongle->id);
	pthread_mutex_lock(&coder->second_dongle->mutex);
	printf("Coder %d got dongle %d\n",
		coder->id, coder->second_dongle->id);

	printf("Coder %d is using both dongles\n", coder->id);
	usleep(2000000);

	pthread_mutex_unlock(&coder->second_dongle->mutex);
	printf("Coder %d released dongle %d\n",
		coder->id, coder->second_dongle->id);

	pthread_mutex_unlock(&coder->first_dongle->mutex);
	printf("Coder %d released dongle %d\n",
		coder->id, coder->first_dongle->id);

	return (NULL);
}

int	main(void)
{
	t_dongle_data	dongle_0;
	t_dongle_data	dongle_1;
	t_coder_test	coder_0;
	t_coder_test	coder_1;
	pthread_t		thread_0;
	pthread_t		thread_1;

	dongle_0.id = 0;
	dongle_0.is_available = 1;
	dongle_1.id = 1;
	dongle_1.is_available = 1;
	pthread_mutex_init(&dongle_0.mutex, NULL);
	pthread_mutex_init(&dongle_1.mutex, NULL);

	coder_0.id = 0;
	coder_0.first_dongle = &dongle_0;
	coder_0.second_dongle = &dongle_1;

	coder_1.id = 1;
	coder_1.first_dongle = &dongle_0;
	coder_1.second_dongle = &dongle_1;

	pthread_create(&thread_0, NULL, thread_function, &coder_0);
	pthread_create(&thread_1, NULL, thread_function, &coder_1);

	pthread_join(thread_0, NULL);
	pthread_join(thread_1, NULL);

	pthread_mutex_destroy(&dongle_0.mutex);
	pthread_mutex_destroy(&dongle_1.mutex);

	return (0);
}
