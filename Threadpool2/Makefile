all:main

main:main.o threadpool.o
	gcc $^ -o $@ -lpthread

main.o:main.c
	gcc -g -c $^ -o $@

threadpool.o:threadpool.c
	gcc -g -c $< -o $@

.PTHONY:clean
clean:
	rm -f *.o
