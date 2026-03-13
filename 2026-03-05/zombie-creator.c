#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // Para fork(), getpid(), _exit(), sleep()
#include <sys/types.h> // Para pid_t

int main(void) {
        
    pid_t pid = fork(); // Cria um novo processo

    if (pid < 0) {
        // Erro no fork
        perror("Erro ao criar processo filho (fork)\n");
        return 1;
    }

    if (pid == 0) {
        // Código do PROCESSO FILHO
        printf("Filho: Meu PID e %d. Meu pai e %d.\n", (int)getpid(), (int)getppid());
        printf("Filho: Vou terminar agora.\n");
        _exit(0); // O filho termina imediatamente
    } else {
        // Código do PROCESSO PAI
        printf("Pai: Meu PID e %d. Criei o filho com PID %d.\n", (int)getpid(), (int)pid);
        printf("Pai: Nao vou chamar wait() para coletar o status do filho.\n");
        printf("Pai: Vou dormir por 30 segundos para voce observar o zumbi.\n");
        sleep(2 * 60); // O pai dorme, permitindo que o filho se torne zumbi
        printf("Pai: Acordei e vou terminar. O zumbi sera limpo pelo init.\n");
        return 0;
    }
}