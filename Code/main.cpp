#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Relacao.hpp"
#include "Eventos.hpp"

#define TAM_HISTORIA 35		///Maximum number of events per story
#define NUM_NOME_MASC 2943
#define NUM_NOME_FEM 5001
#define NUM_SOBRENOME 100

///Variables containing the possible names
char** nome_masc;
char** nome_fem;
char** sobrenome;

void le_arq_nomes ();

int main (int argc, char** argv) {
	int vertices = 120;		///Number of characters in the story
	int faccoes = 3;		///number of factions. There must be at least 2 factions
	float margem = 0.5;
	float pct = 0.15;		///Multiplier associated to the number of connections between factions that is created
	int k_L = 2;			///Number of connections of each vertex in the Lattice structure
	int k_SF = 4;			///Number of new connections that should be made for each vertex
	float prob_SW = 0.15;
	float alpha = 0.9;		///Alpha parameter
	
	if (vertices >= NUM_NOME_MASC || vertices >= NUM_NOME_FEM || faccoes >= NUM_SOBRENOME) {
		printf("Too many vertexes or too many factions!\n");
		exit(0);
	}
	
	le_arq_nomes();
	Relacao *relacoes = new Relacao(vertices, faccoes, margem, pct, k_L, k_SF, prob_SW, alpha);
	Rede_Eventos *rede_evento = new Rede_Eventos(relacoes);
	relacoes->cria_rede();
	relacoes->converte_CSV();
	relacoes->define_relacoes();
	relacoes->define_funcao_personagens();
	relacoes->define_nomes(nome_masc, nome_fem, sobrenome, NUM_NOME_MASC, NUM_NOME_FEM, NUM_SOBRENOME);
	rede_evento->eventos_iniciais();
	
	for (int i = 0; i < TAM_HISTORIA; i++) {
		if (!rede_evento->prox_eventos->vazia()) {
			//rede_evento->prox_eventos->imprime();
			rede_evento->escolhe_evento();
		}
	}
	
	return 0;
}
//**********************************************************************
void le_arq_nomes () {
	FILE* arq_nm = fopen("Name_Dataset/male.txt", "r");
	FILE* arq_nf = fopen("Name_Dataset/female.txt", "r");
	FILE* arq_sr = fopen("Name_Dataset/surname.txt", "r");
	nome_masc = (char**) malloc (NUM_NOME_MASC*sizeof(char*));
	nome_fem = (char**) malloc (NUM_NOME_FEM*sizeof(char*));
	sobrenome = (char**) malloc (NUM_SOBRENOME*sizeof(char*));
	
	for (int i = 0; i < NUM_NOME_MASC && !feof(arq_nm); i++) {
		nome_masc[i] = (char*) malloc (TAM_NOME*sizeof(char));
		fscanf(arq_nm, "%s\n", nome_masc[i]);
	}
	
	for (int i = 0; i < NUM_NOME_FEM && !feof(arq_nf); i++) {
		nome_fem[i] = (char*) malloc (TAM_NOME*sizeof(char));
		fscanf(arq_nf, "%s\n", nome_fem[i]);
	}
	
	for (int i = 0; i < NUM_SOBRENOME && !feof(arq_sr); i++) {
		sobrenome[i] = (char*) malloc (TAM_NOME*sizeof(char));
		fscanf(arq_sr, "%s\n", sobrenome[i]);
	}
}
