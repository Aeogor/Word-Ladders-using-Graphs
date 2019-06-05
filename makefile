build:
	clear
	gcc -O3 -std=c99 -pedantic main.c avl.c graph.c mymem.c queue.c set.c stack.c timer.c

run:
	clear
	./a.out
