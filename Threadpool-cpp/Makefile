all:test

test:test.o ThreadPool.o TaskQueue.o
	g++ $^ -o $@ -lpthread

test.o:test.cpp
	g++ -c $^ -o $@

ThreadPool.o:ThreadPool.cpp
	g++ -c $< -o $@

TaskQueue.o:TaskQueue.cpp
	g++ -c TaskQueue.cpp -o $@

.PTHONY:clean
clean:
	rm -f *.o
	rm -f test
