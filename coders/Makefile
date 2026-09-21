NAME = codexion

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread

INCLUDES = -Iincludes

SRCS_DIR = srcs

OBJS_DIR = objs

TESTS_DIR = tests

SRCS = main.c parsing.c states.c simulation.c queue/priority_queue.c \
	queue/priority_queue_heap.c time.c init.c cleanup.c coder.c \
	coder_actions.c dongle.c scheduler.c scheduler_dispatch.c \
	monitor.c logging.c

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))

LIB_OBJS = $(addprefix $(OBJS_DIR)/, $(LIB_SRCS:.c=.o))

TEST_QUEUE = test_queue

TEST_TIME = test_time

all: banner $(NAME)

banner:
	@printf "\033[1;32m"
	@printf " ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf "██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf "██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf "██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf "╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf " ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "\033[0m"

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

tests_run: $(TEST_QUEUE) $(TEST_TIME)

$(TEST_QUEUE): $(LIB_OBJS) $(OBJS_DIR)/test_queue.o
	@$(CC) $(CFLAGS) $(LIB_OBJS) $(OBJS_DIR)/test_queue.o -o $(TEST_QUEUE)

$(TEST_TIME): $(OBJS_DIR)/time.o $(OBJS_DIR)/test_time.o
	@$(CC) $(CFLAGS) $(OBJS_DIR)/time.o $(OBJS_DIR)/test_time.o -o $(TEST_TIME)

$(OBJS_DIR)/test_queue.o: $(TESTS_DIR)/test_queue.c | $(OBJS_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/test_time.o: $(TESTS_DIR)/test_time.c | $(OBJS_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJS_DIR)

fclean: clean
	@rm -f $(NAME) $(TEST_QUEUE) $(TEST_TIME)

re: fclean all

.PHONY: all clean fclean re tests_run banner