CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = build/hello11
OBJDIR = build
SRCDIR = .

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJDIR)/hello11.o
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJDIR)/hello11.o

$(OBJDIR)/hello11.o: $(SRCDIR)/hello11.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/hello11.c -o $(OBJDIR)/hello11.o

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

.PHONY: all clean