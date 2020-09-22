EXE=FMLserver
CXX = g++
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
CXXFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all:$(EXE)

$(EXE):$(OBJ)
	$(CXX) -g $(CXXFLAGS) -o $(EXE) $(OBJ) $(LIB) -D_THREAD_SAFE

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -g $<

clean:
	@rm $(EXE) $(OBJ) -f

#FMLserver: main.cpp buffer.o Connection.o Eventloop.o fastcgi.o logs.o server.o
#	g++ $(CFLAGS) -o FMLserver main.cpp buffer.o Connection.o Eventloop.o fastcgi.o logs.o server.o $(LIB) -D_THREAD_SAFE

#buffer.o: buffer.cpp
#	g++ $(CFLAGS) -c buffer.cpp

#connection.o: connection.cpp
#	g++ $(CFLAGS) -c connection.cpp

#Eventloop.o: Eventloop.cpp
#	g++ $(CFLAGS) -c Eventloop.cpp

#fastcgi.o: fastcgi.cpp
#	g++ $(CFLAGS) -c fastcgi.cpp

#logs.o: logs.cpp
#	g++ $(CFLAGS) -c logs.cpp

#server.o: server.cpp
#	g++ $(CFLAGS) -c server.cpp

#clean:
#	rm -f *.o FMLserver *~
