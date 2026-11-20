CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

msh: msh.c
	$(CC) $(CFLAGS) -o msh msh.c

clean:
	rm -f msh *.o
