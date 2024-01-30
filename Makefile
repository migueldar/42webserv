NAME := webserv
SRCS := srcs/sockets/server.cpp
OBJS := $(SRCS:srcs/%.cpp=objs/%.o)
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -I inc -std=c++98
LDFLAGS = -std=c++98
RM = /bin/rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $(NAME)

objs:
	@mkdir	objs \
			objs/sockets \

objs/%.o: srcs/%.cpp | objs
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) objs
fclean: clean
	$(RM) $(NAME)
re: fclean all

#check sanitize#
sanitize:: CXXFLAGS += -fsanitize=address -g3
sanitize:: LDFLAGS += -fsanitize=address
sanitize:: re

client: test/cli/client.o
	$(CXX) $(LDFLAGS) test/cli/client.o -o client

.PHONY: all clean fclean re sanitize