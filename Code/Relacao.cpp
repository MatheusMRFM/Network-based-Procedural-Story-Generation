#include <math.h>
#include "Relacao.hpp"

#define PI 3.14159265358979323846
#define VALOR_GRANDE 100000000000

using namespace std;

/// non-deterministic int generator to choose seeds
random_device rd;
unsigned int seed = rd();   

///generator of random numbers
mt19937 gen(seed);  					/// to seed mersenne twister.
uniform_real_distribution<> uni(0,1); 	/// distribute results.	

Relacao::Relacao (int v, int fac, float margem, float pct, int k, int m, float p, float a) {
	///Seta as variáveis
	this->num_aresta = 0;
	this->num_vert = v;
	this->num_faccoes = fac;
	this->margem_tam_fac = margem;
	this->pct_aresta_fac = pct;
	this->k_L = k;
	this->k_SF = m;
	this->prob_SW = p;
	this->alpha = a;
	this->desvio_p = 0.3;
	this->bondade_base = 0.4;
	
	///Aloca estruturas
	peso = (float*) malloc (sizeof(float)*v);
	faccoes = (Faccao*) malloc (sizeof(Faccao)*fac);
	for (int f = 0; f < fac; f++)
		faccoes[f].dominada = NAO_DOMINADA;
	amizades = (float**) malloc (sizeof(float*)*v);
	caracteristica = (float**) malloc (sizeof(float*)*v);
	funcao_personagem = (int*) malloc (sizeof(int)*v);
	nome = (char**) malloc (num_vert*sizeof(char*));
	float s;
	uniform_real_distribution<> uniFor(0.0, 0.4);
	for (int i = 0; i < v; i++) {
		nome[i] = (char*) malloc (TAM_NOME*sizeof(char));
		amizades[i] = (float*) malloc (sizeof(float)*v);
		caracteristica[i] = (float*) malloc (sizeof(float)*NUM_CARACTERISTICAS);
		caracteristica[i][IND_SAUDE] = VIVO;
		caracteristica[i][IND_LIDER_DE] = -1;
		caracteristica[i][IND_FORCA] = uniFor(gen);
		s = uni(gen);
		caracteristica[i][IND_SEXO] = MASCULINO;
		if (s < 0.5)
			caracteristica[i][IND_SEXO] = FEMININO;
		funcao_personagem[i] = POVO;
		for (int j = 0; j < v; j++) {
			amizades[i][j] = SEM_AMIZADE;
		}
	}
	
	///Inicializa algumas estruturas
	this->separa_faccoes();
	this->define_peso_vertices();
}
//**********************************************************************
Relacao::~Relacao() {
	for (int i = 0; i < num_vert; i++) {
		free(amizades[i]);
		free(caracteristica[i]);
	} 
	free(amizades);
	free(caracteristica);
	free(faccoes);
	free(peso);
	free(funcao_personagem);
}
//**********************************************************************
void Relacao::cria_rede_aleatoria (int n_aresta) {
	printf("Criando rede aleatoria...\n");
	uniform_int_distribution<> uniVert(0,num_vert-1);
	int a, b;
	
	for (int i = 0; i < n_aresta; i++) {
		a = uniVert(gen);
		b = uniVert(gen);
		while (a == b)
			b = uniVert(gen);
		if (amizades[a][b] == SEM_AMIZADE)
			num_aresta += 1;
		amizades[a][b] = INIT_AMIZADE;
	}
	printf("....rede aleatoria criada!\n");
}
//**********************************************************************
void Relacao::cria_rede () {
	this->cria_lattice();
	printf("Lattice criado....\n");
	//Relacao->trans_small_world();
	this->scale_free();
	printf("Rede livre de escala criada....\n");
	this->conecta_faccoes();
	printf("Facções conectadas...\n");
}
//**********************************************************************
void Relacao::converte_CSV () {
	char nome_arq[100];
	sprintf(nome_arq, "Output/Relacao_%d_%d_%.2f.csv", num_vert, k_L, prob_SW);
	FILE *arq = fopen(nome_arq, "w");
	
	for (int i = 0; i < num_vert; i++) {
		fprintf(arq, "%d", i);
		for (int j = 0; j < num_vert; j++) {
			if (amizades[i][j] != SEM_AMIZADE)
				fprintf(arq, ";%d", j);
		}
		fprintf(arq, "\n");
	}
}
//**********************************************************************
void Relacao::separa_faccoes () {
	int tam = num_vert/num_faccoes;
	int resto = num_vert % num_faccoes;
	int b = 0, exato[num_faccoes];
	float m;
	uniform_real_distribution<> uniNeg(-1,1);
	
	for (int f = 0; f < num_faccoes; f++) {
		exato[f] = b + tam;
		if (resto > 0) {
			exato[f]++;
			resto--;
		}
		///Pertuba o valor esperado
		if (f != num_faccoes-1) {
			m = uniNeg(gen)*margem_tam_fac;
			faccoes[f].fim = exato[f] + m*tam;
		}
		else
			faccoes[f].fim = exato[f]-1;
			
		///Define o indice do primeiro personagem da facção
		if (f == 0)
			faccoes[f].ini = 0;
		else
			faccoes[f].ini = faccoes[f-1].fim + 1;
		faccoes[f].tam = faccoes[f].fim - (faccoes[f].ini - 1);
		
		b = exato[f];
	}
	
	///Define a facção de cada personagem
	int fac = 0;
	for (int i = 0; i < num_vert; i++) {
		if (i > faccoes[fac].fim)
			fac++;
		caracteristica[i][IND_FACCAO] = fac;
	}
}
//**********************************************************************
void Relacao::define_peso_vertices () {
	float peso_total;
	
	for (int f = 0; f < num_faccoes; f++) {
		peso_total = 0.0;
		///Define pesos com base no modelo de rede livre de escala
		for (int i = faccoes[f].ini; i < faccoes[f].fim; i++) {
			peso[i] = pow((i-faccoes[f].ini+1), -alpha);
			peso_total += peso[i];
		}
		///Normaliza os pesos de cada vértice
		for (int i = faccoes[f].ini; i < faccoes[f].fim; i++) {
			peso[i] = peso[i]/peso_total;
		}
	}
}
//**********************************************************************
void Relacao::cria_lattice() {
	int aux;
	for (int i = 0; i < num_vert; i++) {
		for (int x = 1; x <= k_L; x++) {
			aux = i+x;
			if (aux >= num_vert)
				aux -= num_vert;
			amizades[i][aux] = INIT_AMIZADE;
			
			aux = i-x;
			if (aux < 0)
				aux += num_vert;
			amizades[i][aux] = INIT_AMIZADE;
			
			num_aresta += 2;
		}
	}
}
//**********************************************************************
void Relacao::trans_small_world() {
	float aleatorio;
	int v;
	uniform_int_distribution<> uniVert(0,num_vert);
	
	for (int i = 0; i < num_vert; i++) {
		for (int j = 0; j < num_vert; j++) {
			if (amizades[i][j] != SEM_AMIZADE) {
				aleatorio = uni(gen);
				if (aleatorio <= prob_SW) {
					v = uniVert(gen);
					if (v != i) {
						amizades[i][j] = SEM_AMIZADE;
						amizades[i][v] = INIT_AMIZADE;
					}
				}
			}
		}
	}
}
//**********************************************************************
void Relacao::conecta_faccoes () {
	int nv = pct_aresta_fac*num_aresta;
	float peso_fac[num_faccoes];
	int f1, f2, v1, v2;
	
	///Calcula o peso de cada facção com base no número de vertices de cada
	int n;
	for (int f = 0; f < num_faccoes; f++) {
		n = faccoes[f].tam;
		peso_fac[f] = (float)n / (float)num_vert;
	}
	
	///Cria as arestas entre vértices de facções distintas
	for (int i = 0; i < nv; i++) {
		///Seleciona 2 facções distintas
		f1 = this->seleciona_faccao_aleatoria(peso_fac, uni(gen));
		f2 = f1;
		while (f2 == f1)
			f2 = this->seleciona_faccao_aleatoria(peso_fac, uni(gen));
		///Seleciona um vértice de cada facção e cria uma aresta entre estes
		v1 = this->busca_vert_pesos(uni(gen), f1);	
		v2 = this->busca_vert_pesos(uni(gen), f2);	
		if (amizades[v1][v2] == SEM_AMIZADE) {
			amizades[v1][v2] = INIT_AMIZADE;
			num_aresta++;
		}
	}
}
//**********************************************************************
int Relacao::seleciona_faccao_aleatoria (float *peso, float p) {
	float peso_ant = 0.0;
	
	for (int f = 0; f < num_faccoes; f++) {
		if (p <= peso[f] + peso_ant) {
			return f;
		}
		peso_ant += peso[f];	
	}
	
	return 0;
}
//**********************************************************************
int Relacao::busca_vert_pesos (float p, int fac) {
	float peso_ant = 0.0;
		
	for (int i = faccoes[fac].ini; i <= faccoes[fac].fim; i++) {
		if (p <= peso[i] + peso_ant)
			return i;
		peso_ant += peso[i];	
	}
	
	return 0;
}
//**********************************************************************
void Relacao::scale_free () {
	int v1, v2, t;
	int nv;
	
	for (int f = 0; f < num_faccoes; f++) {
		uniform_int_distribution<> uniFac(faccoes[f].ini, faccoes[f].fim);
		nv = faccoes[f].tam;
		if (nv < k_SF)
			printf("VAI DAR ERRO!!!\n");
		for (int i = 0; i < nv*k_SF; i++) {
			t = 0;
			v1 = this->busca_vert_pesos(uni(gen), f);	
			v2 = this->busca_vert_pesos(uni(gen), f);	
			//printf("(%d, %d)\n", v1, v2);
			while (amizades[v1][v2] != SEM_AMIZADE || v1 == v2) {
				if (t > 10) {
					v1 = uniFac(gen);
					v2 = uniFac(gen);
				}
				else {
					v1 = this->busca_vert_pesos(uni(gen), f);	
					v2 = this->busca_vert_pesos(uni(gen), f);	
					t++;
				}
			}
			amizades[v1][v2] = INIT_AMIZADE;
			num_aresta++;
		}
		/*for (int i = faccoes[f].ini; i <= faccoes[f].fim; i++) {
			for (int j = 0; j < k_SF; j++) {
				v1 = this->busca_vert_pesos(uni(gen), f);
				while (v1 == i || amizades[i][v1] != SEM_AMIZADE)
					v1 = this->busca_vert_pesos(uni(gen), f);
				amizades[i][v1] = INIT_AMIZADE;
				num_aresta++;
			}
		}*/
	}
}
//**********************************************************************
void Relacao::define_relacoes() {
	this->define_nivel_bondade();
	this->define_amizades();
	this->verifica_lideres_inimigos();
}
//**********************************************************************
void Relacao::define_nomes (char** nome_masc, char** nome_fem, char** sobrenome, int nm, int nf, int sr) {
	uniform_int_distribution<> uniMas(0, nm-1);
	uniform_int_distribution<> uniFem(0, nf-1);
	uniform_int_distribution<> uniSr(0, sr-1);
	int e;
	
	///Define os nomes de facção
	for (int f = 0; f < num_faccoes; f++) {
		e = uniSr(gen);
		while (sobrenome[e][0] == '\0') {
			e++;
			if (e > sr)
				e = 0;
		}
		strcpy(faccoes[f].nome, sobrenome[e]);
		///Marca o nome como usado
		sobrenome[e][0] = '\0';
	}
	
	///Define os nomes dos homens e mulheres
	for (int i = 0; i < num_vert; i++) {
		if (caracteristica[i][IND_SEXO] == MASCULINO) {
			e = uniMas(gen);
			while (nome_masc[e][0] == '\0') {
				e++;
				if (e > nm)
					e = 0;
			}
			strcpy(nome[i], nome_masc[e]);
			///Marca o nome como usado
			nome_masc[e][0] = '\0';
		}
		else {
			e = uniFem(gen);
			while (nome_fem[e][0] == '\0') {
				e++;
				if (e > nf)
					e = 0;
			}
			strcpy(nome[i], nome_fem[e]);
			///Marca o nome como usado
			nome_fem[e][0] = '\0';
		}
	}
}
//**********************************************************************
void Relacao::define_nivel_bondade () {
	float u1, u2, z;
	
	for (int i = 0; i < num_vert; i++) {
		///Método de Box-Muller para gerar valores aleatórios com uma distribuição normal
		u1 = uni(gen);
		u2 = uni(gen);
		z = sqrt(-2.0 * log(u1)) * cos(2.0*PI*u2) * desvio_p;
		this->caracteristica[i][IND_BONDADE] = bondade_base + z;
	}
}
//**********************************************************************
void Relacao::define_amizades () {
	float r, a;
	uniform_real_distribution<> uniAux(0, MAX_AMIZADE);
	
	for (int i = 0; i < num_vert; i++) {
		for (int j = 0; j < num_vert; j++) {
			if (amizades[i][j] == INIT_AMIZADE) { 
				r = uni(gen);
				a = uniAux(gen);
				///i e j serão amigos
				if (r <= caracteristica[i][IND_BONDADE]) {
					amizades[i][j] = a;
				}
				///i e j serão inimigos
				else {
					amizades[i][j] = -a;
				}
			}
		}
	}
}
//**********************************************************************
void Relacao::domina_faccao (int vencedor, int perdedor) {
	faccoes[perdedor].dominada = vencedor;
}
//**********************************************************************
void Relacao::verifica_lideres_inimigos () {
	uniform_int_distribution<> uniLid(0, num_faccoes);
	
	///Verifica se ao menos dois lideres são inimigos
	bool existe = false;
	for (int f = 0; f < num_faccoes; f++) {
		for (int i = 0; i < num_faccoes; i++) {
			if (i != f) {
				if (amizades[faccoes[f].lider][faccoes[i].lider] < AMIZADE_NEUTRA) {
					existe = true;
					break;
				}
			}
		}
		if (existe)
			break;
	}
	///Se não existe nenhuma inimizade entre líderes, cria-se uma
	if (!existe) {
		int p1 = uniLid(gen);
		int p2 = p1;
		while (p2 == p1) 
			p2 = uniLid(gen);
		amizades[faccoes[p1].lider][faccoes[p2].lider] = RIVALIDADE_BASE;
		amizades[faccoes[p2].lider][faccoes[p1].lider] = RIVALIDADE_BASE;
	}
}
//**********************************************************************
void Relacao::define_funcao_personagens () {
	int n;
	ListaDE *lista;
	
	for (int f = 0; f < num_faccoes; f++) {
		n = faccoes[f].tam;
		
		lista = nos_mais_influentes(f);
		seleciona_lider(f, lista);
		seleciona_rebelde(f, lista);
		//seleciona_nobreza(f, n, lista);
		seleciona_guerreiro(f, n, lista);
		seleciona_assassino(f, n, lista);
		delete lista;
	}
}
//**********************************************************************
void Relacao::seleciona_lider (int fac, ListaDE *lista) {
	Item *l = lista->remove_topo();
	funcao_personagem[l->id] = LIDER;
	caracteristica[l->id][IND_LIDER_DE] = fac;
	faccoes[fac].lider = l->id;
	caracteristica[l->id][IND_FORCA] = uni(gen);
	free(l);
}
//**********************************************************************
void Relacao::seleciona_rebelde(int fac, ListaDE *lista) {
	Item *l = lista->remove_topo();
	funcao_personagem[l->id] = REBELDE;
	faccoes[fac].rebelde = l->id;
	caracteristica[l->id][IND_FORCA] = uni(gen);
	free(l);
}
//**********************************************************************
void Relacao::seleciona_nobreza (int fac, int tam_fac, ListaDE *lista) {
	uniform_real_distribution<> uniNobreza(tam_fac*MIN_NOBREZA, tam_fac*MAX_NOBREZA);
	int max = uniNobreza(gen);
	ListaDE *l_aux = new ListaDE();
	Item *nobre;
	
	for (int n = 0; n < max && !lista->vazia(); n++) {
		nobre = lista->remove_topo();
		///O personagem em questão é amigo do lider
		if (amizades[faccoes[fac].lider][nobre->id] != SEM_AMIZADE) {
			funcao_personagem[nobre->id] = NOBRE;
			free(nobre);
		}
		///O personagem em questão não conhece o líder
		else {
			l_aux->insere_inicio(nobre->id, nobre->f);
		}
	}
	
	///Junta as duas listas novamente
	while (!l_aux->vazia()) {
		nobre = l_aux->remove_topo();
		lista->insere_ordenado(nobre->id, nobre->f);
	}
	delete l_aux;
}
//**********************************************************************
void Relacao::seleciona_guerreiro (int fac, int tam_fac, ListaDE *lista) {
	uniform_real_distribution<> uniGue(0, MAX_GUERREIRO);
	uniform_real_distribution<> uniFor(0.7, 1.0);
	int n_gue = uniGue(gen) * tam_fac;
	if (n_gue == 0)
		n_gue = 1;
	int n = 0;
	Item *e;
	
	for (int i = 0; i < n_gue && !lista->vazia(); i++) {
		e = lista->remove_topo();
		printf("\tCONEXOES: %f\n", e->f);
		funcao_personagem[e->id] = GUERREIRO;
		caracteristica[e->id][IND_FORCA] = uniFor(gen);
		n++;
	}
	printf("Guerreiros = %d\n", n);
}
//**********************************************************************
void Relacao::seleciona_assassino (int fac, int tam_fac, ListaDE *lista) {
	uniform_real_distribution<> uniAs(0, MAX_ASSASSINO);
	int n_as = uniAs(gen) * tam_fac;
	if (n_as == 0)
		n_as = 1;
	int n = 0;
	Item *e;
	
	for (int i = 0; i < n_as && !lista->vazia(); i++) {
		e = lista->remove_topo();
		printf("\tCONEXOES: %f\n", e->f);
		funcao_personagem[e->id] = ASSASSINO;
		n++;
	}
	printf("Assassinos = %d\n", n);
}
//**********************************************************************
ListaDE* Relacao::nos_mais_influentes (int fac) {
	ListaDE *lista = new ListaDE();
	
	int n;

	for (int i = faccoes[fac].ini; i <= faccoes[fac].fim; i++) {
		n = 0;
		for (int j = 0; j < num_vert; j++) {
			if (amizades[i][j] != SEM_AMIZADE)
				n++;
		}
		if (n > POUCOS_AMIGOS) {
			lista->insere_ordenado(i, n);
		}
	}
	
	return lista;
}
//**********************************************************************

















