TARGET = echo
OBJS = ae.o ae_epoll.o anet.o main.o
CFLAGS = -g
CC = gcc
$(TARGET) : $(OBJS) 
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

ae.o : ae.c
ae_epoll.o : ae_epoll.c
anet.o : anet.c
main.o : main.c

clean :
	rm $(OBJS) $(TARGET)
