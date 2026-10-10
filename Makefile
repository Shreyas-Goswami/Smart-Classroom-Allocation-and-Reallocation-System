CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2 -g
LDLIBS  := -lm
TARGET  := classroom_engine
SRCS    := main.c engine.c resource_db.c interval_tree.c max_heap.c ai_model.c mem_track.c
OBJS    := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)