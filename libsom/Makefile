all: som

DEBUG=

som: CNode.o CSOM.o libsom.so GUI.o main.o
	g++ $(DEBUG) CNode.o CSOM.o GUI.o main.o `pkg-config --libs --cflags gtkmm-2.4` -lboost_thread-mt -o som

main.o: main.cpp
	g++ $(DEBUG) -c main.cpp -fPIC `pkg-config --libs --cflags gtkmm-2.4` -lboost_thread-mt

GUI.o: GUI.cpp GUI.h
	g++ $(DEBUG) -c GUI.cpp -fPIC `pkg-config --libs --cflags gtkmm-2.4` -lboost_thread-mt

CNode.o: CNode.cpp CNode.h
	g++ $(DEBUG) -c CNode.cpp -fPIC

CSOM.o: CSOM.cpp CSOM.h
	g++ $(DEBUG) -c CSOM.cpp -fPIC

#libsom.so: CSOM.o CNode.o ## mac
#	g++ $(DEBUG) -shared -Wl,-dylib_install_name -Wl,libsom.so -o libsom.so CSOM.o CNode.o

libsom.so: CSOM.o CNode.o ## linux
	g++ $(DEBUG) -fPIC -shared -Wl,-soname,libsom.so -o libsom.so CSOM.o CNode.o

clean:
	rm -rf main.o GUI.o CNode.o CSOM.o som libsom.so

