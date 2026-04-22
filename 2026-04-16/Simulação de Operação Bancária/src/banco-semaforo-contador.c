#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>  // Para getopt

#define MAX_THREADS 50
#define MAX_DELAY 10
#define DEBUG 0       // 0: ativado    1: desativado

// Variáveis globais configuráveis via linha de comando
int n_threads = 5;
int max_delay = 5;
double saldo = 0;

// Semáforo contador (inicializado com 3): permite até 3 threads simultâneas na RC
// (Silberschatz cap. 6: controle de acessos concorrentes limitados, ex: pools de recursos)
sem_t semaforo;

// Função executada por cada thread: incrementa o saldo 10 vezes em 100
void* incrementa(void* arg) {
    for(int i = 0; i < 10; i++) {

        sem_wait(&semaforo);  // P(semaforo): decrementa; bloqueia se ==0

        // Impressão antes do incremento (modo debug)
        if (DEBUG) printf("\t[DEBUG]Thread %lX: Iter %2d, saldo ANTES:  %7.2f\n", pthread_self(), i, saldo);

        // Operação agora segura (até 3 threads simultâneas)
        saldo += 100;

        // Impressão após o incremento (modo debug)
        if (DEBUG) printf("\t[DEBUG]Thread %lX: Iter %2d, saldo DEPOIS: %7.2f\n", pthread_self(), i, saldo);

        sem_post(&semaforo);  // V(semaforo): incrementa (libera para próxima)

        // Atraso aleatório de 1 a max_delay s (fora da região crítica)
        int delay = rand() % max_delay + 1;
        sleep(delay);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse de argumentos com getopt (mantido idêntico)
    int opt;
    while ((opt = getopt(argc, argv, "n:t:")) != -1) {
        switch (opt) {
            case 'n':
                n_threads = atoi(optarg);
                if (!(1 <= n_threads && n_threads <= MAX_THREADS)) {
                    fprintf(stderr, "Erro: -n deve estar entre 1 e %d.\n", MAX_THREADS);
                    return 1;
                }
                break;
            case 't':
                max_delay = atoi(optarg);
                if (!(1 <= max_delay && max_delay <= MAX_DELAY)) {
                    fprintf(stderr, "Erro: -t deve estar entre 1 e %d.\n", MAX_DELAY);
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Uso: %s [-n <threads>] [-t <max_delay>]\n", argv[0]);
                fprintf(stderr, "  -n: threads (1-%d, default 5)\n", MAX_THREADS);
                fprintf(stderr, "  -t: max sleep (1-%d, default 5)\n", MAX_DELAY);
                return 1;
        }
    }

    srand(time(NULL));  // Seed para rand() aleatório

    // Inicializa semáforo contador (0: intra-processo; 3: capacidade máxima simultânea)
    sem_init(&semaforo, 0, 3);

    puts("Estudo de caso: regiões críticas COM sincronismo (semáforo contador)");
    puts("\tDescrição: threads simulando operações bancárias");
    printf("\tUsando %d threads, max_delay=%d, capacidade semáforo=3\n", n_threads, max_delay);

    // Array para armazenar os identificadores das threads
    pthread_t threads[n_threads];
    
    printf("\tCriando %d threads, cada uma executando a função incrementa() [semáforo contador]....\n", n_threads);
    for(int i = 0; i < n_threads; i++) {
        if(pthread_create(&threads[i], NULL, incrementa, NULL) != 0) {
            perror("Erro ao criar thread");
            exit(1);
        }
    }
    
    puts("\tAguarda todas as threads terminarem...");
    for(int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double esperado = 10 * 100 * n_threads;

    printf("\tImprime o saldo final....\n");
    printf("\tSaldo final: R$ %'.2f (esperado: R$ %.2f)\n", saldo, esperado);
    
    sem_destroy(&semaforo);  // Limpa semáforo
    
    return 0;
}