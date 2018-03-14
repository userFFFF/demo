#echo Makefile
objects = echo.o sync_server.o asyn_server.o cfg_reader.o background.o asyn_log.o single.o
echo: $(objects)
	cc -o echo $(objects) -lpthread

echo.o: src/echo.c
	cc -c src/echo.c -I./include

sync_server.o: src/sync_server.c
	cc -c src/sync_server.c -I./include

asyn_server.o: src/asyn_server.c
	cc -c src/asyn_server.c -I./include

cfg_reader.o: src/cfg_reader.c
	cc -c src/cfg_reader.c -I./include

background.o: src/background.c
	cc -c src/background.c

asyn_log.o: src/asyn_log.c
	cc -c src/asyn_log.c

single.o: src/single.c
	cc -c src/single.c

clean:
	rm echo $(objects)
