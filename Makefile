main: clean
	clang++ -std=c++14 -Weverything -Wno-old-style-cast -Wno-padded -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-unused-macros -Wno-zero-length-array -Wno-global-constructors -Wno-shadow-field-in-constructor -Wno-c++98-compat-pedantic -g -O0 -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fno-optimize-sibling-calls -o main transformer.cc runner.cc fidl.cc -std=c++17

clean:
	rm -f *.o
