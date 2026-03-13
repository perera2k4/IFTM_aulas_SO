/* fork_n_filhos_multiplas_geracoes.c (Linux/Unix) */
/* Versão com tratamento de zumbis e criação de múltiplas gerações */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int parse_n(const char *s, int *out) {
    char *end = NULL;
    long v;

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
    int n, x, i, j;
    pid_t pid_pai;
    pid_t *pids_filhos;

    if (argc < 3) {
        fprintf(stderr, "\tUso: %s <N> <X>\n", argv[0]);
        fprintf(stderr, "  N = quantidade de processos filhos de 1a geracao (inteiro >= 0)\n");
        fprintf(stderr, "  X = quantidade de processos filhos de 2a geracao por filho (inteiro >= 0)\n");
        return 1;
    }

    if (!parse_n(argv[1], &n)) {
        fprintf(stderr, "\tErro: parametro N invalido. Use um inteiro >= 0.\n");
        return 1;
    }

    if (!parse_n(argv[2], &x)) {
        fprintf(stderr, "\tErro: parametro X invalido. Use um inteiro >= 0.\n");
        return 1;
    }

    pid_pai = getpid();

    /* Aloca vetor para guardar os PIDs dos filhos de primeira geração */
    pids_filhos = (pid_t *)calloc((size_t)n, sizeof(pid_t));
    if (pids_filhos == NULL && n > 0) {
        perror("calloc falhou");
        return 1;
    }
    
    printf("[PAI] PID=%ld vai criar %d filhos de 1a geracao\n", (long)pid_pai, n);
    printf("[PAI] Cada filho de 1a geracao vai criar %d filhos de 2a geracao\n\n", x);

    /*
      CRIAÇÃO DOS FILHOS DE PRIMEIRA GERAÇÃO
      Só o processo principal cria os N filhos.
    */
    for (i = 0; i < n; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork falhou");
            break;
        }

        if (pid == 0) {
            /* FILHO DE PRIMEIRA GERAÇÃO */
            pid_t pid_filho1 = getpid();
            pid_t pid_avô = getppid();
            pid_t *pids_netos;
            
            printf("[FILHO-1G %d/%d] PID=%ld | PPID=%ld (Avô=%ld)\n", 
                   i + 1, n, (long)pid_filho1, (long)pid_avô, (long)pid_pai);

            /* Aloca vetor para guardar os PIDs dos netos (filhos de segunda geração) */
            pids_netos = (pid_t *)calloc((size_t)x, sizeof(pid_t));
            if (pids_netos == NULL && x > 0) {
                perror("calloc falhou no filho");
                return 1;
            }

            /* Este filho de primeira geração cria X filhos de segunda geração */
            for (j = 0; j < x; j++) {
                pid_t pid_neto = fork();

                if (pid_neto < 0) {
                    perror("fork falhou no filho");
                    break;
                }

                if (pid_neto == 0) {
                    /* FILHO DE SEGUNDA GERAÇÃO (NETO) */
                    printf("  [FILHO-2G %d/%d do Pai-1G %d] PID=%ld | PPID=%ld\n", 
                           j + 1, x, i + 1, (long)getpid(), (long)getppid());
                    
                    sleep(1); /* trabalho independente */
                    
                    printf("  [FILHO-2G %d/%d do Pai-1G %d] encerrando. PID=%ld\n", 
                           j + 1, x, i + 1, (long)getpid());
                    return 0;
                }

                /* Filho de primeira geração guarda PID do neto */
                pids_netos[j] = pid_neto;
            }

            /* 
               TRATAMENTO DE ZUMBIS - PRIMEIRA GERAÇÃO 
               Cada filho de primeira geração espera por seus filhos (netos do processo original)
            */
            printf("[FILHO-1G %d/%d] Aguardando %d filhos de 2a geracao...\n", i + 1, n, x);
            for (j = 0; j < x; j++) {
                if (pids_netos[j] > 0) {
                    int status;
                    pid_t pid_terminado = waitpid(pids_netos[j], &status, 0);
                    
                    if (pid_terminado > 0) {
                        if (WIFEXITED(status)) {
                            printf("[FILHO-1G %d/%d] Neto PID=%ld terminou com codigo %d\n", 
                                   i + 1, n, (long)pid_terminado, WEXITSTATUS(status));
                        }
                    }
                }
            }
            printf("[FILHO-1G %d/%d] Todos os netos foram coletados. Encerrando. PID=%ld\n", 
                   i + 1, n, (long)pid_filho1);
            
            free(pids_netos);
            return 0;
        }

        /* PAI: guarda PID do filho de primeira geração e continua criando */
        pids_filhos[i] = pid;
    }

    /*
      PAI: imprime uma mensagem mostrando a "hierarquia".
      Hierarquia em árvore: 1 pai -> N filhos -> X netos por filho
    */
    printf("\n=== Hierarquia de processos (arvore multi-geracional) ===\n");
    printf("PAI (Processo Original): PID=%ld\n", (long)pid_pai);
    printf("  Total de filhos (1a geracao): %d\n", n);
    printf("  Total de netos (2a geracao): %d (= %d filhos x %d netos cada)\n", n * x, n, x);

    for (i = 0; i < n; i++) {
        if (pids_filhos[i] != 0) {
            printf("  |- FILHO-1G[%d] PID=%ld\n", i + 1, (long)pids_filhos[i]);
            printf("      |- (criou %d filhos de 2a geracao)\n", x);
        } else {
            printf("  |- FILHO-1G[%d] NAO_CRIADO\n", i + 1);
        }
    }
    printf("\n");

    /*
      TRATAMENTO DE ZUMBIS - PAI PRINCIPAL
      O pai principal espera explicitamente pela terminação de todos os 
      filhos de primeira geração
    */
    printf("[PAI] Aguardando terminacao de todos os %d filhos de 1a geracao...\n", n);
    for (i = 0; i < n; i++) {
        if (pids_filhos[i] > 0) {
            int status;
            pid_t pid_terminado = waitpid(pids_filhos[i], &status, 0);
            
            if (pid_terminado > 0) {
                if (WIFEXITED(status)) {
                    printf("[PAI] Filho-1G PID=%ld terminou com codigo %d\n", 
                           (long)pid_terminado, WEXITSTATUS(status));
                } else {
                    printf("[PAI] Filho-1G PID=%ld terminou anormalmente\n", 
                           (long)pid_terminado);
                }
            } else {
                perror("waitpid falhou");
            }
        }
    }
    printf("[PAI] Todos os filhos foram coletados. Sem processos zumbis!\n\n");

    printf("[PAI] Encerrando. PID=%ld\n", (long)pid_pai);
    free(pids_filhos);
    return 0;
}
