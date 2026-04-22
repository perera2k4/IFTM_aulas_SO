#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <getopt.h>

#define MAX_THREADS 50

int n_threads = 5, max_delay = 3;
double vendas_totais = 0;
pthread_t threads[MAX_THREADS];

sem_t semaforo;

void* cliente(void* arg) {
    for(int i = 0; i < 20; i++) {
        
        // semaforo P ou sem_wait decrementa o contador 1 para 0
        // se ja for 0, a thread dorme até que ele se torne > 0
        sem_wait(&semaforo); 

        vendas_totais += 50; // regiao critica

        // semaforo V ou sem_post incrementa o contador 0 para 1
        // acorda uma thread que esteja esperando
        sem_post(&semaforo);

        if (max_delay > 0) sleep(rand() % max_delay + 1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "n:t:")) != -1) {
        switch (opt) {
            case 'n': n_threads = atoi(optarg); break;
            case 't': max_delay = atoi(optarg); break;
        }
    }

    // inicializa o semáforo:
    // 2 param 0 compartilhado apenas entre threads deste processo
    // 3 param 1 valor inicial 1 = semáforo binario/livre
    sem_init(&semaforo, 0, 1);

    printf("iniciando vendas com %d clientes (semaforo binario)...\n", n_threads);

    for(int i = 0; i < n_threads; i++) 
        pthread_create(&threads[i], NULL, cliente, NULL);

    for(int i = 0; i < n_threads; i++) 
        pthread_join(threads[i], NULL);

    // destruicao do semaforo
    sem_destroy(&semaforo);

    double esperado = 20.0 * 50 * n_threads;
    printf("Vendas totais: R$ %.2f (Esperado: R$ %.2f)\n", vendas_totais, esperado);

    return 0;
}