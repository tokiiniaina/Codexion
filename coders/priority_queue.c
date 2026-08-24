#include "codexion.h"

static void	heapify_up(t_priority_queue *queue, int index,
		t_scheduler scheduler);

int	compare_requests(t_compile_request *first,
		t_compile_request *second, t_scheduler scheduler)
{
	if (scheduler == SCHEDULER_FIFO)
		return (first->arrival_order < second->arrival_order);
	if (scheduler == SCHEDULER_EDF)
		return (first->deadline < second->deadline);
	return (0);
}

static void	heapify_up(t_priority_queue *queue, int index,
		t_scheduler scheduler)
{
	int				parent_index;
	t_compile_request	temp;

	while (index > 0)
	{
		parent_index = (index - 1) / 2;
		if (!compare_requests(&queue->requests[index],
				&queue->requests[parent_index], scheduler))
			break;
		temp = queue->requests[index];
		queue->requests[index] = queue->requests[parent_index];
		queue->requests[parent_index] = temp;
		index = parent_index;
	}
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

int	init_priority_queue(t_priority_queue *queue, int capacity)
{
	queue->requests = malloc(sizeof(t_compile_request) * capacity);
	if (!queue->requests)
		return (1);
	queue->size = 0;
	queue->capacity = capacity;
	return (0);
}

