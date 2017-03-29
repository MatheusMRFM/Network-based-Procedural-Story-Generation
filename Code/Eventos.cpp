#include <sstream>
#include <random>
#include "Eventos.hpp"

#define SEM_MUDANCA 	-1
#define VARIAS_MUDANCAS	-2

#define DELTA_CONFLITO 				25
#define DELTA_GUERRA 				50
#define DELTA_DISPUTA 				50
#define DELTA_ASSASSINATO 			40
#define DELTA_DESCOBRE_ASSASSINO 	10
#define DELTA_REPRESSAO_POP		 	10
#define DELTA_CONFLITO_CIVIL	 	20

#define TEMPO_GUERRA_TOMA			2
#define TEMPO_GUERRA_MORTE			1

using namespace std;

random_device rdm;
unsigned int seed_2 = rdm();   
mt19937 gen2(seed_2);  				
uniform_int_distribution<> randMortos(0, MAX_PARTICIPANTES-2);
uniform_real_distribution<> randUm(0, 1);

///Variaveis globais
long int timeline = 0;
int ultimo_assassinato_inimigos = -1;
int ultimo_descobre_assassino = -1;
typedef struct Dado_Mudanca {
	int muda_per;
	int muda_fac;
}Dado_Mudanca;
Dado_Mudanca *mudanca;		///Vetor que indica para cada personagem i qual personagem sua amizade foi alterada (ou conjunto de
							///personagens de outra facção, caso muda_fac != -1, ou seja, i alterou sua amizade com todos os
							///personagens da facção muda_fac
							
///**************************** TODO ***********************************
///2) Fazer p1 assassinar p2 se p2 matou algum amigo de p1 em um duelo


void altera_dados_evento (Dados_Evento ***evento_ocorrido, int p1, int p2, int e) {
	evento_ocorrido[p1][p2][e].ocorrencias++;
	evento_ocorrido[p1][p2][e].ultima_ocr = timeline;
	if (e != DESCOBRE_ASSASSINO && e != NOVO_LIDER && e != MORTE_BATALHA && e != ASSASSINA && e != DUELO && e != REPRESSAO_POP && e != CONFLITO_CIVIL) {
		evento_ocorrido[p2][p1][e].ocorrencias++;
		evento_ocorrido[p2][p1][e].ultima_ocr = timeline;
	}
	timeline++;
}
//**********************************************************************
Rede_Eventos::Rede_Eventos (Relacao *per) {
	personagens = per;
	prox_eventos = new Lista_Evento();
	///Inicializa os eventos
	evento[CONFLITO_FAC] 		= new Conflito_Fac();
	evento[DECLARA_GUERRA] 		= new Declara_Guerra();
	evento[TOMA_TERRITORIO] 	= new Toma_Territorio();
	evento[GUERRA] 				= new Guerra();
	evento[NOVO_LIDER] 			= new Novo_Lider();
	evento[MORTE_BATALHA] 		= new Morte_Batalha();
	evento[ASSASSINA]			= new Assassina();
	evento[DISPUTA_TERRITORIO] 	= new Disputa_Territorio();
	evento[DESCOBRE_ASSASSINO] 	= new Duelo();
	evento[DUELO] 				= new Descobre_Assassino();
	evento[REPRESSAO_POP] 		= new Repressao_Pop();
	evento[CONFLITO_CIVIL] 		= new Conflito_Civil();
	///Variavel global inicializada aqui
	mudanca = (Dado_Mudanca*) malloc (sizeof(Dado_Mudanca) * per->num_vert);
	for (int i = 0; i < personagens->num_vert; i++) {
		mudanca[i].muda_per = SEM_MUDANCA;
		mudanca[i].muda_fac = SEM_MUDANCA;
	}
	
	evento_ocorrido = (Dados_Evento***) malloc (sizeof(Dados_Evento**)*personagens->num_vert);
	for (int i = 0; i < personagens->num_vert; i++) {
		evento_ocorrido[i] = (Dados_Evento**) malloc (sizeof(Dados_Evento*)*personagens->num_vert);
		for (int j = 0; j < personagens->num_vert; j++) {
			evento_ocorrido[i][j] = (Dados_Evento*) malloc (sizeof(Dados_Evento)*NUM_EVENTOS);
			for (int e = 0; e < NUM_EVENTOS; e++) {
				evento_ocorrido[i][j][e].ocorrencias = 0;
				evento_ocorrido[i][j][e].ultima_ocr = -1;
				for (int x = 0; x < MAX_PARTICIPANTES; x++)
					evento_ocorrido[i][j][e].participante[x] = -1;
			}
		}
	}
}
//----------------------------------------------------------------------
Rede_Eventos::~Rede_Eventos () {
	for (int i = 0; i < personagens->num_vert; i++) {
		for (int j = 0; j < personagens->num_vert; j++) {
			free(evento_ocorrido[i][j]);
		}
		free(evento_ocorrido[i]);
	}
	free(evento_ocorrido);
	delete prox_eventos;
}
//----------------------------------------------------------------------
void Rede_Eventos::eventos_iniciais () {
	for (int i = 0; i < personagens->num_vert; i++) {
		for (int j = 0; j < personagens->num_vert; j++) {
			if (i != j)
				busca_novos_eventos(i, j);
		}
	}
}
//----------------------------------------------------------------------
void Rede_Eventos::busca_novos_eventos (int p1, int p2) {
	Item_Evento aux;
	float p;
	
	for (int e = 0; e < NUM_EVENTOS; e++) {
		if (evento[e]->pre_condicoes(personagens, evento_ocorrido, p1, p2) == true) {
			p = evento[e]->pobabilidade_ocorre(personagens, evento_ocorrido, p1, p2);
			aux.id = e;
			aux.p = p;
			for (int x = 0; x < MAX_PARTICIPANTES; x++)
				aux.participante[x] = -1;
			aux.participante[0] = p1;
			aux.participante[1] = p2;
			for (int i = 0; i < MAX_PARTICIPANTES-2; i++) 
				aux.participante[i+2] = evento[e]->extra[i];
			prox_eventos->insere_ordenado(aux);
		}
	}
}
//----------------------------------------------------------------------
void Rede_Eventos::escolhe_evento () {
	Item_Evento* ev = this->prox_eventos->remove_topo();
	int p1 = ev->participante[0], p2 = ev->participante[1];
	while (ev != NULL && !this->evento[ev->id]->pre_condicoes(personagens, evento_ocorrido, p1, p2)) {
		free(ev);
		ev = this->prox_eventos->remove_topo();
	}
	if (ev != NULL) {
		printf("- ");
		this->evento[ev->id]->executa_evento(personagens, evento_ocorrido, p1, p2);
		this->evento[ev->id]->consequencias(personagens, evento_ocorrido, p1, p2);
	}
	this->eventos_iniciais();
}
//**********************************************************************
//************************* Conflito_Fac *******************************
//**********************************************************************
bool Conflito_Fac::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	
	///Verifica se o evento ocorreu recentemente
	if (evento_ocorrido[p1][p2][CONFLITO_FAC].ocorrencias > 0) {
		if (evento_ocorrido[p1][p2][CONFLITO_FAC].ultima_ocr + DELTA_CONFLITO > timeline) {
			return false;
		}
	}
	if (evento_ocorrido[p2][p1][CONFLITO_FAC].ocorrencias > 0) {
		if (evento_ocorrido[p2][p1][CONFLITO_FAC].ultima_ocr + DELTA_CONFLITO > timeline) {
			return false;
		}
	}
		
	///Verifica se os envolvidos estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO) {
		return false;
	}
	
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	if (personagens->faccoes[fac1].lider == p1)
		ok1 = true;
	if (personagens->faccoes[fac2].lider == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;
		
	///Verifica se os envolvidos são inimigos
	if (personagens->amizades[p1][p2] >= AMIZADE_NEUTRA) {
		return false;
	}
		
	return true;
}
//----------------------------------------------------------------------
float Conflito_Fac::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	float p = personagens->amizades[p1][p2] * personagens->amizades[p2][p1];
	return p;
}
//----------------------------------------------------------------------
void Conflito_Fac::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, CONFLITO_FAC);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	//printf("Ocorre um conflito entre as facções %s e %s!\n", personagens->faccoes[fac1].nome, personagens->faccoes[fac2].nome);
	printf("A conflict occurs between the %s and %s factions!\n", personagens->faccoes[fac1].nome, personagens->faccoes[fac2].nome);
}
//----------------------------------------------------------------------
void Conflito_Fac::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	
	///Diminui a amizade dos lideres
	mudanca[p1].muda_per = p2;
	personagens->amizades[p1][p2] -= REDUZ_CONFLITO_FAC;
	if (personagens->amizades[p1][p2] < MIN_AMIZADE)
		personagens->amizades[p1][p2] = MIN_AMIZADE;
	
	///Diminui a amizade do resto da população (apenas para os pares que já são inimigos)
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		for (int j = personagens->faccoes[fac2].ini; j <= personagens->faccoes[fac2].fim; j++) {
			if ((i != p1 || j != p2) && personagens->amizades[i][j] < AMIZADE_NEUTRA) {
				if (personagens->caracteristica[i][IND_SAUDE] != MORTO && personagens->caracteristica[j][IND_SAUDE] != MORTO) {
					mudanca[i].muda_fac = fac2;
					personagens->amizades[i][j] -= REDUZ_CONFLITO_FAC_POVO;
					if (personagens->amizades[i][j] < MIN_AMIZADE)
						personagens->amizades[i][j] = MIN_AMIZADE;
				}
			}
		}
	}
}
//**********************************************************************
//************************ Declara_Guerra ******************************
//**********************************************************************
bool Declara_Guerra::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	this->extra[0] = DECLARA_INIMIZADE;
	
	///Verifica se já teve algum conflito entre as facções lideradas por p1 e p2
	if (evento_ocorrido[p1][p2][CONFLITO_FAC].ocorrencias == 0)
		return false;
		
	///Verifica se p1 já declarou guerra a p2 e ainda não efetuou a guerra
	if (evento_ocorrido[p1][p2][DECLARA_GUERRA].ocorrencias > 0) {
		///Verifica se a guerra já ocorreu
		if (evento_ocorrido[p1][p2][GUERRA].ocorrencias > 0) {
			///Verifica se a guerra ocorreu antes da ultima declaração de guerra
			if (evento_ocorrido[p1][p2][GUERRA].ultima_ocr < evento_ocorrido[p1][p2][DECLARA_GUERRA].ultima_ocr)
				return false;
			///Verifica se a guerra ocorreu recentemente
			if (evento_ocorrido[p1][p2][GUERRA].ultima_ocr + DELTA_GUERRA > timeline) 
				return false;
		}
		else 
			return false;
	}
		
	///Verifica se p2 já declarou guerra à p1
	if (evento_ocorrido[p2][p1][DECLARA_GUERRA].ocorrencias > 0) {
		///Verifica se a guerra já ocorreu
		if (evento_ocorrido[p2][p1][GUERRA].ocorrencias > 0) {
			///Verifica se a guerra ocorreu antes da ultima declaração de guerra
			if (evento_ocorrido[p2][p1][GUERRA].ultima_ocr < evento_ocorrido[p2][p1][DECLARA_GUERRA].ultima_ocr)
				return false;
			///Verifica se a guerra ocorreu recentemente
			if (evento_ocorrido[p2][p1][GUERRA].ultima_ocr + DELTA_GUERRA > timeline) 
				return false;
		} 
	}
		
	///Verifica se os envolvidos estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
	
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	int lider2 = personagens->faccoes[fac2].lider;
	if (lider1 == p1)
		ok1 = true;
	if (lider2 == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;	
		
	///Verifica se alguem da facção p1 descobriu que alguem da faccao de p2 assassinou algum lider da faccao de p1
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		for (int j = personagens->faccoes[fac2].ini; j <= personagens->faccoes[fac2].fim; j++) {
			if (evento_ocorrido[i][j][DESCOBRE_ASSASSINO].ocorrencias > 0) {
				///Verifica se quem j assassinou era um líder ou não
				int morto = evento_ocorrido[i][j][DESCOBRE_ASSASSINO].participante[2];
				if (personagens->caracteristica[morto][IND_FACCAO] == fac1 && personagens->funcao_personagem[morto] == LIDER) {
					///Verifica se os lideres das facções fac1 e fac2 já declaram guerra depois do evento DESCOBRE_ASSASSINO
					if (evento_ocorrido[i][j][DESCOBRE_ASSASSINO].ultima_ocr > evento_ocorrido[lider1][lider2][DECLARA_GUERRA].ultima_ocr) {
						///Indica que o evento DECLARA_GUERRA ocorrerá por vingança
						this->extra[0] = DECLARA_VINGANCA_P1_P2;
						this->extra[1] = i;
						this->extra[2] = j;
					}
				}
			}
			if (evento_ocorrido[j][i][DESCOBRE_ASSASSINO].ocorrencias > 0) {
				///Verifica se quem j assassinou era um líder ou não
				int morto = evento_ocorrido[j][i][DESCOBRE_ASSASSINO].participante[2];
				if (personagens->caracteristica[morto][IND_FACCAO] == fac2 && personagens->funcao_personagem[morto] == LIDER) {
					///Verifica se os lideres das facções fac1 e fac2 já declaram guerra depois do evento DESCOBRE_ASSASSINO
					if (evento_ocorrido[j][i][DESCOBRE_ASSASSINO].ultima_ocr > evento_ocorrido[lider2][lider1][DECLARA_GUERRA].ultima_ocr) {
						///Indica que o evento DECLARA_GUERRA ocorrerá por vingança
						this->extra[0] = DECLARA_VINGANCA_P2_P1;
						this->extra[1] = j;
						this->extra[2] = i;
					}
				}
			}
		}
	}
		
	///Verifica se os envolvidos são inimigos
	if (this->extra[0] == DECLARA_INIMIZADE && personagens->amizades[p1][p2] >= AMIZADE_NEUTRA)
		return false;
		
	return true;
}
//----------------------------------------------------------------------
float Declara_Guerra::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	float p = 1.0;
	if (this->extra[0] == DECLARA_INIMIZADE)
		p = personagens->amizades[p1][p2] * personagens->amizades[p2][p1];
		
	return p;
}
//----------------------------------------------------------------------
void Declara_Guerra::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, DECLARA_GUERRA);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	if (this->extra[0] == DECLARA_INIMIZADE) {
		//printf("A facção %s declara guerra à facção %s.\n", n1, n2);
		printf("The %s faction declares war to the %s faction.\n", n1, n2);
	}
	else if (this->extra[0] == DECLARA_VINGANCA_P1_P2) {
		char* na = personagens->nome[this->extra[2]];
		char* nf = personagens->faccoes[(int)personagens->caracteristica[this->extra[2]][IND_FACCAO]].nome;
		//printf("A facção %s declara guerra à facção %s após descobrir sobre o assassinato de seu antigo líder pelas mãos do assassino %s, da facção %s.\n", n1, n2, na, nf);
		printf("The %s faction declares war to the %s faction after discovering that the assassin %s, from the %s faction, murdered their previous leader.\n", n1, n2, na, nf);
	}
	else {
		char* na = personagens->nome[this->extra[2]];
		char* nf = personagens->faccoes[(int)personagens->caracteristica[this->extra[2]][IND_FACCAO]].nome;
		//printf("A facção %s declara guerra à facção %s após descobrir sobre o assassinato de seu antigo líder pelas mãos do assassino %s, da facção %s.\n", n2, n1, na, nf);
		printf("The %s faction declares war to the %s faction after discovering that the assassin %s, from the %s faction, murdered their previous leader.\n", n2, n1, na, nf);
	}
}
//----------------------------------------------------------------------
void Declara_Guerra::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	
	///Diminui a amizade dos lideres
	mudanca[p1].muda_per = p2;
	personagens->amizades[p1][p2] -= REDUZ_DECLARA_GUERRA;
	if (personagens->amizades[p1][p2] < MIN_AMIZADE)
		personagens->amizades[p1][p2] = MIN_AMIZADE;
	
	///Diminui a amizade do resto da população (apenas para os pares que já são inimigos)
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		for (int j = personagens->faccoes[fac2].ini; j <= personagens->faccoes[fac2].fim; j++) {
			if ((i != p1 || j != p2) && personagens->amizades[i][j] < AMIZADE_NEUTRA) {
				if (personagens->caracteristica[i][IND_SAUDE] != MORTO && personagens->caracteristica[j][IND_SAUDE] != MORTO) {
					mudanca[i].muda_fac = fac2;
					personagens->amizades[i][j] -= REDUZ_DECLARA_GUERRA_POVO;
					if (personagens->amizades[i][j] < MIN_AMIZADE)
						personagens->amizades[i][j] = MIN_AMIZADE;
				}
			}
		}
	}
}
//**********************************************************************
//***************************** Guerra *********************************
//**********************************************************************
bool Guerra::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se p1 já declarou guerra à p2
	if (evento_ocorrido[p1][p2][DECLARA_GUERRA].ocorrencias == 0)
		return false;
		
	///Verifica se p1 e p2 duelaram
	if (evento_ocorrido[p1][p2][DUELO].ocorrencias > 0 || evento_ocorrido[p2][p1][DUELO].ocorrencias > 0)
		return false;
	
	///Verifica se a guerra entre p1 e p2 (ou entre p2 e p1) já ocorreu a pouco tempo	
	if (evento_ocorrido[p1][p2][GUERRA].ocorrencias > 0) {
		if (evento_ocorrido[p1][p2][GUERRA].ultima_ocr + DELTA_GUERRA > timeline) {
			return false;
		}
	}
	if (evento_ocorrido[p2][p1][GUERRA].ocorrencias > 0) {
		if (evento_ocorrido[p2][p1][GUERRA].ultima_ocr + DELTA_GUERRA > timeline) {
			return false;
		}
	}
		
	///Verifica se os envolvidos estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
	
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	if (personagens->faccoes[fac1].lider == p1)
		ok1 = true;
	if (personagens->faccoes[fac2].lider == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;
		
	///Verifica se os envolvidos são inimigos
	if (personagens->amizades[p1][p2] >= AMIZADE_NEUTRA)
		return false;
		
	return true;
}
//----------------------------------------------------------------------
float Guerra::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	float p = personagens->amizades[p1][p2] * personagens->amizades[p2][p1];
	return p;
}
//----------------------------------------------------------------------
void Guerra::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, GUERRA);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	//printf("Uma guerra sangrenta ocorre entre as facções %s e %s!\n", personagens->faccoes[fac1].nome, personagens->faccoes[fac2].nome);
	printf("A bloody war breaks out between the %s and %s factions!\n", personagens->faccoes[fac1].nome, personagens->faccoes[fac2].nome);
}
//----------------------------------------------------------------------
void Guerra::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	
	///Escolhe entre 0 - (MAX_PARTICIPANTES -2) personagens para morrer
	for (int i = 2; i < MAX_PARTICIPANTES; i++) 
		evento_ocorrido[p1][p2][GUERRA].participante[i] = -1;
	int mortos = randMortos(gen2), m, a = 2;
	uniform_int_distribution<> randPer(0,personagens->num_vert-1);
	for (int i = 0; i < mortos; i++) {
		m = randPer(gen2);
		if (personagens->caracteristica[m][IND_FACCAO] == fac1 || personagens->caracteristica[m][IND_FACCAO] == fac2) {
			if (personagens->caracteristica[m][IND_SAUDE] != MORTO) {
				evento_ocorrido[p1][p2][GUERRA].participante[a] = m;
				//printf("\t%d morreu na guerra!\n", m);
				a++;
			}
		}
		else
			i--;
	}
	
	///Diminui a amizade dos lideres
	mudanca[p1].muda_per = p2;
	personagens->amizades[p1][p2] -= REDUZ_GUERRA;
	if (personagens->amizades[p1][p2] < MIN_AMIZADE)
		personagens->amizades[p1][p2] = MIN_AMIZADE;
	
	///Diminui a amizade do resto da população (apenas para os pares que já são inimigos)
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		for (int j = personagens->faccoes[fac2].ini; j <= personagens->faccoes[fac2].fim; j++) {
			if ((i != p1 || j != p2) && personagens->amizades[i][j] < AMIZADE_NEUTRA) {
				if (personagens->caracteristica[i][IND_SAUDE] != MORTO && personagens->caracteristica[j][IND_SAUDE] != MORTO) {
					mudanca[i].muda_fac = fac2;
					personagens->amizades[i][j] -= REDUZ_GUERRA_POVO;
					if (personagens->amizades[i][j] < MIN_AMIZADE)
						personagens->amizades[i][j] = MIN_AMIZADE;
				}
			}
		}
	}
}
//**********************************************************************
//************************ Toma_Territorio *****************************
//**********************************************************************
bool Toma_Territorio::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	if (personagens->faccoes[fac1].lider == p1)
		ok1 = true;
	if (personagens->faccoes[fac2].lider == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;
		
	///Verifica se uma facção já domina a outra
	if (personagens->faccoes[fac1].dominada == fac2 ||	personagens->faccoes[fac2].dominada == fac1)
		return false;
	
	///Verifica se uma terceira facção já domina a fac1 ou fac2
	if (personagens->faccoes[fac1].dominada != NAO_DOMINADA ||	personagens->faccoes[fac2].dominada != NAO_DOMINADA)
		return false;
		
	///Verifica se os envolvidos estão vivos (pelo menos 1 tem que estar vivo)
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO && personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
	
	///Verifica se alguém da facção de p2 assassinou p1 ou vice-versa
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO) {
		for (int i = 0; i < personagens->num_vert; i++) {
			if (evento_ocorrido[i][p1][ASSASSINA].ocorrencias > 0) {
				///Verifica se o assassino é da facção rival considerada ou não
				if (personagens->caracteristica[i][IND_FACCAO] == fac2)
					return true;
				else
					return false;
			}
		}
	}
	else if (personagens->caracteristica[p2][IND_SAUDE] == MORTO) {
		for (int i = 0; i < personagens->num_vert; i++) {
			if (evento_ocorrido[i][p2][ASSASSINA].ocorrencias > 0) {
				///Verifica se o assassino é da facção rival considerada ou não
				if (personagens->caracteristica[i][IND_FACCAO] == fac1)
					return true;
				else
					return false;
			}
		}
	}
	
	///Verifica se p1 e p2 duelaram
	if (evento_ocorrido[p1][p2][DUELO].ocorrencias > 0 || evento_ocorrido[p2][p1][DUELO].ocorrencias > 0) {
		///Verifica se o perdedor ainda é o líder de sua facção (ou se a mesma já foi dominada)
		int perdedor = evento_ocorrido[p1][p2][DUELO].participante[4];
		if (evento_ocorrido[p1][p2][DUELO].ocorrencias > 0)	perdedor = evento_ocorrido[p2][p1][DUELO].participante[4];
		if (personagens->faccoes[fac1].lider == perdedor || personagens->faccoes[fac2].lider == perdedor)
			return true;
	}
	
	///Verifica se p1 já guerreou contra p2 e se foi recentemente
	if (evento_ocorrido[p1][p2][GUERRA].ocorrencias == 0 || timeline - evento_ocorrido[p1][p2][GUERRA].ultima_ocr > TEMPO_GUERRA_TOMA)
		return false;
		
	///Verifica se os envolvidos são inimigos
	if (personagens->amizades[p1][p2] >= AMIZADE_NEUTRA)
		return false;
		
	///Verifica se alguma facção vai de fato perder a guerra
	///Se algum dos líderes tiver morrido na guerra, então o evento ocorre
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return true;
	///Caso contrário, verifica a possibilidade
	float r = randUm(gen2);
	///Calcula o tamanho das facções
	int tam_fac1 = personagens->faccoes[fac1].tam, tam_fac2 = personagens->faccoes[fac2].tam;
	float peso_fac1 = (float)(tam_fac1) / (float)(tam_fac1 + tam_fac2), peso_maior;
	if (peso_fac1 <= 0.5)
		peso_maior = 1.0 - peso_fac1;
	///Verifica se haverá algum vencedor
	if (r > peso_maior) 
		return false;
		
	return true;
}
//----------------------------------------------------------------------
float Toma_Territorio::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Caso algum dos líderes esteja morto, a tomada de território deve ocorrer imediatamente
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return 1.1;
		
	return PROB_TOMA_TERRITORIO;
}
//----------------------------------------------------------------------
void Toma_Territorio::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, TOMA_TERRITORIO);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int vencedor, perdedor;
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO) {
		vencedor = fac2;
		perdedor = fac1;
	}
	else if (personagens->caracteristica[p2][IND_SAUDE] == MORTO) {
		vencedor = fac1;
		perdedor = fac2;
	}
	else {
		///Calcula o tamanho das facções
		int tam_fac1 = personagens->faccoes[fac1].tam, tam_fac2 = personagens->faccoes[fac2].tam;
		float peso_fac1 = (float)(tam_fac1) / (float)(tam_fac1 + tam_fac2);
		
		///Verifica quem vai ganhar a guerra
		float r = randUm(gen2);
		if (r <= peso_fac1) {
			vencedor = fac1;
			perdedor = fac2;
		}
		else {
			vencedor = fac2;
			perdedor = fac1;
		}
	}
	
	this->extra[0] = vencedor;
	this->extra[1] = perdedor;
	
	///Domina a facção perdedora
	personagens->domina_faccao(vencedor, perdedor);
	evento_ocorrido[p1][p2][TOMA_TERRITORIO].participante[2] = vencedor;
	evento_ocorrido[p1][p2][TOMA_TERRITORIO].participante[3] = perdedor;
	evento_ocorrido[p2][p1][TOMA_TERRITORIO].participante[2] = vencedor;
	evento_ocorrido[p2][p1][TOMA_TERRITORIO].participante[3] = perdedor;
	
	char* nv = personagens->faccoes[vencedor].nome;
	char* np = personagens->faccoes[perdedor].nome;
	//printf("A facção %s consegue destruir a facção %s e tomar seu território.\n", nv, np);
	printf("The %s faction destroys the %s faction and takes control of their territory.\n", nv, np);
}
//----------------------------------------------------------------------
void Toma_Territorio::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//**********************************************************************
//************************** Novo_Lider ********************************
//**********************************************************************
///p1 escolhe um novo lider pra facção de p2
bool Novo_Lider::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se p1 já tomou o território de p2
	if (evento_ocorrido[p1][p2][TOMA_TERRITORIO].ocorrencias == 0)
		return false;
		
	///Verifica se o lider vencedor está vivo
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO)
		return false;
	
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	if (personagens->faccoes[fac1].lider == p1)
		ok1 = true;
	if (personagens->faccoes[fac2].lider == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;
		
	///Verifica se os envolvidos são inimigos
	if (personagens->amizades[p1][p2] >= AMIZADE_NEUTRA)
		return false;
	
	return true;
}
//----------------------------------------------------------------------
float Novo_Lider::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return 1;
}
//----------------------------------------------------------------------
void Novo_Lider::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Escolhe quem será o novo líder da facção derrotada (cujo antigo líder era p2)
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;

	int escolhido = -1;
	float max = -2;
	for (int i = personagens->faccoes[fac1].ini; i < personagens->faccoes[fac1].fim; i++) {
		if (personagens->funcao_personagem[i] == NOBRE && personagens->amizades[p1][i] > max)
			escolhido = i;
	}
	
	altera_dados_evento(evento_ocorrido, p1, p2, NOVO_LIDER);
	
	char* nl = personagens->nome[p1];
	char* ne = personagens->nome[escolhido];
	
	///Se não existir nenhum candidato para líder, o líder p1 se torna líder de duas facções ao mesmo tempo
	if (escolhido == -1) {
		personagens->faccoes[fac2].lider = p1;
		//printf("O líder da facção %s, %s, se torna o novo líder para a recém dominada facção %s.\n", n1, nl, n2);
		printf("The leader of the %s faction, %s, becomes the new leader of the recently dominated %s faction.\n", n1, nl, n2);
	}
	///Faz o escolhido se tornar o novo lider
	else {
		personagens->faccoes[fac2].lider = escolhido;
		personagens->funcao_personagem[escolhido] = LIDER;
		personagens->caracteristica[escolhido][IND_LIDER_DE] = fac2;
		//printf("A Facção %s elege %s como o novo líder para a recém dominada facção %s.\n", n1, ne, n2);
		printf("The %s faction elects %s as the new leader of the recently dominated %s faction.\n", n1, ne, n2);
	}
}
//----------------------------------------------------------------------
void Novo_Lider::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//**********************************************************************
//************************* Morte_Batalha ******************************
//**********************************************************************
///p1 matou p2 em uma batalha
bool Morte_Batalha::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	int lider2 = personagens->faccoes[fac2].lider;
	///Verifica se teve uma batalha no tempo anterior entre o lider se sua facção contra outra facção
	if (evento_ocorrido[lider1][lider2][GUERRA].ocorrencias == 0 || timeline - evento_ocorrido[lider1][lider2][GUERRA].ultima_ocr > TEMPO_GUERRA_MORTE)
		return false;
		
	///Verifica se p1 é um guerreiro
	if (personagens->funcao_personagem[p1] != GUERREIRO)
		return false;
	
	///Verifica se p2 foi de fato escolhido para ser morto na guerra
	bool ok = false;
	for (int i = 2; i < MAX_PARTICIPANTES; i++) {
		if (evento_ocorrido[lider1][lider2][GUERRA].participante[i] == p2) {
			ok = true;
			break;
		}
	}
	if (!ok)
		return false;
	
	///Verifica se os envolvidos estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
		
	///Verifica se os envolvidos são inimigos
	if (personagens->amizades[p1][p2] >= AMIZADE_NEUTRA)
		return false;
		
	///Verifica se p1 vai de fato matar p2 (com base na força de ambos)
	float p = randUm(gen2);
	float dif = personagens->caracteristica[p1][IND_FORCA] - personagens->caracteristica[p2][IND_FORCA];
	if (dif < 0)	dif = -dif;
	///p1 NÃO conseguiu matar p2
	if (p > dif)
		return false;
	
	return true;
}
//----------------------------------------------------------------------
float Morte_Batalha::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///A probabilidade é maior q 1.0 para forçar a ocorrência deste evento imediatamente após uma batalha;
	return 1.1;
}
//----------------------------------------------------------------------
void Morte_Batalha::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, MORTE_BATALHA);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	
	if (personagens->faccoes[fac1].lider == p1) {
		//printf("Durante a batalha entre as facções %s e %s, o líder da facção %s, %s, derrotou %s durante um duelo sangrento.\n", n1, n2, n1, np1, np2);
		printf("During the battle between the %s and %s factions, the leader of the %s faction, %s, defeated %s in a bloody duel.\n", n1, n2, n1, np1, np2);
	}
	else if (personagens->faccoes[fac2].lider == p2) {
		//printf("Na batalha entre as facções %s e %s, o líder da facção %s, %s, torna-se um alvo e é assasinado de forma brutal por %s.\n", n1, n2, n2, np2, np1);
		printf("In the battle between the %s and %s factions, the leader of the %s faction, %s, becomes the target and ends up being brutally executed by %s.\n", n1, n2, n2, np2, np1);
	}
	else {
		//printf("Durante o caos da batalha entre as facções %s e %s, %s de %s derrotou %s de %s de forma brutal.\n", n1, n2, np1, n1, np2, n2);
		printf("Amidst the chaos of the battle between the %s and %s factions, %s from the %s faction brutally defeated %s from the %s faction.\n", n1, n2, np1, n1, np2, n2);
	}
}
//----------------------------------------------------------------------
void Morte_Batalha::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Mata p2
	personagens->caracteristica[p2][IND_SAUDE] = MORTO;
	
	///Reduz a amizade dos amigos de p2 em relação à p1
	for (int i = 0; i < personagens->num_vert; i++) {
		if (personagens->amizades[i][p2] > AMIZADE_NEUTRA && personagens->caracteristica[i][IND_SAUDE] != MORTO)
			personagens->amizades[i][p1] -= personagens->amizades[i][p2];
	}
}
//**********************************************************************
//************************ Disputa_Territorio **************************
//**********************************************************************
bool Disputa_Territorio::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se os envolvidos estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
		
	///Verifica se já teve uma disputa entre p1 e p2 recentemente (ou entre p2 e p1)
	if (evento_ocorrido[p1][p2][DISPUTA_TERRITORIO].ocorrencias > 0 && evento_ocorrido[p1][p2][DISPUTA_TERRITORIO].ultima_ocr + DELTA_DISPUTA > timeline) 
		return false;
	else if (evento_ocorrido[p2][p1][DISPUTA_TERRITORIO].ocorrencias > 0 && evento_ocorrido[p2][p1][DISPUTA_TERRITORIO].ultima_ocr + DELTA_DISPUTA > timeline) 
		return false;
		
	///Verifica se os envolvidos no evento são líderes de facções
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	if (personagens->faccoes[fac1].lider == p1)
		ok1 = true;
	if (personagens->faccoes[fac2].lider == p2)
		ok2 = true;
	
	if (!ok1 || !ok2)
		return false;
		
	return true;
}
//----------------------------------------------------------------------
float Disputa_Territorio::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return PROB_DISPUTA_TERRITORIO;
}
//----------------------------------------------------------------------
void Disputa_Territorio::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, DISPUTA_TERRITORIO);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	
	//printf("Os líderes %s de %s e %s de %s discutem e se desentendem por conta de um território recentemente descoberto.\n", np1, n1, np2, n2);
	printf("The leaders %s from the %s faction and %s from the %s faction argued and reached a misunderstanding about a recently discovered territory.\n", np1, n1, np2, n2);
}
//----------------------------------------------------------------------
void Disputa_Territorio::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Diminui a amizade entre os líderes
	float diminui = REDUZ_DISPUTA_TERRITORIO;
	///Antes, verifica-se se estes eram amigos
	if (personagens->amizades[p1][p2] > AMIZADE_NEUTRA)
		diminui = (1.0-personagens->amizades[p1][p2])*diminui;
	if (personagens->amizades[p2][p1] - diminui > AMIZADE_NEUTRA)
		diminui = (1.0*personagens->amizades[p2][p1])*diminui;
		
	personagens->amizades[p2][p1] -= diminui;
	personagens->amizades[p1][p2] -= diminui;
}
//**********************************************************************
//*************************** Assassina ********************************
//**********************************************************************
///p1 assassina p2
bool Assassina::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se p1 é assassino
	if (personagens->funcao_personagem[p1] != ASSASSINO)
		return false;
		
	///Verifica se p2 já está morto
	if (personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
		
	///Verifica se p2 é algum líder que teve conflito/guerra/etc com a facção de p1
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	///Verifica também se p1 é amigo do líder de seu clan
	if (personagens->faccoes[fac2].lider == p2 && personagens->amizades[p1][lider1] > AMIZADE_NEUTRA) {
		if (evento_ocorrido[lider1][p2][TOMA_TERRITORIO].ocorrencias > 0) {
			this->extra[0] = ASSASSINA_TOMA_TERRITORIO;
			return true;
		}
		else if (evento_ocorrido[lider1][p2][GUERRA].ocorrencias > 0) {
			this->extra[0] = ASSASSINA_GUERRA;
			return true;
		}
		else if (evento_ocorrido[lider1][p2][DECLARA_GUERRA].ocorrencias > 0) {
			this->extra[0] = ASSASSINA_DECLARA_GUERRA;
			return true;
		}
		else if (evento_ocorrido[lider1][p2][CONFLITO_FAC].ocorrencias > 0) {
			this->extra[0] = ASSASSINA_CONFLITO_FAC;
			return true;
		}
	}
	///Verifica se p2 é o novo líder da facção de p1 (após a facção de p1 ser tomada)
	else if (fac1 != fac2 && lider1 == p2) {
		this->extra[0] = ASSASSINA_NOVO_LIDER;
		return true;
	}
		
	///Verifica se p1 é inimigo de p2
	if (personagens->amizades[p1][p2] < MIN_AMI_ASSASSINA) {
		if (ultimo_assassinato_inimigos == -1 || ultimo_assassinato_inimigos + DELTA_ASSASSINATO <= timeline) {
			this->extra[0] = ASSASSINA_INIMIGOS;
			return true;
		}
	}
	
	///Verifica se p2 assassinou algum amigo de alguem e esse alguem contraou p1 para vingá-lo
	for (int i = 0; i < personagens->num_vert; i++) {
		///Se i descobriu que p2 assassinou seu amigo
		if (evento_ocorrido[i][p2][DESCOBRE_ASSASSINO].ocorrencias > 0) {
			this->extra[0] = ASSASSINA_VINGANCA;
			this->extra[1] = i;
			this->extra[2] = evento_ocorrido[i][p2][DESCOBRE_ASSASSINO].participante[2];	///Amigo de i que foi assassinado
			return true;
		}
	}
	
	///Verifica se p2 matou algum amigo de p1 em um duelo
	for (int i = 0; i < personagens->num_vert; i++) {
		if (personagens->amizades[p1][i] > AMIZADE_NEUTRA) {
			if (evento_ocorrido[i][p2][DUELO].ocorrencias > 0 && evento_ocorrido[i][p2][DUELO].participante[5] == i) {
				this->extra[0] = ASSASSINA_DUELO;
				this->extra[1] = i;
				return true;
			}
			else if (evento_ocorrido[p2][i][DUELO].ocorrencias > 0 && evento_ocorrido[p2][i][DUELO].participante[5] == i) {
				this->extra[0] = ASSASSINA_DUELO;
				this->extra[1] = i;
				return true;
			}
		}
	}
	
	return false;
}
//----------------------------------------------------------------------
float Assassina::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	switch (this->extra[0]) {
		case ASSASSINA_CONFLITO_FAC:
			return 0.05;
			break;
		case ASSASSINA_DECLARA_GUERRA:
			return 0.07;
			break;
		case ASSASSINA_GUERRA:
			return 0.1;
			break;
		case ASSASSINA_TOMA_TERRITORIO:
			return 0.13;
			break;
		case ASSASSINA_NOVO_LIDER:
			return 0.15;
			break;
		case ASSASSINA_INIMIGOS:
			return 0.2*personagens->amizades[p1][p2];
			break;
		case ASSASSINA_VINGANCA:
			return 0.6;
			break;
		case ASSASSINA_DUELO:
			return 0.6;
			break;
	}
	
	return 0;
}
//----------------------------------------------------------------------
void Assassina::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, ASSASSINA);
	evento_ocorrido[p1][p2][ASSASSINA].participante[2] = this->extra[0];
	evento_ocorrido[p1][p2][ASSASSINA].participante[3] = this->extra[1];
	evento_ocorrido[p1][p2][ASSASSINA].participante[4] = this->extra[2];
	
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	char* nl = personagens->nome[lider1];
	char* nf = personagens->faccoes[(int)personagens->caracteristica[lider1][IND_FACCAO]].nome;
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	char *ne1, *ne1_f, *ne2, *ne2_f;
	switch (this->extra[0]) {
		case ASSASSINA_CONFLITO_FAC:
			//printf("O líder da facção %s, %s, é assassinado ao mando de %s, líder da facção rival %s, como retaliação pelo conflito ocorrido entre as duas facções.\n", n2, np2, nl, n1);
			printf("The leader %s of the %s faction is murdered at the behest of %s, leader of the rival faction %s, as a retaliation for the conflict between these two factions.\n", np2, n2, nl, n1);
			break;
		case ASSASSINA_DECLARA_GUERRA:
			//printf("De forma a evitar uma guerra, %s, líder da facção %s, envia um assassino para eliminar o líder da facção rival %s, %s.\n", nl, n1, n2, np2);
			printf("To avoid war, the leader %s from the %s faction sends an assassin to eliminate the leader of their rival faction %s, %s.\n", nl, n1, n2, np2);
			break;
		case ASSASSINA_GUERRA:
			//printf("%s, líder da facção %s, é encontrado morto pelas mãos de um assassino como consequência da guerra que ocorreu entre as facções %s e %s.\n", np2, n2, n1, n2);
			printf("%s, leader of the %s faction , is found murdered by the hands of an assassin as a consequence of the war between the %s and %s factions.\n", np2, n2, n1, n2);
			break;
		case ASSASSINA_TOMA_TERRITORIO:
			//printf("As insatisfações advindas do novo governo que se instalou na facção %s resultou no assassinato de %s, líder da facção rival %s.\n", n1, np2, n2);
			printf("The insatisfactions of the newly established govern in the %s faction resulted in the murder of %s, leader of the %s, their rival faction.\n", n1, np2, n2);
			break;
		case ASSASSINA_NOVO_LIDER:
			//printf("A nova liderança instaurada na facção %s provoca revolta por parte da população, culminando no assassinato do novo líder %s de %s.\n", n1, nl, nf);
			printf("The new leadership established in the %s faction results in a popular revolt, culminating in the murder of their new leader %s from the %s faction.\n", n1, nl, nf);
			break;
		case ASSASSINA_INIMIGOS:
			//printf("Devido a problemas pessoais que vêm ocorrendo entre %s de %s e %s de %s, %s decide assassinar seu rival à sangue frio.\n", np1, n1, np2, n2, np1);
			printf("Due to personal problems between %s from the %s faction and %s from the %s faction, %s decides to murder his rival in cold blood.\n", np1, n1, np2, n2, np1);
			ultimo_assassinato_inimigos = timeline;
			break;
		case ASSASSINA_VINGANCA:
			ne2 = personagens->nome[this->extra[2]];
			ne2_f = personagens->faccoes[(int)personagens->caracteristica[this->extra[2]][IND_FACCAO]].nome;
			if (this->extra[1] == p1) {
				//printf("%s de %s assassina %s de %s para vingar a morte de seu amigo %s de %s.\n", np1, n1, np2, n2, ne2, ne2_f);
				printf("%s from the %s faction murders %s from the %s faction in order to avenge the death os his friend %s from the %s faction.\n", np1, n1, np2, n2, ne2, ne2_f);
			}
			else {
				ne1 = personagens->nome[this->extra[1]];
				ne1_f = personagens->faccoes[(int)personagens->caracteristica[this->extra[1]][IND_FACCAO]].nome;
				//printf("%s de %s contrata um assassino e elimina %s de %s, responsável pela morte de seu falecido amigo, %s de %s.\n", ne1, ne1_f, np2, n2, ne2, ne2_f);
				printf("%s from the %s faction hires an assassin and elimnates %s from the %s faction, responsible for the death of his friend, %s from the %s faction.\n", ne1, ne1_f, np2, n2, ne2, ne2_f);
			}
			break;
		case ASSASSINA_DUELO:
			ne1 = personagens->nome[this->extra[1]];
			ne1_f = personagens->faccoes[(int)personagens->caracteristica[this->extra[1]][IND_FACCAO]].nome;
			//printf("%s de %s assassina %s de %s para vingar da morte de seu amigo %s de %s, morto em um duelo contra %s de %s.\n", np1, n1, np2, n2, ne1, ne1_f, np2, n2);
			printf("%s from the %s faction murders %s from the %s faction to avenge the death of his friend %s from the %s faction, dead in a duel against %s from the %s faction.\n", np1, n1, np2, n2, ne1, ne1_f, np2, n2);
			break;
	}
}
//----------------------------------------------------------------------
void Assassina::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Mata p2
	personagens->caracteristica[p2][IND_SAUDE] = MORTO;
}
//**********************************************************************
//********************** Descobre_Assassino ****************************
//**********************************************************************
///p1 descobre que p2 matou algum de seus amigos
bool Descobre_Assassino::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se houve algum evento Descobre_Assassino recentemente
	if (ultimo_descobre_assassino >= 0 && ultimo_descobre_assassino + DELTA_DESCOBRE_ASSASSINO > timeline)
		return false;
	
	///Verifica se p2 matou algum amigo de p1
	bool encontrado = false;
	for (int i = 0; i < personagens->num_vert; i++) {
		if (evento_ocorrido[p2][i][ASSASSINA].ocorrencias > 0 && personagens->amizades[p1][i] > AMIZADE_NEUTRA) {
			encontrado = true;
			this->extra[0] = i;
			break;
		}
	}
	if (!encontrado)
		return false;
	
	return true;
}
//----------------------------------------------------------------------
float Descobre_Assassino::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return PROB_DESCOBRE_ASSASSINO*personagens->amizades[p1][this->extra[0]];
}
//----------------------------------------------------------------------
void Descobre_Assassino::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, DESCOBRE_ASSASSINO);
	evento_ocorrido[p1][p2][DESCOBRE_ASSASSINO].participante[2] = this->extra[0];
	
	int morto = this->extra[0];
	char* nmorto = personagens->nome[morto];
	char* nmorto_f = personagens->faccoes[(int)personagens->caracteristica[morto][IND_FACCAO]].nome;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	///Se p2 assassinou o antigo lider de p1
	if (personagens->funcao_personagem[morto] == LIDER && personagens->caracteristica[morto][IND_FACCAO] == fac1) {
		//printf("%s de %s descobre que %s de %s foi o responsável pelo assassinato de seu antigo amigo e líder, %s de %s.\n", np1, n1, np2, n2, nmorto, nmorto_f);
		printf("%s from the %s faction finds out that %s from the %s faction was responsible for the murder of his old friend and leader, %s from the %s faction.\n", np1, n1, np2, n2, nmorto, nmorto_f);
	}
	///Se p2 assassinou um amigo de p1
	else {
		//printf("%s de %s descobre que seu falecido amigo, %s de %s, foi morto pelas mãos de %s de %s.\n", np1, n1, nmorto, nmorto_f, np2, n2);
		printf("%s from the %s faction finds out that his deceased friend, %s from the %s faction, was killed by the hands of %s from the %s faction.\n", np1, n1, nmorto, nmorto_f, np2, n2);
	}
		
	ultimo_descobre_assassino = timeline;
}
//----------------------------------------------------------------------
void Descobre_Assassino::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///aumenta ou cria uma inimizade entre p1 e p2
	if (personagens->amizades[p1][p2] < AMIZADE_NEUTRA)
		personagens->amizades[p1][p2] = AMI_DES_ASSASSINO_INIMIGO;
	else
		personagens->amizades[p1][p2] = AMI_DES_ASSASSINO_AMIGO;
	if (personagens->amizades[p2][p1] < AMIZADE_NEUTRA)
		personagens->amizades[p2][p1] = AMI_DES_ASSASSINO_INIMIGO;
	else
		personagens->amizades[p2][p1] = AMI_DES_ASSASSINO_AMIGO;	
	
}
//**********************************************************************
//***************************** Duelo  *********************************
//**********************************************************************
///p1 chama p2 para um duelo e duelam
bool Duelo::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///verifica se p1 e p2 estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
		
	///Verifica se p1 descobriu que p2 assassinou um de seus amigos
	if (evento_ocorrido[p1][p2][DESCOBRE_ASSASSINO].ocorrencias > 0 && personagens->funcao_personagem[p1] == GUERREIRO) {
		this->extra[0] = DUELA_VINGANCA;
		this->extra[1] = evento_ocorrido[p1][p2][DESCOBRE_ASSASSINO].participante[2];
		return true;
	}
	
	///Verifica se p2 se casou com alguem que p1 gostava
	///TODO
	
	///Verifica se p1 é líder e p2 iniciou um conflito civil na facção de p1
	///TODO
	
	///Verifica se p1 e p2 declaram guerra entre suas facções
	///Inicialmente, verifica-se se p1 e p2 são líderes
	bool ok1 = false, ok2 = false;
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	int lider2 = personagens->faccoes[fac2].lider;
	if (lider1 == p1)
		ok1 = true;
	if (lider2 == p2)
		ok2 = true;
	if (ok1 && ok2) {
		///Verifica se p1 e p2 declararm guerra
		if (evento_ocorrido[p1][p2][DECLARA_GUERRA].ocorrencias > 0) {
			///Verifica se a guerra já ocorreu
			if (evento_ocorrido[p1][p2][GUERRA].ocorrencias == 0 || evento_ocorrido[p1][p2][GUERRA].ultima_ocr < evento_ocorrido[p1][p2][DECLARA_GUERRA].ultima_ocr) {
				this->extra[0] = DUELA_DECLARA_GUERRA;
				return true;
			}
		}
	}
	
	///Verifica se p2 matou algum amigo de p1 em batalha
	int a;
	if (evento_ocorrido[lider1][lider2][GUERRA].ocorrencias > 0 && personagens->funcao_personagem[p1] == GUERREIRO) {
		for (int i = 2; i < MAX_PARTICIPANTES; i++) {
			a = evento_ocorrido[lider1][lider2][GUERRA].participante[i];
			if (personagens->amizades[p1][a] > AMIZADE_NEUTRA && evento_ocorrido[p2][a][MORTE_BATALHA].ocorrencias > 0) {
				this->extra[0] = DUELA_MORTE_BATALHA;
				this->extra[1] = a;
				return true;
			}
		}
	}
	
	return false;
}
//----------------------------------------------------------------------
float Duelo::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	float p = 0.0, a;
	int amigo;
	switch (this->extra[0]) {
		case DUELA_VINGANCA:
			amigo = this->extra[1];
			a = personagens->amizades[p1][amigo];
			p = a*personagens->caracteristica[p1][IND_FORCA];
			break;
		case DUELA_DECLARA_GUERRA:
			p = personagens->caracteristica[p1][IND_FORCA] * personagens->caracteristica[p2][IND_FORCA];
			break;
		case DUELA_MORTE_BATALHA:
			amigo = this->extra[1];
			a = personagens->amizades[p1][amigo];
			p = a*personagens->caracteristica[p1][IND_FORCA];
			break;
	}
	
	return p;
}
//----------------------------------------------------------------------
void Duelo::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, DUELO);
	evento_ocorrido[p1][p2][DUELO].participante[2] = this->extra[0];
	evento_ocorrido[p1][p2][DUELO].participante[3] = this->extra[1];
	
	float dif = personagens->caracteristica[p1][IND_FORCA] - personagens->caracteristica[p2][IND_FORCA];
	int vencedor = p2, perdedor = p1;
	///p1 é o vencedor do duelo
	if (randUm(gen2) <= 0.5 + dif) {
		vencedor = p1;
		perdedor = p2;
	}
	personagens->caracteristica[perdedor][IND_SAUDE] = MORTO;
	evento_ocorrido[p1][p2][DUELO].participante[4] = vencedor;
	evento_ocorrido[p1][p2][DUELO].participante[5] = perdedor;
	
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	char *nef, *ne;
	switch (this->extra[0]) {
		case DUELA_VINGANCA:
			ne = personagens->nome[this->extra[1]];
			nef = personagens->faccoes[(int)personagens->caracteristica[this->extra[1]][IND_FACCAO]].nome;
			//printf("Após descobrir que %s de %s assassinou seu amigo %s de %s, %s de %s desafia o assassino para um duelo.\n", np2, n2, ne, nef, np1, n1);
			printf("After discovering that %s from the %s faction killed his friend %s from the %s faction, %s from the %s faction challenges the assassin to a duel.\n", np2, n2, ne, nef, np1, n1);
			break;
		case DUELA_DECLARA_GUERRA:
			//printf("Para popupar a vida de seu exército, %s de %s decide desafiar o líder de sua facção rival, %s de %s, para um duelo.\n", np1, n1, np2, n2);
			printf("To save his army, %s from the %s faction decides to challenge the leader of his rival faction, %s from the %s faction, to a duel.\n", np1, n1, np2, n2);
			break;
		case DUELA_MORTE_BATALHA:
			ne = personagens->nome[this->extra[1]];
			nef = personagens->faccoes[(int)personagens->caracteristica[this->extra[1]][IND_FACCAO]].nome;
			//printf("%s de %s, enfurecido pela perda de seu amigo %s de %s em batalha, desafia o responsável por sua morte, %s de %s, para um duelo.\n", np1, n1, ne, nef, np2, n2);
			printf("%s from the %s faction, infuriated by the loss of his friend %s from the %s faction during a battle, challenges the responsible for his death, %s from the %s faction, to a duel.\n", np1, n1, ne, nef, np2, n2);
			break;
	}
	char* nv = personagens->nome[vencedor];
	char* nfv = personagens->faccoes[(int)personagens->caracteristica[vencedor][IND_FACCAO]].nome;
	char* np = personagens->nome[perdedor];
	char* nfp = personagens->faccoes[(int)personagens->caracteristica[perdedor][IND_FACCAO]].nome;
	//printf("%s de %s sai vitorioso do duelo e %s de %s é morto.\n", nv, nfv, np, nfp);
	printf("%s from the %s faction emerges victorious from the duel and %s from the %s faction is killed.\n", nv, nfv, np, nfp);
}
//----------------------------------------------------------------------
void Duelo::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//**********************************************************************
//*********************** Repressão Popular ****************************
//**********************************************************************
///o líder p1 reprime o lider rebelde de sua facção, p2
bool Repressao_Pop::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Verifica se p1 e p2 estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
	
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider2 = personagens->faccoes[fac2].lider;
	int rebelde2 = personagens->faccoes[fac2].rebelde;
	
	///Verifica se p1 é um rebelde (e se é o rebelde ativo)
	if (personagens->funcao_personagem[p2] != REBELDE || rebelde2 != p2)
		return false;
		
	///Verifica se p2 é o líder da facção de p1
	if (lider2 != p1)
		return false;
	
	///Verifica se não já ocorreu uma repressão recentemente
	if (evento_ocorrido[p1][p2][REPRESSAO_POP].ocorrencias > 0) {
		if (evento_ocorrido[p1][p2][REPRESSAO_POP].ultima_ocr + DELTA_REPRESSAO_POP > timeline)	
			return false;
	}
	
	///Verifica se o nível de bondade do líder p1 é baixo o suficiente
	if (personagens->caracteristica[p1][IND_BONDADE] > MAX_BONDADE_REPRESSAO)
		return false;
		
	return true;
}
//----------------------------------------------------------------------
float Repressao_Pop::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return (1.0-personagens->caracteristica[p1][IND_BONDADE])*PROB_REPRESSAO_POP;
}
//----------------------------------------------------------------------
void Repressao_Pop::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, REPRESSAO_POP);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	if (evento_ocorrido[p1][p2][REPRESSAO_POP].ocorrencias == 0) {
		//printf("Certas medidas extremas impostas pelo líder %s de %s causaram insatisfação do povo. De forma a controlar a situação, %s reprime o líder popular, %s.\n", np1, n1, np1, np2);
		printf("Certain extreme meassures imposed by the leader %s from the %s faction resulted in the people's insatisfaction. To control the situation, %s represses the rebel leader, %s.\n", np1, n1, np1, np2);
	}
	else {
		//printf("Com o aumento dos eventos rebeldes liderados por %s contra o governo de %s de %s, %s ordena que seu exército reprima os movimentos populares.\n", np2, np1, n1, np1);
		printf("With the increase of the rebel events led by %s against the govern of %s from the %s faction, %s command his army to repress the rebel acts.\n", np2, np1, n1, np1);
	}
}
//----------------------------------------------------------------------
void Repressao_Pop::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Faz p1 e p2 ficarem inimigos
	if (personagens->amizades[p1][p2] > AMIZADE_NEUTRA)	personagens->amizades[p1][p2] = -REDUZ_REPRESSAO_POP_LIDER;
	else 	personagens->amizades[p1][p2] -= REDUZ_REPRESSAO_POP_LIDER;
	if (personagens->amizades[p2][p1] > AMIZADE_NEUTRA)	personagens->amizades[p2][p1] = -REDUZ_REPRESSAO_POP_LIDER;
	else 	personagens->amizades[p2][p1] -= REDUZ_REPRESSAO_POP_LIDER;
	
	///Faz todos os amigos de p1 ficarem com mais raiva de p2 e vice-versa
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		if (personagens->amizades[i][p1] > personagens->amizades[i][p2])
			personagens->amizades[i][p2] -= REDUZ_REPRESSAO_POP;
		else
			personagens->amizades[i][p1] -= REDUZ_REPRESSAO_POP;
	}
}
//**********************************************************************
//************************ Conflito_Civil ******************************
//**********************************************************************
///o rebelde p1 se revolta contra seu líder p2
bool Conflito_Civil::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Valor default 
	this->extra[0] = CONFLITO_INIMIGOS;
	
	///Verifica se p1 e p2 estão vivos
	if (personagens->caracteristica[p1][IND_SAUDE] == MORTO || personagens->caracteristica[p2][IND_SAUDE] == MORTO)
		return false;
	
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	int lider1 = personagens->faccoes[fac1].lider;
	int rebelde1 = personagens->faccoes[fac1].rebelde;
	
	///Verifica se p1 é um rebelde (e se é o rebelde ativo)
	if (personagens->funcao_personagem[p1] != REBELDE || rebelde1 != p1)
		return false;
		
	///Verifica se p2 é o líder da facção de p1
	if (lider1 != p2)
		return false;
		
	///Verifica se não já ocorreu um conflito recentemente
	if (evento_ocorrido[p1][p2][CONFLITO_CIVIL].ocorrencias > 0) {
		if (evento_ocorrido[p1][p2][CONFLITO_CIVIL].ultima_ocr + DELTA_CONFLITO_CIVIL > timeline)	
			return false;
	}
		
	///Verifica se p2 é um líder inserido após alguma tomada de território
	if (fac1 != fac2) {
		this->extra[0] = CONFLITO_TOMA;
		return true;
	}
	
	///Verifica se já houve uma repressão antes (p2 reprimiu p1)
	if (evento_ocorrido[p2][p1][REPRESSAO_POP].ocorrencias > 0) {
		this->extra[0] = CONFLITO_REPRESSAO;
		if (evento_ocorrido[p1][p2][CONFLITO_CIVIL].ocorrencias > 0) {
			if (evento_ocorrido[p2][p1][REPRESSAO_POP].ultima_ocr > evento_ocorrido[p1][p2][CONFLITO_CIVIL].ultima_ocr)
				return false;
		}
	}
	
	///Verifica se p2 possui muitos líderes inimigos
	int l;
	this->extra[1] = 0;		///Contabiliza quantos lideres são inimigos de p2
	for (int f = 0; f < personagens->num_faccoes; f++) {
		if (f != fac2) {
			l = personagens->faccoes[f].lider;
			if (personagens->amizades[l][p2] < AMIZADE_NEUTRA)
				this->extra[1]++;
		}
	}	
	if (this->extra[0] == CONFLITO_INIMIGOS && this->extra[1] < MUITOS_INIMIGOS)
		return false;
	
	return true;
}
//----------------------------------------------------------------------
float Conflito_Civil::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	float p;
	switch (this->extra[0]) {
		case CONFLITO_INIMIGOS:
			p = ((float)this->extra[1]/(float)personagens->num_faccoes) * PROB_CONFLITO_CIVIL;
			break;
		case CONFLITO_REPRESSAO:
			p = PROB_CONFLITO_CIVIL;
			break;
		case CONFLITO_TOMA:
			p = PROB_CONFLITO_CIVIL;
			break;
	}
	return p;
}
//----------------------------------------------------------------------
void Conflito_Civil::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	altera_dados_evento(evento_ocorrido, p1, p2, CONFLITO_CIVIL);
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	int fac2 = personagens->caracteristica[p2][IND_FACCAO];
	char* n1 = personagens->faccoes[fac1].nome;
	char* n2 = personagens->faccoes[fac2].nome;
	char* np1 = personagens->nome[p1];
	char* np2 = personagens->nome[p2];
	switch (this->extra[0]) {
		case CONFLITO_INIMIGOS:
			//printf("O número crescente de inimizades da facção %s advindas do governo de seu líder, %s de %s, causam insatisfação, culminando em uma revolta civil liderada por %s.\n", n1, np2, n2, np1);
			printf("The increasing number of rivalries from the %s faction as a result of the leadership of %s from the %s faction, caused insatisfaction, culminating in a civil revolt led by %s.\n", n1, np2, n2, np1);
			break;
		case CONFLITO_REPRESSAO:
			//printf("O povo da facção %s, liderado por %s, se revolta contra seu líder, %s de %s, devido à insatisfações de seu governo repressor.\n", n1, np1, np2, n2);
			printf("The people from the %s faction, led by %s, revolt against their leader, %s from the %s faction, due to his repressive govern.\n", n1, np1, np2, n2);
			break;
		case CONFLITO_TOMA:
			//printf("A dominação territorial e nova liderança imposta pela facção %s sobre a facção %s causou uma grande revolta popular, culminando em um conflito civil liderado por %s.\n", n2, n1, np1);
			printf("The territorial domination and the new leadership imposed by the %s faction over the %s faction caused a greate popular revolt, culminating in a civil conflict led by %s.\n", n2, n1, np1);
			break;
	}
}
//----------------------------------------------------------------------
void Conflito_Civil::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	///Faz p1 e p2 ficarem inimigos
	if (personagens->amizades[p1][p2] > AMIZADE_NEUTRA)	personagens->amizades[p1][p2] = -REDUZ_CONFLITO_CIVIL_LIDER;
	else 	personagens->amizades[p1][p2] -= REDUZ_CONFLITO_CIVIL_LIDER;
	if (personagens->amizades[p2][p1] > AMIZADE_NEUTRA)	personagens->amizades[p2][p1] = -REDUZ_CONFLITO_CIVIL_LIDER;
	else 	personagens->amizades[p2][p1] -= REDUZ_CONFLITO_CIVIL_LIDER;
	
	///Faz todos os amigos de p1 ficarem com mais raiva de p2 e vice-versa
	int fac1 = personagens->caracteristica[p1][IND_FACCAO];
	for (int i = personagens->faccoes[fac1].ini; i <= personagens->faccoes[fac1].fim; i++) {
		if (personagens->amizades[i][p1] > personagens->amizades[i][p2])
			personagens->amizades[i][p2] -= REDUZ_CONFLITO_CIVIL;
		else
			personagens->amizades[i][p1] -= REDUZ_CONFLITO_CIVIL;
	}
}
//**********************************************************************
//************************** Guerra_Civil ******************************
//**********************************************************************
bool Guerra_Civil::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return true;
}
//----------------------------------------------------------------------
float Guerra_Civil::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return 1;
}
//----------------------------------------------------------------------
void Guerra_Civil::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//----------------------------------------------------------------------
void Guerra_Civil::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//**********************************************************************
//************************** Golpe_Civil *******************************
//**********************************************************************
bool Golpe_Civil::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return true;
}
//----------------------------------------------------------------------
float Golpe_Civil::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return 1;
}
//----------------------------------------------------------------------
void Golpe_Civil::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//----------------------------------------------------------------------
void Golpe_Civil::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//**********************************************************************
//*************************** Festival *********************************
//**********************************************************************
bool Festival::pre_condicoes (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return true;
}
//----------------------------------------------------------------------
float Festival::pobabilidade_ocorre (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	return 1;
}
//----------------------------------------------------------------------
void Festival::executa_evento (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
//----------------------------------------------------------------------
void Festival::consequencias (Relacao *personagens, Dados_Evento ***evento_ocorrido, int p1, int p2) {
	
}
