CFLAGS+= -Wall -lpthread
SOURCES = $(wildcard src/*.cpp)
OBJS := $(patsubst %.cpp, %.o,$(SOURCES))
DEPE=${OBJS:%.o=%.d}

CC = g++
all:st
st: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o st
	
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
	rm -f st