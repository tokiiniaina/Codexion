#include "codexion.h"

static int	coder_cycle(t_coder_context *context, int first, int second)
{
	if (enqueue_compile_request(context->coder, context->simulation) != 0)
		return (1);
	if (wait_for_permission(context))
		return (1);
	if (take_dongles(context, first, second))
		return (1);
	compile_coder(context);
	release_dongles(context, first, second);
	if (finish_compile_cycle(context))
		return (1);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder_context	*context;
	t_coder_data	*coder;
	int				first;
	int				second;

	context = (t_coder_context *)arg;
	coder = context->coder;
	get_coder_dongles(context->simulation, coder->id, &first, &second);
	if (get_compile_count(context->simulation, coder)
		>= context->simulation->config->number_of_compiles_required)
	{
		mark_coder_finished(context->simulation, coder);
		return (NULL);
	}
	while (!is_simulation_stopped(context->simulation)
		&& get_compile_count(context->simulation, coder)
		< context->simulation->config->number_of_compiles_required)
	{
		if (coder_cycle(context, first, second))
			return (NULL);
	}
	return (NULL);
}
