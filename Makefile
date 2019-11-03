main: clean
	clang++ \
		-std=c++14 \
		-Weverything \
		-Wno-old-style-cast \
		-Wno-padded \
		-Wno-missing-prototypes \
		-Wno-missing-variable-declarations \
		-Wno-unused-macros -Wno-zero-length-array \
		-Wno-global-constructors \
		-Wno-shadow-field-in-constructor \
		-Wno-c++98-compat-pedantic \
		-Wno-used-but-marked-unused \
		-Wunused-parameter \
		-idirafter "." \
		-g -O0 \
		-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls \
		-o main \
		transformer.cc transformer_tests.cc fidl.cc

clean:
	rm -f *.o
