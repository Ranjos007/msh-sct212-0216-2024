CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

msh: msh.c
.git$(CC) $(CFLAGS) -o msh msh.c

clean:
.gitrm -f msh *.o
