main: clean
	g++ -g -o main transformer.cc runner.c fidl.cc -std=c++17

clean:
	rm -f *.o
