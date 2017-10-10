NAME=app
CFLAGS=-Wall -Wextra -g -O0 -std=c++11
SRCS=$(shell find . -name "*.cpp")
HDRS=$(shell find . -name "*.h")
OBJS=$(SRCS:.cpp=.o)
LIBS= -lwiringPi -lm -lpthread
CC=g++
LD=gcc
 
all: $(SRCS) $(NAME)
 
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
 
clean:
	@rm -f $(OBJS)
	@rm -f *.d *~

clean_all: clean
	@rm -f $(NAME)
