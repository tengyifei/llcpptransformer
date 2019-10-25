main: clean
	clang++ -g -O0 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -o main transformer.cc runner.cc fidl.cc -std=c++17

clean:
	rm -f *.o
