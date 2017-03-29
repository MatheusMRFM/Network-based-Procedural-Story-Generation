#ifndef RELACAO_HPP
#define RELACAO_HPP

#include <sstream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Lista.hpp"

#define MAX_AMIZADE 	 1.0
#define MIN_AMIZADE 	-1.0
#define INIT_AMIZADE	 2.0
#define SEM_AMIZADE 	-2.0
#define AMIZADE_NEUTRA 	0.0
#define RIVALIDADE_BASE	-0.5

#define VIVO	0
#define MORTO 	1

#define NUM_CARACTERISTICAS 6
#define IND_BONDADE 		0
#define IND_FACCAO 			1
#define IND_SAUDE 			2
#define IND_FORCA 			3
#define IND_LIDER_DE 		4
#define IND_SEXO			5

#define LIDER 		0
#define NOBRE 		1
#define GUERREIRO 	2
#define ASSASSINO 	3
#define REBELDE		4
#define POVO 		5

#define FEMININO 	0
#define MASCULINO 	1

#define POUCOS_AMIGOS 5
#define MAX_NOBREZA 0.7
#define MIN_NOBREZA 0.4
#define MAX_GUERREIRO 15
#define MAX_ASSASSINO 5

#define TAM_NOME 		30
#define NAO_DOMINADA	-1
typedef struct Faccao {
	int ini;						///indice do primeiro personagem da facção
	int fim;						///indice do último personagem da facção
	int lider;						///indice do líder da facção
	int rebelde;					///indice do rebelde da facção
	int tam;						///Número de personagens na facção
	int dominada;					///Indica se a facção foi dominada ou não
	char nome[TAM_NOME];			///Nome da facção
}Faccao;

class Relacao {
	public:
		int num_vert;
		int num_aresta;
		float** amizades;		///Matriz de Adjacencia que representa as amizades entre os nós
		float prob_SW;			///Probabilidade de reposicionar as ligações para formar uma rede small world
		int k_L;				///Número de vizinhos de um nó na lattice
		int k_SF;				///Número de vizinhos de um nó na rede scale-free
		float alpha;			///Valor alpha do modelo de criação de redes scale-free
		float *peso;			///Vetor de pesos (normalizados) de cada vértice (usado para redes scale-free)
								///A normalização ocorre para cada facção separadamente
		int num_faccoes;		
		float margem_tam_fac;	///Valor entre 0 e 1 que indica qual a margem de erro permitida para o tamanho de cada
								///faccção em relação ao tamanho simétrico esperado
		Faccao *faccoes;		///Vetor que armazena informações de cada facção
		float pct_aresta_fac;	///esse valor multiplicado pelo numero de arestas existentes indica quantas arestas são
								///criadas entre facções
		float **caracteristica;	///Vetor de características para cada personagem
		float desvio_p;			///Desvio padrão usado para definir o nível de bondade de cada personagem
		float bondade_base;		///Valor base para o nivel de bondade de cada personagem
		int *funcao_personagem;	///Vetor que indica a função de cada personagem na história
		char** nome;			///Nome de cada personagem
	
		Relacao(int v, int fac, float margem, float pct, int k,int m, float p, float alpha);
		~Relacao();
		void converte_CSV ();
		int busca_vert_pesos (float p, int fac);
		int seleciona_faccao_aleatoria (float *peso, float p);
		void cria_rede();
		void conecta_faccoes();
		void cria_lattice();
		void separa_faccoes();
		void define_peso_vertices();
		void scale_free();
		void trans_small_world();
		void define_relacoes();
		void define_nivel_bondade();
		void define_amizades();
		void domina_faccao (int vencedor, int perdedor);
		void verifica_lideres_inimigos ();
		void define_funcao_personagens();
		ListaDE* nos_mais_influentes(int fac);
		void seleciona_lider (int fac, ListaDE *lista);
		void seleciona_rebelde(int fac, ListaDE *lista);
		void seleciona_nobreza (int fac, int tam_fac, ListaDE *lista);
		void seleciona_guerreiro (int fac);
		void seleciona_assassino (int fac);
		void define_nomes (char** nome_masc, char** nome_fem, char** sobrenome, int nm, int nf, int sr);
};

#endif
