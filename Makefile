CC=g++ -Wall
SERVER=server.o servermain.o utils.o
server:$(SERVER)
	$(CC) $(SERVER) -o server

server.o:src/server.cpp src/server.h src/vod.h
	$(CC) -c src/server.cpp -o $@
servermain.o:src/servermain.cpp src/utils.h src/server.h
	$(CC) -c src/servermain.cpp -o $@
utils.o:src/utils.cpp src/utils.h
	$(CC) -c src/utils.cpp -o $@
