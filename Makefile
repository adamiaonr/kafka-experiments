CC=g++
CXXFLAGS=-Wall -std=c++11 $(shell pkg-config --cflags glib-2.0 rdkafka++) -I/usr/local/Cellar/boost/1.81.0_1/include
LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka++) -L/usr/local/Cellar/boost/1.81.0_1/lib -lboost_system -lboost_thread-mt -lboost_chrono

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

producer: producer.o config.o message.o
	$(CC) $(LDLIBS) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~
