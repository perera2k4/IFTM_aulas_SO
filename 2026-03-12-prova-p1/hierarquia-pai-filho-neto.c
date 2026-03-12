#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid_filho1, pid_neto1, pid_neto2;

    printf("[PAI] PID=%d | Iniciando programa...\n", getpid());
    pid_filho1 = fork();

    if (pid_filho1 < 0) {
        perror("Erro no fork do Filho 1");
        exit(1);
    }

    if (pid_filho1 == 0) {
        printf("[FILHO #1] PID=%d | Pai(PPID)=%d\n", getpid(), getppid());

        // filho, neto 1
        pid_neto1 = fork();

        if (pid_neto1 == 0) {
            printf("  [NETO #1] PID=%d | Pai(PPID)=%d\n", getpid(), getppid());
            sleep(1);
            exit(0);
        }

        // filho, neto 2
        pid_neto2 = fork();

        if (pid_neto2 == 0) {
            printf("  [NETO #2] PID=%d | Pai(PPID)=%d\n", getpid(), getppid());
            sleep(1);
            exit(0);
        }

        // pai espera seus filhos e netos terminarem
        wait(NULL);
        wait(NULL);
        printf("[FILHO #1] Meus filhos terminaram. Encerrando...\n");
        exit(0);
    } else {
        // pai espera o processo filho terminar
        wait(NULL);
        printf("[PAI] Filho #1 encerrou. Programa finalizado.\n");
    }

    return 0;
}
