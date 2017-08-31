#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Relacao.hpp"
#include "Eventos.hpp"

#define TAM_HISTORIA 20
#define NUM_NOME_MASC 2943
#define NUM_NOME_FEM 5001
#define NUM_SOBRENOME 100

extern int eventos_pessoais;
extern int descobre;
extern int assassinatos;
extern int duelos;

///Variáveis os nomes dos personagens
char** nome_masc;
char** nome_fem;
char** sobrenome;

void le_arq_nomes ();

int main (int argc, char** argv) {
	int vertices = 120;
	int aresta_aleatoria = 6*vertices;
	int faccoes = 4;		///Deve existir no mínimo 2 facções
	float margem = 0.2;
	float pct = 0.05;
	int k_L = 1;
	int k_SF = 4;
	float prob_SW = 0.15;
	float alpha = 0.98;
	
	if (vertices >= NUM_NOME_MASC || vertices >= NUM_NOME_FEM || faccoes >= NUM_SOBRENOME) {
		printf("Muitos vértices ou muitas facções!\n");
		exit(0);
	}
	
	le_arq_nomes();
	Relacao *relacoes = new Relacao(vertices, faccoes, margem, pct, k_L, k_SF, prob_SW, alpha);
	Rede_Eventos *rede_evento = new Rede_Eventos(relacoes);
	relacoes->cria_rede();
	//relacoes->cria_rede_aleatoria(aresta_aleatoria);
	printf("---Arestas = %d\n", relacoes->num_aresta);
	relacoes->converte_CSV();
	relacoes->define_relacoes();
	relacoes->define_funcao_personagens();
	relacoes->define_nomes(nome_masc, nome_fem, sobrenome, NUM_NOME_MASC, NUM_NOME_FEM, NUM_SOBRENOME);
	rede_evento->eventos_iniciais();
	
	int i;
	for (i = 0; i < TAM_HISTORIA; i++) {
		if (!rede_evento->prox_eventos->vazia()) {
			//rede_evento->prox_eventos->imprime();
			rede_evento->escolhe_evento();
		}
		else
			break;
	}
	
	//FILE *arq = fopen("Random.dat", "a");
	FILE *arq = fopen("Scale_Free.dat", "a");
	fprintf(arq, "%d\n", eventos_pessoais);
	
	
	
	printf("\n\nEVENTOS PESSOAIS = %d\n", eventos_pessoais);
	printf("\tDESCOBRE:\t%d\n\tASSASSINATOS:\t%d\n\tDUELOS:\t\t%d\n\n\n", descobre, assassinatos, duelos);
	
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
