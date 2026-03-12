#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

void criar_arvore(int nivel_atual, int max_niveis) {
    if (nivel_atual >= max_niveis) {
        return;
    }

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Falha no fork");
            exit(1);
        }

        if (pid == 0) {
            srand(time(NULL) ^ (getpid() << 16));
            int tempo = (rand() % 5) + 1;

            printf("[Nível %d] PID=%d | Pai=%d | dormirei por %ds\n", 
                   nivel_atual + 1, getpid(), getppid(), tempo);

            // cria a proxima geracao com recursividade
            criar_arvore(nivel_atual + 1, max_niveis);
            sleep(tempo);

            printf("[Nível %d] PID=%d encerrando.\n", nivel_atual + 1, getpid());
            exit(0); // garante que o filho não continue no loop
        }
    }

    // pai espera seus dois filhos diretos terminarem
    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }
}

int main() {
    printf("[RAIZ] PID=%d | Iniciando arvore de 3 niveis...\n", getpid());
    criar_arvore(0, 3);

    printf("[RAIZ] Todos os processos descendentes finalizaram.\n");
    return 0;
}
