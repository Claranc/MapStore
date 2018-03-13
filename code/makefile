main: main.o parameter.o run.o function.o
	g++ main.o parameter.o run.o function.o -o main

main.o: main.cpp Header.h
	g++ -c main.cpp

parameter.o: parameter.cpp 
	g++ -c parameter.cpp

run.o: run.cpp 
	g++ -c run.cpp

function.o: function.cpp
	g++ -c function.cpp
clean:
	rm *.o
