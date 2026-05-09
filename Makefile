CC = gcc
CFLAGS = -Wall -g

all: mylsof

mylsof: main.c opt_Ange.c opt_Cassandra.c opt_Naomy.c opt_Salif.c opt_Sobrin.c
	$(CC) $(CFLAGS) -o mylsof main.c opt_Ange.c opt_Cassandra.c opt_Naomy.c opt_Salif.c opt_Sobrin.c

clean:
	rm -f mylsof
