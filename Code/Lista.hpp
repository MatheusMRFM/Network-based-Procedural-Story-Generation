#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

//Tentar definir isso no Eventos.hpp (se der tempo....)
#define MAX_PARTICIPANTES 6		///Número máximo de participantes em um evento (deve ser >= MIN_PARTICIPANTES)

///*********************************************************************
///************************ LISTA RELACAO ******************************
///*********************************************************************
typedef struct Item {
	int id;
	float f;
	
	Item *ant;
	Item *prox;
}Item;

class ListaDE {	
	public:
		Item *first;
		int num_itens;
		
		ListaDE();
		~ListaDE();
		void insere_ordenado (int indice, float valor);
		void insere_fim (int indice, float valor);
		void insere_inicio (int indice, float valor);
		void imprime ();
		Item* remove_topo ();
		Item* remove (int indice);
		bool vazia () { if (this->first == NULL) 
							return true;
						return false;
		};
};

///*********************************************************************
///************************ LISTA EVENTOS ******************************
///*********************************************************************
typedef struct Item_Evento {
	int id;
	int participante[MAX_PARTICIPANTES];
	float p;
	
	Item_Evento *ant;
	Item_Evento *prox;
}Item_Evento;

class Lista_Evento {
	public:
		Item_Evento *first;
		int num_itens;
		
		Lista_Evento();
		~Lista_Evento();
		bool atualiza_item (Item_Evento item);
		void insere_ordenado (Item_Evento item);
		void imprime ();
		Item_Evento* remove_topo ();
		bool vazia () { if (this->first == NULL) 
							return true;
						return false;
		};
};


#endif //LISTA_H_INCLUDED
