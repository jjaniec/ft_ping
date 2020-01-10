# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jjaniec <jjaniec@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/10/26 17:45:35 by jjaniec           #+#    #+#              #
#    Updated: 2020/01/03 22:33:28 by jjaniec          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_ping

UNAME_S := $(shell uname -s)

SRC_NAME = 	calc_checksum.c \
			resolve_hostname.c \
			ft_ping.c \
			handle_sigalrm.c \
			handle_sigint.c \
			send_icmp_echo.c \
			update_rtt.c \
			format_reply_output.c \
			main.c

SRC_DIR = ./srcs/
INCLUDES_DIR = ./includes/
OBJ_DIR = ./objs/

SRC = $(addprefix $(SRC_DIR), $(SRC_NAME))
OBJ = $(addprefix $(OBJ_DIR), $(SRC_NAME:.c=.o))

CC = gcc
CFLAGS = -Wall -Wextra -Werror
# DEV_FLAGS = -g -fsanitize=address -fno-omit-frame-pointer
IFLAGS = -I./ft_printf/includes -I./$(INCLUDES_DIR)
LFLAGS = -L./ft_printf -lftprintf

CFLAGS += $(DEV_FLAGS)
FT_PRINTF_DIR = ./ft_printf
LIBFTPRINTF = $(addprefix $(FT_PRINTF_DIR),"/libftprintf.a")

MAKEFILE_STATUS = $(addprefix $(addprefix $(FT_PRINTF_DIR),"/libft"),"/.makefile_status.sh")

UNAME_S := $(shell uname -s)

define ui_line
	$(MAKEFILE_STATUS) $(1) $(2) || true
endef

all : $(NAME)

$(NAME) : $(LIBFTPRINTF) $(OBJ)
	@cp $(LIBFTPRINTF) ./libftprintf.a
ifeq ($(UNAME_S),Linux)
	@$(CC) $(CFLAGS) $(LFLAGS) $(OBJ) $(LIBFTPRINTF) -o $(NAME)
endif
ifeq ($(UNAME_S),Darwin)
	@$(CC) $(CFLAGS) $(LFLAGS) $(OBJ) -o $(NAME)
endif

$(OBJ_DIR)%.o : $(SRC_DIR)%.c $(INCLUDES_DIR)/ft_ping.h
	@mkdir -p $(OBJ_DIR)
	@gcc $(CFLAGS) -c $(IFLAGS) $< -o $@ && $(call ui_line, $@, $(NAME))

$(FT_PRINTF_DIR):
	@git clone https://github.com/jjaniec/ft_printf 2> /dev/null || true

$(LIBFTPRINTF): $(FT_PRINTF_DIR)
	make -C ft_printf

clean:
	@rm -rf $(OBJ_DIR)
	@make clean -C ft_printf/

fclean: clean
	@make fclean -C ft_printf/
	@rm -f $(NAME)

re: fclean all

.PHONY: clean fclean all re
