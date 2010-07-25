NAME := cdecl

SRCS := $(shell find -name "*.c")
OBJS := $(SRCS:%.c=%.o)

CFLAGS += -std=c99 -Wall -Werror -O2

DOXYGEN_CONFIG := Doxyfile
DOXYGEN_DIR    := doxygen

.PHONY : all
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(LD_FLAGS) $(CFLAGS) $(CPPFLAGS) $^  -o $@

doxygen: $(DOXYGEN_CONFIG) $(SRCS)
	@doxygen $(DOXYGEN_CONFIG)

.PHONY : doxygen-clean
doxygen-clean:
	@rm -rf $(DOXYGEN_DIR)

.PHONY : clean
clean: doxygen-clean
	@-rm -f *.o *.d *~ $(NAME)