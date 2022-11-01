# change application name here (executable output name)
TARGET=BKDict
 
# compiler
CC=gcc

main: src/main.c
	$(CC) -w -o $(TARGET) src/main.c src/lib/libbt.a -pthread `pkg-config gtk+-3.0 --cflags --libs` -export-dynamic
run: main
	./BKDict
clean:
	rm -f BKDict
