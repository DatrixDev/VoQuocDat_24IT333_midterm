CC = cc
CFLAGS = -Wall -Wextra -std=c11 -D_NETBSD_SOURCE
TARGET = myls
OBJS = main.o ls.o fileinfo.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c ls.h
	$(CC) $(CFLAGS) -c main.c

ls.o: ls.c ls.h fileinfo.h
	$(CC) $(CFLAGS) -c ls.c

fileinfo.o: fileinfo.c fileinfo.h ls.h
	$(CC) $(CFLAGS) -c fileinfo.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean