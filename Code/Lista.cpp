#include "Lista.hpp"

///**********************************************************************
///*************************** LISTA RELACAO ****************************
///**********************************************************************
ListaDE::ListaDE () {
	this->first = NULL;
	num_itens = 0;
}
//----------------------------------------------------------------------
ListaDE::~ListaDE () {
	Item *aux;
	for (Item *p = this->first; p != NULL; ) {
		aux = p;
		p = p->prox;
		free(aux);
	}
}
//----------------------------------------------------------------------
void ListaDE::insere_ordenado (int indice, float valor) {
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	num_itens++;
	
	if (this->first == NULL) {
		this->first = aux;
		this->first->ant = NULL;
		this->first->prox = NULL;
	}
	else if (this->first->f <= valor) {
		aux->prox = this->first;
		this->first->ant = aux;
		aux->ant = NULL;
		this->first = aux;
	}
	else {
		Item* p;
		for (p = this->first; p->prox != NULL; p = p->prox) {
			if (p->f >= valor && p->prox->f < valor) {
				aux->ant = p;
				aux->prox = p->prox;
				p->prox = aux;
				aux->prox->ant = aux;
				return;
			}
		}
		
		if (p->f >= valor) {
			aux->ant = p;
			aux->prox = NULL;
			p->prox = aux;
		}
		else {
			aux->prox = p;
			aux->ant = p->ant;
			p->ant = aux;
			if (aux->ant != NULL) 
				aux->ant->prox = aux;
		}
	}
}
//----------------------------------------------------------------------
void ListaDE::insere_fim (int indice, float valor) {
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	num_itens++;
	
	if (this->first == NULL) {
		this->first = aux;
		aux->prox = NULL;
		aux->ant = NULL;
	}
	else {
		Item* p;
		for (p = this->first; p->prox != NULL; p = p->prox) { }
		
		p->prox = aux;
		aux->prox = NULL;
		aux->ant = p;
	}
}
//----------------------------------------------------------------------
void ListaDE::insere_inicio (int indice, float valor) {
	num_itens++;
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	aux->prox = this->first;
	aux->ant = NULL;
	this->first = aux;
	if (aux->prox != NULL)
		aux->prox->ant = aux;
}
//----------------------------------------------------------------------
Item* ListaDE::remove_topo () {
	if (this->first != NULL) {
		num_itens--;
		Item* aux = this->first;
		this->first = this->first->prox;
		aux->prox = NULL;
		if (this->first != NULL) 
			this->first->ant = NULL;
		
		return aux;
	}
	
	return NULL;
}
//----------------------------------------------------------------------
Item* ListaDE::remove (int indice) {
	Item* aux;
	if (this->first->id == indice) {
		num_itens--;
		aux = this->first;
		this->first = this->first->prox;
		aux->prox = NULL;
		if (this->first != NULL) 
			this->first->ant = NULL;
		return aux;
	}
	Item *p;
	if (this->first == NULL)
		return NULL;
	for (p = this->first; p->prox != NULL && p->prox->id != indice; p = p->prox) {}
	if (p->prox == NULL) 
		return NULL;
	else {
		num_itens--;
		aux = p->prox;
		p->prox = aux->prox;
		aux->ant = NULL;
		aux->prox = NULL;
		if (p->prox != NULL)
			p->prox->ant = p;
	}
	
	return aux;
}
//----------------------------------------------------------------------
void ListaDE::imprime () {
	printf("LISTA ORDENADA CRESCENTE: \n");
	for (Item* p = this->first; p != NULL; p = p->prox) {
		printf("(%d, %.1f)\n", p->id, p->f);
	}
}
///*********************************************************************
///*************************** LISTA EVENTOS ***************************
///*********************************************************************
Lista_Evento::Lista_Evento () {
	this->first = NULL;
	num_itens = 0;
}
//----------------------------------------------------------------------
Lista_Evento::~Lista_Evento () {
	Item_Evento *aux;
	for (Item_Evento *p = this->first; p != NULL; ) {
		aux = p;
		p = p->prox;
		free(aux);
	}
}
//----------------------------------------------------------------------
bool Lista_Evento::atualiza_item (Item_Evento item) {
	Item_Evento *aux;
	for (aux = this->first; aux != NULL; aux = aux->prox) {
		if (aux->id == item.id) {
			for (int i = 0; i < MAX_PARTICIPANTES; i++) {
				if (aux->participante[i] != item.participante[i])
					break;
				///Item encontrado
				else if (i == MAX_PARTICIPANTES-1) {
					aux->p = item.p;
					return true;
				}
			}
		}
	}
	
	return false;
}
//----------------------------------------------------------------------
void Lista_Evento::insere_ordenado (Item_Evento item) {
	if (this->atualiza_item(item))
		return;
		
	Item_Evento* aux = (Item_Evento*) malloc (sizeof(Item_Evento));
	aux->id = item.id;
	aux->p = item.p;
	for (int i = 0; i < MAX_PARTICIPANTES; i++) 
		aux->participante[i] = item.participante[i];
	num_itens++;
		
	if (this->first == NULL) {
		this->first = aux;
		this->first->ant = NULL;
		this->first->prox = NULL;
	}
	else if (this->first->p <= aux->p) {
		aux->prox = this->first;
		this->first->ant = aux;
		aux->ant = NULL;
		this->first = aux;
	}
	else {
		Item_Evento* p;
		for (p = this->first; p->prox != NULL; p = p->prox) {
			if (p->p >= aux->p && p->prox->p < aux->p) {
				aux->ant = p;
				aux->prox = p->prox;
				p->prox = aux;
				aux->prox->ant = aux;
				return;
			}
		}
		
		if (p->p >= item.p) {
			aux->ant = p;
			aux->prox = NULL;
			p->prox = aux;
		}
		else {
			aux->prox = p;
			aux->ant = p->ant;
			p->ant = aux;
			if (aux->ant != NULL) 
				aux->ant->prox = aux;
		}
	}
}
//----------------------------------------------------------------------
Item_Evento* Lista_Evento::remove_topo () {
	if (this->first != NULL) {
		num_itens--;
		Item_Evento* aux = this->first;
		this->first = this->first->prox;
		aux->prox = NULL;
		if (this->first != NULL) 
			this->first->ant = NULL;
		
		return aux;
	}
	
	return NULL;
}
//----------------------------------------------------------------------
void Lista_Evento::imprime () {
	printf("LISTA ORDENADA: \n");
	for (Item_Evento* p = this->first; p != NULL; p = p->prox) {
		printf("Evento %d (%d, %d) - %.3f)\n", p->id, p->participante[0], p->participante[1], p->p);
	}
}
//----------------------------------------------------------------------
















