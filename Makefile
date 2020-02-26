lzw: main.c
	gcc -g -O0 -Wall -Wextra -o lzw main.c

clean:
	rm lzw

.PHONY: clean
