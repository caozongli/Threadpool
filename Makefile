all:main

main:main.o threadpool.o
	gcc $^ -o $@ -lpthread

main.o:main.c
	gcc -c $^ -o $@

threadpool.o:threadpool.c
	gcc -c $< -o $@

.PTHONY:clean
clean:
	rm -f *.o
