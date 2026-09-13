#include "codexion.h"

int	compare_requests(t_compile_request *first,
		t_compile_request *second, t_scheduler scheduler)
{
	if (scheduler == SCHEDULER_FIFO)
		return (first->arrival_order < second->arrival_order);
	if (scheduler == SCHEDULER_EDF)
	{
		if (first->deadline != second->deadline)
			return (first->deadline < second->deadline);
		return (first->arrival_order < second->arrival_order);
	}
	return (0);
}

void	heapify_up(t_priority_queue *queue, int index,
		t_scheduler scheduler)
{
	int					parent_index;
	t_compile_request	temp;

	while (index > 0)
	{
		parent_index = (index - 1) / 2;
		if (!compare_requests(&queue->requests[index],
				&queue->requests[parent_index], scheduler))
			break ;
		temp = queue->requests[index];
		queue->requests[index] = queue->requests[parent_index];
		queue->requests[parent_index] = temp;
		index = parent_index;
	}
}

void	heapify_down(t_priority_queue *queue, int index,
		t_scheduler scheduler)
{
	int					left_child;
	int					right_child;
	int					best_child;
	t_compile_request	temp;

	while (index * 2 + 1 < queue->size)
	{
		left_child = index * 2 + 1;
		right_child = index * 2 + 2;
		if (right_child >= queue->size)
			best_child = left_child;
		else if (compare_requests(&queue->requests[left_child],
				&queue->requests[right_child], scheduler))
			best_child = left_child;
		else
			best_child = right_child;
		if (!compare_requests(&queue->requests[best_child],
				&queue->requests[index], scheduler))
			break ;
		temp = queue->requests[index];
		queue->requests[index] = queue->requests[best_child];
		queue->requests[best_child] = temp;
		index = best_child;
	}
}
