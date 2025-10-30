CC = clang
CFLAGS = -Wall -Wvla -Werror -gdwarf-4

########################################################################

.PHONY: asan msan nosan

asan: CFLAGS += -fsanitize=address,leak,undefined
asan: all

msan: CFLAGS += -fsanitize=memory,undefined -fsanitize-memory-track-origins
msan: all

nosan: all

########################################################################

.PHONY: all
all: encode decode

encode: src/encode.c src/huffman.c src/Counter.c src/File.c
	$(CC) $(CFLAGS) -o encode src/encode.c src/huffman.c src/Counter.c src/File.c

decode: src/decode.c src/huffman.c src/Counter.c src/File.c
	$(CC) $(CFLAGS) -o decode src/decode.c src/huffman.c src/Counter.c src/File.c 

.PHONY: clean
clean:
	rm -f encode decode
