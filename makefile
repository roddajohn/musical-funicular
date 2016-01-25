CC=gcc
CFLAGS=--std=c99 -g -o2 -Wall --pedantic `freetype-config --cflags` `sdl-config --cflags`
LDFLAGS=`icu-config --ldflags`
LIBS=-lcairo -lharfbuzz -lharfbuzz-icu `pkg-config --cflags --libs cairo ` `freetype-config --libs` `sdl-config --libs`

client: singleplayer-client.c
	$(CC) $< $(CFLAGS) -o $@ $(LDFLAGS) $(LIBS)
	#gcc -I /usr/include/SDL singleplayer-client.c -o singleplayer `pkg-config --cflags --libs gtk+-3.0 sdl2-config --cflags --libs`



server: singleplayer-server.c
	gcc singleplayer-server.c -o server

clean:
	rm client
	rm server
	rm mario

