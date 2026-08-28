#include "codexion.h"

int	main(void)
{
	t_priority_queue	queue;
	t_compile_request	request;

	init_priority_queue(&queue, 5);

	request.coder_id = 0;
	request.deadline = 800;
	request.arrival_order = 2;
	push_request(&queue, request, SCHEDULER_FIFO);

	request.coder_id = 1;
	request.deadline = 600;
	request.arrival_order = 0;
	push_request(&queue, request, SCHEDULER_FIFO);

	request.coder_id = 2;
	request.deadline = 700;
	request.arrival_order = 1;
	push_request(&queue, request, SCHEDULER_FIFO);

	printf("size = %d\n", queue.size);
	printf("root coder = %d\n", queue.requests[0].coder_id);
	printf("root arrival = %d\n", queue.requests[0].arrival_order);

	free(queue.requests);
	return (0);
}
