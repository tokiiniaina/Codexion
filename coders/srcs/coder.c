#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	int				first;
	int				second;
	int				temp;

	context = (t_coder_context *)arg;
	coder = context->coder;
	first = coder->id;
	if (context->simulation->config->number_of_coders == 1)
		second = -1;
	else
	{
		second = (coder->id + 1)
			% context->simulation->config->number_of_coders;
		if (first > second)
		{
			temp = first;
			first = second;
			second = temp;
		}
	}
	while (!is_simulation_stopped(context->simulation)
		&& get_compile_count(context->simulation, coder)
		< context->simulation->config->number_of_compiles_required)
	{
		if (enqueue_compile_request(coder, context->simulation) != 0)
			return (NULL);
		if (wait_for_permission(context))
			return (NULL);
		if (take_dongles(context, first, second))
			return (NULL);
		compile_coder(context);
		release_dongles(context, first, second);
		if (finish_compile_cycle(context))
			return (NULL);
	}
	return (NULL);
}
