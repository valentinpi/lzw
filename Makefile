lzw: lzw.c
	gcc -g -O0 -Wall -Wextra -o lzw lzw.c

clean:
	rm lzw

.PHONY: clean
