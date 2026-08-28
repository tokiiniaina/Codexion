#include "codexion.h"
#include <unistd.h>

int	main(void)
{
	long	first;
	long	second;

	first = get_time_ms();
	usleep(100000);
	second = get_time_ms();
	printf("First:  %ld ms\n", first);
	printf("Second: %ld ms\n", second);
	printf("Elapsed: %ld ms\n", second - first);
	return (0);
}
