#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // Para fork(), getpid(), _exit(), sleep()
#include <sys/types.h> // Para pid_t
#include <sys/wait.h>  // Para waitpid(), WIFEXITED(), WEXITSTATUS()

int main(void) {

    pid_t pid = fork(); // Cria um novo processo

    if (pid < 0) {
        perror("Erro ao criar processo filho (fork)");
        return 1;
    }

    if (pid == 0) {
        // Código do PROCESSO FILHO
        printf("Filho: Meu PID e %d. Meu pai e %d.\n", (int)getpid(), (int)getppid());
        printf("Filho: Vou terminar com codigo 42.\n");
        sleep(3);
        _exit(42); // O filho termina com um código de saída específico
    } else {
        // Código do PROCESSO PAI
        int status; // Variavel para armazenar o status de termino do filho
        printf("Pai: Meu PID e %d. Criei o filho com PID %d.\n", (int)getpid(), (int)pid);
        printf("Pai: Vou chamar waitpid() para coletar o status do filho.\n");

        // waitpid() bloqueia o pai ate o filho terminar e coleta seu status
        if (waitpid(pid, &status, 0) == -1) {
            perror("Erro em waitpid()");
            return 1;
        }

        // Verifica se o filho terminou normalmente e exibe seu codigo de saida
        if (WIFEXITED(status)) {
            printf("Pai: Filho com PID %d terminou normalmente com codigo %d.\n", (int)pid, WEXITSTATUS(status));
        } else {
            printf("Pai: Filho com PID %d nao terminou normalmente.\n", (int)pid);
        }

        printf("Pai: Status do filho coletado. Nao havera zumbi.\n");
        return 0;
    }
}