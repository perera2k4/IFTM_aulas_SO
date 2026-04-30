#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

long saldo = 0;
sem_t semaforo_binario;

void* incrementa_saldo(void* arg) {
    for (int i = 0; i < 1000000; i++) {
        sem_wait(&semaforo_binario); // troca o semaforo (de 1 para 0)
        
        saldo++;
        
        sem_post(&semaforo_binario); // troca o semaforo (de 0 para 1)
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
     
    sem_init(&semaforo_binario, 0, 1); // inicia o semaforo binario
    
    pthread_create(&thread1, NULL, incrementa_saldo, NULL);
    pthread_create(&thread2, NULL, incrementa_saldo, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    sem_destroy(&semaforo_binario); // destroi o semaforo antes de encerrar

    printf("Saldo final: %ld (esperado: 2000000)\n", saldo);
    return 0;
}