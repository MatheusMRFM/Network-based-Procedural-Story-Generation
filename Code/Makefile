CC = g++ -std=c++11
PARAMETERS = -g -Wall
LIB = -lm

historiaProcedural: main.o Relacao.o Eventos.o Lista.o
	$(CC) $(PARAMETERS) -o ProceduralStory main.o Relacao.o Eventos.o Lista.o $(LIB)

main.o: main.cpp
	$(CC) $(PARAMETERS) -c main.cpp -o main.o $(LIB)
	
Relacao.o: Relacao.cpp Relacao.hpp
	$(CC) $(PARAMETERS) -c Relacao.cpp -o Relacao.o $(LIB)
	
Eventos.o: Eventos.cpp Eventos.hpp
	$(CC) $(PARAMETERS) -c Eventos.cpp -o Eventos.o $(LIB)
	
Lista.o: Lista.cpp Lista.hpp
	$(CC) $(PARAMETERS) -c Lista.cpp -o Lista.o $(LIB)

clean:
	rm -f *.o ProceduralStory *~
