NAME = Webserv
CXX = c++
CXXFLAGS = #-Wall -Wextra -Werror #-std=c++98

# Use wildcard and recursion to find all .cpp files in the src directory and subdirectories
SRCS = $(wildcard src/**/*.cpp) src/main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "DONE ✅"
clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: clean all

.PHONY: all clean fclean re
