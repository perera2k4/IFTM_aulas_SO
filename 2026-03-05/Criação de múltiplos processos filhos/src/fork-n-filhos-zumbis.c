/* fork_n_filhos_zumbis.c (Linux/Unix) */
/* Versão com tratamento preventivo de processos zumbis */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int parse_n(const char *s, int *out) {
    char *end = NULL;
    long v;

    //printf("[DEBUG] parse_n : %s\n", s);

    if (s == NULL || *s == '\0') return 0;

    v = strtol(s, &end, 10);
    if (*end != '\0') {
        return 0;          /* tem lixo: "10abc" */
    }
    if (v < 0) {
        return 0;          /* negativo não pode */
    }
    if (v > 100000) {
        return 0;          /* limite defensivo p/ lab */
    }
    *out = (int)v;
    return 1;
}

int main(int argc, char *argv[]) {
    int n, i;
    pid_t pid_pai;
    pid_t *pids_filhos;

    if (argc < 2) {
        fprintf(stderr, "\tUso: %s <N>\n", argv[0]);
        fprintf(stderr, "  N = quantidade de processos filhos (inteiro >= 0)\n");
        return 1;
    }

    if (!parse_n(argv[1], &n)) {
        fprintf(stderr, "\tErro: parametro N invalido. Use um inteiro >= 0.\n");
        return 1;
    }

    pid_pai = getpid();

    /* Aloca vetor para guardar os PIDs dos filhos (para imprimir hierarquia) */
    pids_filhos = (pid_t *)calloc((size_t)n, sizeof(pid_t));
    if (pids_filhos == NULL && n > 0) {
        perror("calloc falhou");
        return 1;
    }
    

    /*
      REGRA: só o processo principal cria os N filhos.
      Para garantir isso, o pai faz o loop.
      Cada filho sai imediatamente do loop (retorna do main) após imprimir.
    */
    for (i = 0; i < n; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork falhou");
            /* hierarquia parcial ainda pode ser impressa */
            break;
        }

        if (pid == 0) {
            /* FILHO: não cria mais ninguém */
            printf("[FILHO %d/%d] PID=%ld | PPID=%ld\n", i + 1, n, (long)getpid(), (long)getppid());

            sleep(1); /* trabalho independente */

            printf("[FILHO %d/%d] encerrando. PID=%ld | PPID=%ld\n", i + 1, n, (long)getpid(), (long)getppid());
            return 0;
        }

        /* PAI: guarda PID do filho e continua criando */
        pids_filhos[i] = pid;
    }

    /*
      PAI: imprime uma mensagem mostrando a "hierarquia".
      Aqui é hierarquia em estrela: 1 pai -> N filhos.
    */
    printf("\n=== Hierarquia de processos (criacao em estrela) ===\n");
    printf("PAI  : PID=%ld\n", (long)pid_pai);

    for (i = 0; i < n; i++) {
        if (pids_filhos[i] != 0) {
            printf("  |- FILHO[%d] PID=%ld (pai=%ld)\n", i + 1, (long)pids_filhos[i], (long)pid_pai);
        } else {
            printf("  |- FILHO[%d] NAO_CRIADO\n", i + 1);
        }
    }
    printf("\n\n");

    /*
      TRATAMENTO DE ZUMBIS:
      O pai agora espera pela terminação de todos os filhos com waitpid().
      Isso evita que os filhos se tornem processos zumbis.
    */
    printf("[PAI] Aguardando terminacao de todos os %d filhos...\n", n);
    for (i = 0; i < n; i++) {
        if (pids_filhos[i] > 0) {
            int status;
            pid_t pid_terminado = waitpid(pids_filhos[i], &status, 0);
            
            if (pid_terminado > 0) {
                if (WIFEXITED(status)) {
                    printf("[PAI] Filho PID=%ld terminou com codigo %d\n", 
                           (long)pid_terminado, WEXITSTATUS(status));
                } else {
                    printf("[PAI] Filho PID=%ld terminou anormalmente\n", 
                           (long)pid_terminado);
                }
            } else {
                perror("waitpid falhou");
            }
        }
    }
    printf("[PAI] Todos os filhos foram coletados. Sem processos zumbis!\n\n");

    /*
      Pai segue independente após aguardar todos os filhos.
    */
    sleep(2);
    printf("[PAI] Encerrando. PID=%ld\n", (long)pid_pai);

    free(pids_filhos);
    return 0;
}
