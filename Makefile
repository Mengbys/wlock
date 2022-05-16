CC = gcc

LDFLAGS = -lpam -lpthread

OBJS = auth.o ui.o main.o

.PHONY: all
all: wlock

wlock: $(OBJS)
	$(CC) -o wlock $(OBJS) $(LDFLAGS)

auth.o: auth.c
	$(CC) -c auth.c -o auth.o

ui.o: ui.c
	$(CC) -c ui.c -o ui.o

main.o: main.c
	$(CC) -c main.c -o main.o

.PHONY: install
install: wlock
	install -m 4711 -o root -g root wlock /usr/sbin/wlock

.PHONY: uninstall
uninstall:
	rm /usr/sbin/wlock

.PHONY: clean
clean:
	rm wlock $(OBJS)

