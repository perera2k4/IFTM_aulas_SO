#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 50
#define INSERCOES_POR_THREAD 10

struct Node {
    int data;
    int seq_id;
    struct Node *next;
};

int n_threads = 4;
int max_delay = 3;
struct Node *head = NULL;
pthread_t threads[MAX_THREADS];
int thread_ids[MAX_THREADS];

sem_t sem_binario;

static int validar_parametros(void) {
    if (n_threads < 1 || n_threads > MAX_THREADS) {
        fprintf(stderr, "Erro: -n deve estar entre 1 e %d.\n", MAX_THREADS);
        return 0;
    }

    if (max_delay < 1 || max_delay > 10) {
        fprintf(stderr, "Erro: -t deve estar entre 1 e 10.\n");
        return 0;
    }

    return 1;
}

static void liberar_lista(void) {
    struct Node *atual = head;

    while (atual != NULL) {
        struct Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }

    head = NULL;
}

void *inseridor(void *arg) {
    int thread_id = *(int *)arg;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(thread_id * 1103515245u);

    for (int i = 0; i < INSERCOES_POR_THREAD; i++) {
        struct Node *new_node = malloc(sizeof(struct Node));
        if (new_node == NULL) {
            fprintf(stderr, "[T%d] Erro: malloc falhou (errno=%d).\n", thread_id, errno);
            continue;
        }

        new_node->data = (int)(rand_r(&seed) % 100);
        new_node->seq_id = (thread_id * INSERCOES_POR_THREAD) + i;

        printf("[T%d] antes da RC | insercao=%d seq=%d\n", thread_id, i + 1, new_node->seq_id);
        sem_wait(&sem_binario);

        new_node->next = head; /* RC 1 */
        head = new_node;       /* RC 2 */

        sem_post(&sem_binario);
        printf("[T%d] depois da RC | insercao=%d seq=%d\n", thread_id, i + 1, new_node->seq_id);

        sleep((int)(rand_r(&seed) % max_delay) + 1);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "n:t:")) != -1) {
        switch (opt) {
        case 'n':
            n_threads = atoi(optarg);
            break;
        case 't':
            max_delay = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Uso: %s [-n threads] [-t atraso]\n", argv[0]);
            return 1;
        }
    }

    if (!validar_parametros()) {
        return 1;
    }

    if (sem_init(&sem_binario, 0, 1) != 0) {
        perror("Erro em sem_init");
        return 1;
    }

    srand((unsigned int)time(NULL));

    printf("Criando %d fornecedores...\n", n_threads);

    for (int i = 0; i < n_threads; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, inseridor, &thread_ids[i]) != 0) {
            perror("Erro em pthread_create");
            sem_destroy(&sem_binario);
            liberar_lista();
            return 1;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Erro em pthread_join");
            sem_destroy(&sem_binario);
            liberar_lista();
            return 1;
        }
    }

    int esperado = INSERCOES_POR_THREAD * n_threads;
    int count = 0;
    int duplicatas = 0;
    int invalidos = 0;
    int vistos_ok = 0;

    int *vistos = calloc((size_t)esperado, sizeof(int));
    if (vistos == NULL) {
        fprintf(stderr, "Erro: calloc falhou para vetor de validacao.\n");
        sem_destroy(&sem_binario);
        liberar_lista();
        return 1;
    }

    struct Node *temp = head;
    while (temp != NULL) {
        count++;

        if (temp->seq_id < 0 || temp->seq_id >= esperado) {
            invalidos++;
        } else {
            if (vistos[temp->seq_id] == 1) {
                duplicatas++;
            } else {
                vistos[temp->seq_id] = 1;
                vistos_ok++;
            }
        }

        temp = temp->next;
    }

    int perdas = esperado - vistos_ok;

    printf("Nos na lista: %d (esperado: %d)\n", count, esperado);
    printf("Validacao de integridade -> perdas: %d | duplicatas(seq): %d | seq_invalidos: %d\n",
           perdas,
           duplicatas,
           invalidos);

    free(vistos);
    sem_destroy(&sem_binario);
    liberar_lista();

    return 0;
}
