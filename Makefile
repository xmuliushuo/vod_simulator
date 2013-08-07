CFLAGS+= -Wall -lpthread
SOURCES = $(wildcard src/*.cpp)
OBJS := $(patsubst %.cpp, %.o,$(SOURCES))
DEPE=${OBJS:%.o=%.d}

SERVER=src/dbufferdw.o\
	src/dbufferfifo.o\
	src/server.o\
	src/servermain.o\
	src/utils.o\
	src/timer.o

CLIENT=src/clientmain.o\
	src/utils.o\
	src/client.o\
	src/timer.o\
	src/dbufferfifo.o\
	src/dbufferdw.o

CC = g++
all:server client
server: $(SERVER)
	$(CC) $(SERVER) $(CFLAGS) -o server
client: $(CLIENT)
	$(CC) $(CLIENT) $(CFLAGS) -o client
-include $(DEPE)

depend:$(DEPE)
$(DEPE):%.d:%.cpp
	-rm -f $*.d;\
	g++ -MM $<|sed 's,^.*:,$*.o:,' >log.$$$$1;\
	echo $(CC) $(CFLAGS) -c -o $*.o $< >log.$$$$2;\
	cat log.$$$$1 log.$$$$2|sed 's/^$(CC)/\t$(CC)/g' >$@;\
	rm -f log.$$$$*;\

clean:
	rm -rf $(OBJS)
	rm -rf $(DEPE)
	rm -f server