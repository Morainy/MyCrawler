all : Crawler.c RIO.c
	gcc -o Crawler Crawler.c RIO.c -lz
clean:
	rm -rf *.o Crawler
