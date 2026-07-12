CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic -D_DEFAULT_SOURCE
LDLIBS  := -lncurses
TARGET  := ssh-launcher
SRCDIR  := src
OBJDIR  := build

SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
DEPS    := $(OBJS:.o=.d)

.PHONY: all clean install run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)
