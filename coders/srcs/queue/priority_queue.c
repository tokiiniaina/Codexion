#include "codexion.h"

int	init_priority_queue(t_priority_queue *queue, int capacity)
{
	queue->requests = malloc(sizeof(t_compile_request) * capacity);
	if (!queue->requests)
		return (1);
	queue->size = 0;
	queue->capacity = capacity;
	return (0);
}

int	push_request(t_priority_queue *queue,
		t_compile_request request, t_scheduler scheduler)
{
	if (queue->size >= queue->capacity)
		return (1);
	queue->requests[queue->size] = request;
	queue->size++;
	heapify_up(queue, queue->size - 1, scheduler);
	return (0);
}

int	pop_request(t_priority_queue *queue,
		t_compile_request *request, t_scheduler scheduler)
{
	if (queue->size == 0)
		return (1);
	*request = queue->requests[0];
	queue->requests[0] = queue->requests[queue->size - 1];
	queue->size--;
	heapify_down(queue, 0, scheduler);
	return (0);
}
