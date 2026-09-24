NAME = codexion
TSAN_NAME = codexion_tsan

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread
TSAN_FLAGS = -fsanitize=thread -g

INCLUDES = -Iincludes

SRCS_DIR = srcs
OBJS_DIR = objs
TSAN_OBJS_DIR = objs_tsan
TESTS_DIR = tests

SRCS = main.c parsing.c states.c simulation.c \
    queue/priority_queue.c queue/priority_queue_heap.c \
    time.c init.c cleanup.c coder.c \
    coder_acquire.c coder_cycle.c dongle.c scheduler.c \
    scheduler_dispatch.c monitor.c logging.c

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
TSAN_OBJS = $(addprefix $(TSAN_OBJS_DIR)/, $(SRCS:.c=.o))

TEST_QUEUE = test_queue
TEST_TIME = test_time


# ============================================================
# BUILD NORMAL
# ============================================================

all: banner $(NAME)

banner:
	@printf "\033[1;32m"
	@printf " ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf "██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf "██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf "██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf "╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf " ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "\033[0m"

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)


# ============================================================
# THREADSANITIZER
# ============================================================

tsan: $(TSAN_NAME)

$(TSAN_NAME): $(TSAN_OBJS)
	@$(CC) $(CFLAGS) $(TSAN_FLAGS) $(TSAN_OBJS) -o $(TSAN_NAME)

$(TSAN_OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(TSAN_FLAGS) $(INCLUDES) -c $< -o $@


# ============================================================
# TESTS
# ============================================================

tests_run: $(TEST_QUEUE) $(TEST_TIME)

$(TEST_QUEUE): $(OBJS_DIR)/queue/priority_queue.o \
	$(OBJS_DIR)/queue/priority_queue_heap.o \
	$(OBJS_DIR)/test_queue.o
	@$(CC) $(CFLAGS) \
		$(OBJS_DIR)/queue/priority_queue.o \
		$(OBJS_DIR)/queue/priority_queue_heap.o \
		$(OBJS_DIR)/test_queue.o -o $(TEST_QUEUE)

$(TEST_TIME): $(OBJS_DIR)/time.o $(OBJS_DIR)/test_time.o
	@$(CC) $(CFLAGS) \
		$(OBJS_DIR)/time.o \
		$(OBJS_DIR)/test_time.o -o $(TEST_TIME)

$(OBJS_DIR)/test_queue.o: $(TESTS_DIR)/test_queue.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/test_time.o: $(TESTS_DIR)/test_time.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


# ============================================================
# CLEAN
# ============================================================

clean:
	@rm -rf $(OBJS_DIR)

fclean: clean
	@rm -f $(NAME) $(TEST_QUEUE) $(TEST_TIME) $(TSAN_NAME)
	@rm -rf $(TSAN_OBJS_DIR)

re: fclean all


# ============================================================
# PHONY
# ============================================================

.PHONY: all clean fclean re tests_run tsan banner