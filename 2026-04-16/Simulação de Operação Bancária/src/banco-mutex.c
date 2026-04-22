#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>  // Para sleep
#include <getopt.h>  // Para getopt

#define MAX_THREADS 50
#define MAX_DELAY 10
#define DEBUG 0       // 0 : ativado    1 : desativado

// Variáveis globais configuráveis via linha de comando
int n_threads = 5;
int max_delay = 5;

pthread_mutex_t mutex; // Mutex para proteger a seção crítica
double saldo = 0; // Variável compartilhada

void* incrementar(void* arg) {
    for (int i = 0; i < 10; i++) {
        // Impressão antes do incremento (modo debug)
        if (DEBUG) printf("\t[DEBUG]Thread %lX: Iter %2d, saldo ANTES:  %7.2f\n", pthread_self(), i, saldo);

        pthread_mutex_lock(&mutex); // Bloqueia o mutex antes de acessar a variável compartilhada
        saldo += 100; // Seção crítica: incremento do saldo
        pthread_mutex_unlock(&mutex); // Desbloqueia o mutex após o acesso

        // Impressão após o incremento (modo debug)
        if (DEBUG) printf("\t[DEBUG]Thread %lX: Iter %2d, saldo DEPOIS: %7.2f\n", pthread_self(), i, saldo);

        // Atraso aleatório de 1-5s para forçar interlaçamento entre threads
        int delay = rand() % max_delay + 1;
        sleep(delay);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    
    // Parse de argumentos com getopt
    int opt;
    while ((opt = getopt(argc, argv, "n:t:")) != -1) {
        switch (opt) {
            case 'n':
                n_threads = atoi(optarg);
                if (! (1 <= n_threads && n_threads <= MAX_THREADS) ) {
                    fprintf(stderr, "Erro: -n deve estar entre 1 e %d.\n", MAX_THREADS);
                    return 1;
                }
                break;
            case 't':
                max_delay = atoi(optarg);
                if ( ! (1 <= max_delay && max_delay <= MAX_DELAY) ) {
                    fprintf(stderr, "Erro: -t deve estar entre 1 e %d.\n",MAX_DELAY);
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

    puts("Estudo de caso: regiões críticas COM sincronismo [mutex]");
    puts("     Descrição: threads simulando operações bancárias");

    pthread_t threads[n_threads];
    
    // Inicializa o mutex
    pthread_mutex_init(&mutex, NULL);
    
    printf("\tCriando %d threads, cada uma executando a função incrementa() [sleep 1 a %d seg.]....\n", n_threads, max_delay);
    for (int i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, incrementar, NULL);
    }
    
    puts("\tAguarda todas as threads terminarem");
    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double esperado = /*iteracoes*/ 10 * /* inc. saldo */ 100 * n_threads;

    printf("\tImprime o saldo final....\n");
    printf("\tSaldo final: R$ %'.2f (esperado: R$ %.2f)\n", saldo, esperado);
    
    // Destroi o mutex
    pthread_mutex_destroy(&mutex);
    
    return 0;
}