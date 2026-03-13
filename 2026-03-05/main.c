/* fork_indep.c (Linux) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid_filho;
    pid_t pid_pai;
    pid_pai = getpid();
    pid_filho = fork();

    if (pid_filho < 0) {
        perror("fork falhou");
        return 1;

    }

    if (pid_filho == 0) {
        /* Processo filho */
        pid_t meu_pid = getpid();
        pid_t meu_ppid = getppid();
        printf("[FILHO] meu PID=%ld | meu PPID=%ld | PID do pai (capturado antes do fork)=%ld\n", (long)meu_pid, (long)meu_ppid, (long)pid_pai);

        /* Simula trabalho independente */
        sleep(2);
        printf("[FILHO] ainda executando (independente do pai). meu PID=%ld | PPID atual=%ld\n", (long)getpid(), (long)getppid());
        return 0;
    }

    /* Processo pai */
    printf("[PAI ] meu PID=%ld | PID do filho=%ld\n", (long)pid_pai, (long)pid_filho);

    /* Pai N�O espera o filho -> independentes */
    sleep(1);
    printf("[PAI ] seguindo independente. meu PID=%ld | filho=%ld\n", (long)getpid(), (long)pid_filho);
    return 0;
}
