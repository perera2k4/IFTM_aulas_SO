#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_IMPRESSORAS 4
#define N_SETORES 6
#define REQUISICOES_POR_SETOR 8

typedef enum {
	SECRETARIA,
	FINANCEIRO,
	ALMOXARIFADO,
	APOIO,
	PROFESSORES,
	ALUNOS
} TipoUsuario;

typedef struct RequisicaoImpressao {
	int id;
	TipoUsuario tipo;
	int tempoImpressao;
	int prioridadeBase;
	time_t criadoEm;
	struct RequisicaoImpressao *proximo;
} RequisicaoImpressao;

typedef struct {
	RequisicaoImpressao *inicio;
	int tamanho;
} FilaImpressao;

typedef struct {
	TipoUsuario tipo;
	int totalRequisicoes;
	unsigned int seed;
} ParamSetor;

static const char *nome_setor(TipoUsuario tipo) {
	switch (tipo) {
	case SECRETARIA:
		return "Secretaria";
	case FINANCEIRO:
		return "Financeiro";
	case ALMOXARIFADO:
		return "Almoxarifado";
	case APOIO:
		return "Apoio";
	case PROFESSORES:
		return "Professores";
	case ALUNOS:
		return "Alunos";
	default:
		return "Desconhecido";
	}
}

static int prioridade_base(TipoUsuario tipo) {
	switch (tipo) {
	case SECRETARIA:
	case FINANCEIRO:
	case ALMOXARIFADO:
		return 0;
	case APOIO:
	case PROFESSORES:
		return 1;
	case ALUNOS:
	default:
		return 2;
	}
}

static void formatar_tempo(char *buffer, size_t tamanho, time_t momento) {
	struct tm *info = localtime(&momento);
	if (info == NULL) {
		snprintf(buffer, tamanho, "tempo-indisponivel");
		return;
	}
	strftime(buffer, tamanho, "%H:%M:%S", info);
}

static pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fila_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t contagem_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem_impressoras;
static FilaImpressao fila;

static int produtores_finalizados = 0;
static int total_requisicoes = 0;
static int total_concluidas = 0;

static void log_evento(const char *evento, const RequisicaoImpressao *req) {
	char horario[16];
	time_t agora = time(NULL);
	formatar_tempo(horario, sizeof(horario), agora);

	pthread_mutex_lock(&log_mutex);
	printf("[%s] %s \t| Req no. %d \t| %s \t| %ds\n",
		   horario,
		   evento,
		   req->id,
		   nome_setor(req->tipo),
		   req->tempoImpressao);
	fflush(stdout);
	pthread_mutex_unlock(&log_mutex);
}

static void fila_inicializar(FilaImpressao *f) {
	f->inicio = NULL;
	f->tamanho = 0;
}

static void fila_inserir(FilaImpressao *f, RequisicaoImpressao *req) {
	req->proximo = NULL;
	if (f->inicio == NULL) {
		f->inicio = req;
	} else {
		RequisicaoImpressao *atual = f->inicio;
		while (atual->proximo != NULL) {
			atual = atual->proximo;
		}
		atual->proximo = req;
	}
	f->tamanho++;
}

static int prioridade_com_envelhecimento(const RequisicaoImpressao *req, time_t agora) {
	int prioridade = req->prioridadeBase;
	int espera = (int)difftime(agora, req->criadoEm);

	if (espera >= 8) {
		prioridade -= 2;
	} else if (espera >= 4) {
		prioridade -= 1;
	}

	if (prioridade < 0) {
		prioridade = 0;
	}
	return prioridade;
}

static RequisicaoImpressao *fila_remover_melhor(FilaImpressao *f) {
	if (f->inicio == NULL) {
		return NULL;
	}

	time_t agora = time(NULL);
	RequisicaoImpressao *melhor = f->inicio;
	RequisicaoImpressao *melhor_anterior = NULL;

	RequisicaoImpressao *anterior = NULL;
	RequisicaoImpressao *atual = f->inicio;

	int prioridade_melhor = prioridade_com_envelhecimento(melhor, agora);
	time_t criado_melhor = melhor->criadoEm;

	while (atual != NULL) {
		int prioridade_atual = prioridade_com_envelhecimento(atual, agora);
		if (prioridade_atual < prioridade_melhor ||
			(prioridade_atual == prioridade_melhor && atual->criadoEm < criado_melhor)) {
			melhor = atual;
			melhor_anterior = anterior;
			prioridade_melhor = prioridade_atual;
			criado_melhor = atual->criadoEm;
		}
		anterior = atual;
		atual = atual->proximo;
	}

	if (melhor_anterior == NULL) {
		f->inicio = melhor->proximo;
	} else {
		melhor_anterior->proximo = melhor->proximo;
	}
	melhor->proximo = NULL;
	f->tamanho--;
	return melhor;
}

