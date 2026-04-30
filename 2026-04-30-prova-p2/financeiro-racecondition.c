#include <stdio.h>
#include <pthread.h>
long saldo = 0;
void* incrementa_saldo(void* arg) {
    for (int i = 0; i < 1000000; i++) {
        saldo++;
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, incrementa_saldo, NULL);
    pthread_create(&thread2, NULL, incrementa_saldo, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Saldo final: %ld (esperado: 2000000)\n", saldo);
    return 0;
}