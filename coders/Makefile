NAME = codexion
TSAN_NAME = codexion_tsan
ASAN_NAME = codexion_asan

.DEFAULT_GOAL := all

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread
TSAN_FLAGS = -fsanitize=thread -g
ASAN_FLAGS = -fsanitize=address -g3

INCLUDES = -Iincludes
HEADER = includes/codexion.h

SRCS_DIR = srcs
OBJS_DIR = objs
TSAN_OBJS_DIR = objs_tsan
ASAN_OBJS_DIR = objs_asan

SRCS = main.c parsing.c states.c simulation.c \
	queue/priority_queue.c queue/priority_queue_heap.c \
	time.c init.c cleanup.c coder.c \
	init_sync.c \
	coder_acquire.c coder_cycle.c dongle.c scheduler.c \
	scheduler_dispatch.c monitor.c logging.c termination.c

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
TSAN_OBJS = $(addprefix $(TSAN_OBJS_DIR)/, $(SRCS:.c=.o))
ASAN_OBJS = $(addprefix $(ASAN_OBJS_DIR)/, $(SRCS:.c=.o))

NORMAL_BANNER = $(OBJS_DIR)/.banner
TSAN_BANNER = $(TSAN_OBJS_DIR)/.banner
ASAN_BANNER = $(ASAN_OBJS_DIR)/.banner

GREEN   = \033[1;32m
RED     = \033[1;31m
YELLOW  = \033[1;33m
CYAN    = \033[1;36m
MAGENTA = \033[1;35m
RESET   = \033[0m

define BANNER
	@printf "$(1)"
	@printf " ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗\n"
	@printf "██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║\n"
	@printf "██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║\n"
	@printf "██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║\n"
	@printf "╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║\n"
	@printf " ╚═════╝ ╚═════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝\n" # ██
	@printf "                    $(2)\n"
	@printf "$(RESET)"
endef

define COMPILE
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(1) $(INCLUDES) -c $< -o $@ && \
		printf "$(CYAN)[ok] $(2) object created: $@$(RESET)\n" || \
		(printf "$(RED)[error] $(2) compilation failed: $< -> $@$(RESET)\n" >&2; exit 1)
endef

define LINK
	@$(CC) $(CFLAGS) $(1) $^ -o $@ && \
		printf "$(2)[ok] $(3) executable ready: $@$(RESET)\n" || \
		(printf "$(RED)[error] $(3) link failed: target $@$(RESET)\n" >&2; exit 1)
endef

all: $(NAME)

$(NAME): $(OBJS)
	$(call LINK,,$(GREEN),normal)

$(NORMAL_BANNER): $(addprefix $(SRCS_DIR)/,$(SRCS)) $(HEADER)
	@mkdir -p $(@D)
	$(call BANNER,$(GREEN),[ NORMAL BUILD ])
	@touch $@

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(HEADER) | $(NORMAL_BANNER)
	$(call COMPILE,,normal)

tsan: $(TSAN_NAME)

$(TSAN_NAME): $(TSAN_OBJS)
	$(call LINK,$(TSAN_FLAGS),ThreadSanitizer)

$(TSAN_BANNER): $(addprefix $(SRCS_DIR)/,$(SRCS)) $(HEADER)
	@mkdir -p $(@D)
	$(call BANNER,$(MAGENTA),[ THREADSANITIZER BUILD ])
	@touch $@

$(TSAN_OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(HEADER) | $(TSAN_BANNER)
	$(call COMPILE,$(TSAN_FLAGS),TSAN)

asan: $(ASAN_NAME)

$(ASAN_NAME): $(ASAN_OBJS)
	$(call LINK,$(ASAN_FLAGS),AddressSanitizer)

$(ASAN_BANNER): $(addprefix $(SRCS_DIR)/,$(SRCS)) $(HEADER)
	@mkdir -p $(@D)
	$(call BANNER,$(CYAN),[ ADDRESSANITIZER BUILD ])
	@touch $@

$(ASAN_OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(HEADER) | $(ASAN_BANNER)
	$(call COMPILE,$(ASAN_FLAGS),ASAN)

clean:
	@rm -rf $(OBJS_DIR)
	@printf "$(YELLOW)[ok] clean completed: removed $(OBJS_DIR)$(RESET)\n"

fclean: clean
	@rm -f $(NAME) $(TSAN_NAME) $(ASAN_NAME)
	@rm -rf $(TSAN_OBJS_DIR) $(ASAN_OBJS_DIR)
	@printf "$(YELLOW)[ok] fclean completed: binaries and sanitizer objects removed$(RESET)\n"

re:
	@$(MAKE) --no-print-directory fclean >/dev/null
	@$(MAKE) --no-print-directory all

retsan:
	@$(MAKE) --no-print-directory fclean >/dev/null
	@$(MAKE) --no-print-directory tsan

.PHONY: all clean fclean re tsan retsan asan