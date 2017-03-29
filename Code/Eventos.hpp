#ifndef EVENTOS_HPP
#define EVENTOS_HPP

#include <sstream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include "Relacao.hpp"

#define MIN_PARTICIPANTES 6

#define MAX_BONDADE_REPRESSAO		0.2			

#define MUITOS_INIMIGOS 			2

#define MIN_AMI_ASSASSINA			-0.6
#define AMI_DES_ASSASSINO_INIMIGO	-1.0
#define AMI_DES_ASSASSINO_AMIGO		-0.5
#define ASSASSINA_CONFLITO_FAC		0
#define ASSASSINA_DECLARA_GUERRA	1
#define ASSASSINA_TOMA_TERRITORIO	2
#define ASSASSINA_GUERRA			3
#define ASSASSINA_NOVO_LIDER		4
#define ASSASSINA_INIMIGOS			5
#define ASSASSINA_VINGANCA			6
#define ASSASSINA_DUELO				7

#define DUELA_VINGANCA				0
#define DUELA_DECLARA_GUERRA		1
#define DUELA_MORTE_BATALHA			2

#define CONFLITO_INIMIGOS			0
#define CONFLITO_REPRESSAO			1
#define CONFLITO_TOMA				2

#define DECLARA_VINGANCA_P1_P2		0
#define DECLARA_VINGANCA_P2_P1		1
#define DECLARA_INIMIZADE			2

///The following are macros that define the probabilities of certain events

#define REDUZ_CONFLITO_FAC 			0.3
#define REDUZ_CONFLITO_FAC_POVO 	0.05
#define REDUZ_DECLARA_GUERRA 		0.4
#define REDUZ_DECLARA_GUERRA_POVO 	0.07
#define REDUZ_GUERRA			 	0.5
#define REDUZ_GUERRA_POVO 			0.1
#define REDUZ_DISPUTA_TERRITORIO	0.1
#define REDUZ_REPRESSAO_POP			0.2
#define REDUZ_REPRESSAO_POP_LIDER	0.5
#define REDUZ_CONFLITO_CIVIL		0.2
#define REDUZ_CONFLITO_CIVIL_LIDER	0.5

#define PROB_TOMA_TERRITORIO		0.05
#define PROB_DISPUTA_TERRITORIO		0.001
#define PROB_DESCOBRE_ASSASSINO		0.3
#define PROB_REPRESSAO_POP			0.1
#define PROB_CONFLITO_CIVIL			0.1

///List of events and their IDs

#define NUM_EVENTOS 		12
#define CONFLITO_FAC 		0
#define DECLARA_GUERRA		1
#define TOMA_TERRITORIO		2
#define GUERRA				3
#define NOVO_LIDER			4
#define MORTE_BATALHA		5
#define ASSASSINA 			6
#define DISPUTA_TERRITORIO	7
#define DESCOBRE_ASSASSINO	8
#define DUELO				9
#define REPRESSAO_POP		10
#define CONFLITO_CIVIL		11

typedef struct Dados_Evento {
	int ocorrencias;
	long int ultima_ocr;
	int participante[MAX_PARTICIPANTES];
}Dados_Evento;

///********************* CLASSES DE EVENTO *****************************
class Evento {
	public:
		int extra[MAX_PARTICIPANTES-2];
		
		Evento() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		~Evento() {};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {return false;};
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {return 0;};
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {};
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {};
};

class Conflito_Fac : public Evento {
	public:
		Conflito_Fac() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual ~Conflito_Fac() {};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Declara_Guerra : public Evento {
	public:
		Declara_Guerra() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual ~Declara_Guerra() {};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Guerra : public Evento {
	///VETOR EXTRA: armazena os personagens que morreram na guerra
	public:
		Guerra() {};
		virtual ~Guerra() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Toma_Territorio : public Evento {
	///VETOR EXTRA: posição 0 = vencedor
	///				posição 1 = perdedor
	public:
		Toma_Territorio() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual ~Toma_Territorio() {};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Novo_Lider : public Evento {
	public:
		Novo_Lider() {};
		virtual ~Novo_Lider() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Morte_Batalha : public Evento {
	public:
		Morte_Batalha() {};
		virtual ~Morte_Batalha() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Disputa_Territorio : public Evento {
	public:
		Disputa_Territorio() {};
		virtual ~Disputa_Territorio() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Assassina : public Evento {
	///VETOR EXTRA: posição 0: motivo de p1 assassinar p2
	///				posição 1: personagem que contratou p1 para matar p2 (usado apenas se extra[0] == ASSASSINA_VINGANCA) ou amigo de p1 que morreu em um duelo com p2
	///				posição 2: personagem que era amigo de extra[1] e que foi morto por p2 (usado apenas se extra[0] == ASSASSINA_VINGANCA)
	public:
		Assassina() {};
		virtual ~Assassina() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Descobre_Assassino : public Evento {
	///VETOR EXTRA: posição 0: amigo de p1 assassinado por p2
	public:
		Descobre_Assassino() {};
		virtual ~Descobre_Assassino() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Duelo : public Evento {
	///VETOR EXTRA:	posição 0: motivo de p1 chamar p2 para um duelo
	///				posição 1: amigo de p1 morto em batalha por p2 ou então amigo de p1 que foi assassinado por p2
	///				posição 2: vencedor
	///				posição 3: perdedor
	public:
		Duelo() {};
		virtual ~Duelo() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};


class Repressao_Pop : public Evento {
	public:
		Repressao_Pop() {};
		virtual ~Repressao_Pop() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Conflito_Civil : public Evento {
	///VETOR EXTRA:	posição 0: motivo do conflito
	///				posição 1: número de líderes que são inimigos de p2
	///				posição 2:
	public:
		Conflito_Civil() {};
		virtual ~Conflito_Civil() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Guerra_Civil : public Evento {
	public:
		Guerra_Civil() {};
		virtual ~Guerra_Civil() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Golpe_Civil : public Evento {
	public:
		Golpe_Civil() {};
		virtual ~Golpe_Civil() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};

class Festival : public Evento {
	public:
		Festival() {};
		virtual ~Festival() {for (int i = 0; i < MAX_PARTICIPANTES-2; i++)
						extra[i] = -1;};
		virtual bool pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual float pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
		virtual void consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2);
};












///********************** REDE DE EVENTOS ******************************
class Rede_Eventos {
	public:
		Dados_Evento ***evento_ocorrido;///Matriz NxNxE (N = número de personagem e E = Número de Eventos) que indica quais eventos
										///já ocorreram para cada par de personagens
		Lista_Evento *prox_eventos;		///Lista que armazena os eventos cujas pré-condições já foram satisfeitas
		Evento *evento[NUM_EVENTOS];	///Vetor da classe abstrata 'Evento' que contém a referência para um objeto de cada um dos eventos possíveis
		Relacao *personagens;			///Classe contendo informações de relações entre personagens
		
		
		Rede_Eventos(Relacao *per);
		~Rede_Eventos();
		void eventos_iniciais ();
		void busca_novos_eventos (int p1, int p2);
		void escolhe_evento ();
};

#endif
