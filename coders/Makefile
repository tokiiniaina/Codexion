NAME = codexion
TSAN_NAME = codexion_tsan
ASAN_NAME = codexion_asan

.DEFAULT_GOAL := all

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread
TSAN_FLAGS = -fsanitize=thread -g
ASAN_FLAGS = -fsanitize=address -g3

INCLUDES = -Iincludes

SRCS_DIR = srcs
OBJS_DIR = objs
TSAN_OBJS_DIR = objs_tsan
ASAN_OBJS_DIR = objs_asan

SRCS = main.c parsing.c states.c simulation.c \
	queue/priority_queue.c queue/priority_queue_heap.c \
	time.c init.c cleanup.c coder.c \
	coder_acquire.c coder_cycle.c dongle.c scheduler.c \
	scheduler_dispatch.c monitor.c logging.c termination.c

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
TSAN_OBJS = $(addprefix $(TSAN_OBJS_DIR)/, $(SRCS:.c=.o))
ASAN_OBJS = $(addprefix $(ASAN_OBJS_DIR)/, $(SRCS:.c=.o))

# ============================================================
# COLORS
# ============================================================

GREEN   = \033[1;32m
RED     = \033[1;31m
YELLOW  = \033[1;33m
CYAN    = \033[1;36m
MAGENTA = \033[1;35m
ORANGE  = \033[38;5;208m
RESET   = \033[0m

# ============================================================
# BANNERS
# Chaque contexte (build simple / rebuild / tsan / retsan) a son
# propre logo pour qu'on sache d'un coup d'oeil quel mode tourne.
# ============================================================

banner:
	@printf "$(GREEN)"
	@printf " ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf "██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf "██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf "██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf "╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf " ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "                    [ NORMAL BUILD ]\n"
	@printf "$(RESET)"

banner_re:
	@printf "$(YELLOW)"
	@printf " ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf "██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf "██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf "██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf "╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf " ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "              [ FULL REBUILD: fclean + all ]\n"
	@printf "$(RESET)"

banner_tsan:
	@printf "$(MAGENTA)"
	@printf "  ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf " ██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf " ██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf " ██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf " ╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf "  ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "                [ THREADSANITIZER BUILD ]\n"
	@printf "$(RESET)"

banner_retsan:
	@printf "$(ORANGE)"
	@printf "  ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf " ██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf " ██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf " ██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf " ╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf "  ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "         [ THREADSANITIZER REBUILD : fclean + tsan ]\n"
	@printf "$(RESET)"

banner_asan:
	@printf "$(CYAN)"
	@printf "  ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf " ██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf " ██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf " ██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf " ╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf "  ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n"
	@printf "                [ ADDRESSANITIZER BUILD ]\n"
	@printf "$(RESET)"

# ============================================================
# BUILD NORMAL
# ============================================================

all: banner $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) && \
		printf "$(GREEN)[ok] normal executable ready: $(NAME)$(RESET)\n" || \
		(printf "$(RED)[error] normal link failed: target $(NAME)$(RESET)\n" >&2; exit 1)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ && \
		printf "$(CYAN)[ok] normal object created: $@$(RESET)\n" || \
		(printf "$(RED)[error] normal compilation failed: $< -> $@$(RESET)\n" >&2; exit 1)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)


# ============================================================
# THREADSANITIZER
# ============================================================

tsan: banner_tsan $(TSAN_NAME)

$(TSAN_NAME): $(TSAN_OBJS)
	@$(CC) $(CFLAGS) $(TSAN_FLAGS) $(TSAN_OBJS) -o $(TSAN_NAME) && \
		printf "$(MAGENTA)[ok] ThreadSanitizer executable ready: $(TSAN_NAME)$(RESET)\n" || \
		(printf "$(RED)[error] ThreadSanitizer link failed: target $(TSAN_NAME)$(RESET)\n" >&2; exit 1)

$(TSAN_OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(TSAN_FLAGS) $(INCLUDES) -c $< -o $@ && \
		printf "$(CYAN)[ok] TSAN object created: $@$(RESET)\n" || \
		(printf "$(RED)[error] ThreadSanitizer compilation failed: $< -> $@$(RESET)\n" >&2; exit 1)


# ============================================================
# ADDRESSSANITIZER
# ============================================================

asan: banner_asan $(ASAN_NAME)

$(ASAN_NAME): $(ASAN_OBJS)
	@$(CC) $(CFLAGS) $(ASAN_FLAGS) $(ASAN_OBJS) -o $(ASAN_NAME) && \
		printf "$(CYAN)[ok] AddressSanitizer executable ready: $(ASAN_NAME)$(RESET)\n" || \
		(printf "$(RED)[error] AddressSanitizer link failed: target $(ASAN_NAME)$(RESET)\n" >&2; exit 1)

$(ASAN_OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(ASAN_FLAGS) $(INCLUDES) -c $< -o $@ && \
		printf "$(CYAN)[ok] ASAN object created: $@$(RESET)\n" || \
		(printf "$(RED)[error] AddressSanitizer compilation failed: $< -> $@$(RESET)\n" >&2; exit 1)


# ============================================================
# CLEAN
# ============================================================

clean:
	@rm -rf $(OBJS_DIR) && \
		printf "$(YELLOW)[ok] clean completed: removed $(OBJS_DIR)$(RESET)\n" || \
		(printf "$(RED)[error] clean failed: could not remove $(OBJS_DIR)$(RESET)\n" >&2; exit 1)

fclean: clean
	@rm -f $(NAME) $(TSAN_NAME) $(ASAN_NAME) && \
	rm -rf $(TSAN_OBJS_DIR) $(ASAN_OBJS_DIR) && \
		printf "$(YELLOW)[ok] fclean completed: binaries and sanitizer objects removed$(RESET)\n" || \
		(printf "$(RED)[error] fclean failed: could not remove binaries or sanitizer objects$(RESET)\n" >&2; exit 1)

re: banner_re
	@$(MAKE) --no-print-directory fclean && \
	$(MAKE) --no-print-directory $(NAME) && \
		printf "$(GREEN)[ok] re completed: full normal rebuild$(RESET)\n" || \
		(printf "$(RED)[error] re failed: fclean or normal build failed$(RESET)\n" >&2; exit 1)

retsan: banner_retsan
	@$(MAKE) --no-print-directory fclean && \
	$(MAKE) --no-print-directory $(TSAN_NAME) && \
		printf "$(ORANGE)[ok] retsan completed: full ThreadSanitizer rebuild$(RESET)\n" || \
		(printf "$(RED)[error] retsan failed: fclean or TSAN build failed$(RESET)\n" >&2; exit 1)


# ============================================================
# PHONY
# ============================================================

.PHONY: all clean fclean re tsan retsan asan banner banner_re banner_tsan banner_retsan banner_asan