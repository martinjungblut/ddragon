all:
	gcc -O2 -o ddragon src/ddragon.c src/file.c

clean:
	rm ddragon

install:
	make all
	cp -f ddragon /usr/local/bin/
