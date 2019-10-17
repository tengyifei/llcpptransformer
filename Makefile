main: clean
	g++ -g -o main transformer.cc runner.c -std=c++17

clean:
	rm -f *.o