static int gerar_tempo_impressao(TipoUsuario tipo, unsigned int *seed) {
	int minimo = 1;
	int maximo = 2;

	if (tipo == APOIO || tipo == PROFESSORES) {
		minimo = 2;
		maximo = 4;
	} else if (tipo == ALUNOS) {
		minimo = 4;
		maximo = 6;
	}

	return minimo + (int)(rand_r(seed) % (unsigned int)(maximo - minimo + 1));
}

static int gerar_intervalo(TipoUsuario tipo, unsigned int *seed) {
	int minimo = 1;
	int maximo = 3;

	if (tipo == APOIO || tipo == PROFESSORES) {
		minimo = 2;
		maximo = 4;
	} else if (tipo == ALUNOS) {
		minimo = 3;
		maximo = 6;
	}

	return minimo + (int)(rand_r(seed) % (unsigned int)(maximo - minimo + 1));
}

static void *thread_setor(void *arg) {
	ParamSetor *param = (ParamSetor *)arg;
	int i;

	for (i = 0; i < param->totalRequisicoes; i++) {
		RequisicaoImpressao *req = (RequisicaoImpressao *)malloc(sizeof(RequisicaoImpressao));
		if (req == NULL) {
			continue;
		}

		req->tipo = param->tipo;
		req->prioridadeBase = prioridade_base(param->tipo);
		req->tempoImpressao = gerar_tempo_impressao(param->tipo, &param->seed);
		req->criadoEm = time(NULL);
		req->proximo = NULL;

		pthread_mutex_lock(&contagem_mutex);
		total_requisicoes++;
		req->id = total_requisicoes;
		pthread_mutex_unlock(&contagem_mutex);

		pthread_mutex_lock(&fila_mutex);
		fila_inserir(&fila, req);
		pthread_cond_signal(&fila_cond);
		pthread_mutex_unlock(&fila_mutex);

		log_evento("Requisicao criada", req);
		sleep(gerar_intervalo(param->tipo, &param->seed));
	}

	pthread_mutex_lock(&fila_mutex);
	produtores_finalizados++;
	pthread_cond_broadcast(&fila_cond);
	pthread_mutex_unlock(&fila_mutex);

	return NULL;
}

static void *thread_gerente(void *arg) {
	(void)arg;

	while (1) {
		RequisicaoImpressao *req = NULL;

		pthread_mutex_lock(&fila_mutex);
		while (fila.tamanho == 0 && produtores_finalizados < N_SETORES) {
			pthread_cond_wait(&fila_cond, &fila_mutex);
		}

		if (fila.tamanho == 0 && produtores_finalizados == N_SETORES) {
			pthread_mutex_unlock(&fila_mutex);
			break;
		}

		req = fila_remover_melhor(&fila);
		pthread_mutex_unlock(&fila_mutex);

		if (req == NULL) {
			continue;
		}

		sem_wait(&sem_impressoras);
		log_evento("Inicio impressao", req);
		sleep(req->tempoImpressao);
		log_evento("Fim impressao", req);
		sem_post(&sem_impressoras);

		pthread_mutex_lock(&contagem_mutex);
		total_concluidas++;
		pthread_mutex_unlock(&contagem_mutex);

		free(req);
	}

	return NULL;
}

int main(void) {
	pthread_t setores[N_SETORES];
	pthread_t gerente;
	ParamSetor parametros[N_SETORES];
	int i;

	fila_inicializar(&fila);
	sem_init(&sem_impressoras, 0, N_IMPRESSORAS);

	parametros[0].tipo = SECRETARIA;
	parametros[1].tipo = FINANCEIRO;
	parametros[2].tipo = ALMOXARIFADO;
	parametros[3].tipo = APOIO;
	parametros[4].tipo = PROFESSORES;
	parametros[5].tipo = ALUNOS;

	for (i = 0; i < N_SETORES; i++) {
		parametros[i].totalRequisicoes = REQUISICOES_POR_SETOR;
		parametros[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 7919);
		pthread_create(&setores[i], NULL, thread_setor, &parametros[i]);
	}

	pthread_create(&gerente, NULL, thread_gerente, NULL);

	for (i = 0; i < N_SETORES; i++) {
		pthread_join(setores[i], NULL);
	}

	pthread_join(gerente, NULL);
	sem_destroy(&sem_impressoras);

	printf("\nTotal requisicoes: %d\n", total_requisicoes);
	printf("Total concluidas: %d\n", total_concluidas);

	return 0;
}
