// compilação:
// gcc -ansi -pedantic -o banqueiro algoritmo-do-banqueiro-interativo.c
// execução:
// ./banqueiro

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_PROCS 10
#define MAX_RES 10

/* Variáveis globais */
int n, m;
int Available[MAX_RES];
int Allocation[MAX_PROCS][MAX_RES];
int Max[MAX_PROCS][MAX_RES];
int Need[MAX_PROCS][MAX_RES];

/* Prototipos das funções */
void computeNeed(void);
void displayState(void);
void pause(void);
bool vectors_le(const int a[], const int b[], int len);
bool safetyAlgorithm(void);
bool resourceRequest(int pid, int Request[]);

/* Funcao para calcular a matriz Need = Max - Allocation */
void computeNeed(void) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            Need[i][j] = Max[i][j] - Allocation[i][j];
        }
    }
}

/* Funcao auxiliar: verifica se vetor a <= vetor b elemento a elemento */
bool vectors_le(const int a[], const int b[], int len) {
    for (int i = 0; i < len; i++) {
        if (a[i] > b[i]) return false;
    }
    return true;
}

/* Funcao para exibir o estado atual em formato tabular */
void displayState(void) {
    printf("\nESTADO ATUAL:");
    printf("\nAvailable: ");
    for (int j = 0; j < m; j++) {
        printf("R%d=%d ", j, Available[j]);
    }
    printf("\n");

    printf("\nAllocation:              Max:                Need:");
    printf("\n      ");
    for (int j = 0; j < m; j++) printf("%3d ", j);
    printf("   ");
    for (int j = 0; j < m; j++) printf("%3d ", j);
    printf("   ");
    for (int j = 0; j < m; j++) printf("%3d ", j);
    printf("\n");

    for (int i = 0; i < n; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < m; j++) printf("%3d ", Allocation[i][j]);
        printf(" P%d: ", i);
        for (int j = 0; j < m; j++) printf("%3d ", Max[i][j]);
        printf(" P%d: ", i);
        for (int j = 0; j < m; j++) printf("%3d ", Need[i][j]);
        printf("\n");
    }
}

/* Funcao Safety Algorithm com passos detalhados */
bool safetyAlgorithm(void) {
    int Work[MAX_RES];
    for (int j = 0; j < m; j++) Work[j] = Available[j];

    bool Finish[MAX_PROCS];
    for (int i = 0; i < n; i++) Finish[i] = false;

    int seq[MAX_PROCS];
    int seq_idx = 0;

    printf("\n--- Executando Safety Algorithm (passo a passo) ---\n");
    int iter = 0;
    while (seq_idx < n) {
        bool found = false;
        printf("Iteração %d - Work: ", ++iter);
        for (int j = 0; j < m; j++) printf("%d ", Work[j]);
        printf("\n");

        for (int p = 0; p < n; p++) {
            if (!Finish[p] && vectors_le(Need[p], Work, m)) {
                printf("  P%d pode executar (Need <= Work).\n", p);
                seq[seq_idx++] = p;
                for (int j = 0; j < m; j++) {
                    Work[j] += Allocation[p][j];
                }
                Finish[p] = true;
                found = true;
                break;
            }
        }
        if (!found) {
            printf("Nenhum processo pode prosseguir.\n");
            break;
        }
    }

    if (seq_idx == n) {
        printf("Sequência segura: ");
        for (int k = 0; k < n; k++) {
            printf("P%d ", seq[k]);
        }
        printf("\n");
        return true;
    } else {
        printf("ESTADO INSEGURO - Não há sequência segura.\n");
        return false;
    }
}

/* Funcao Resource-Request Algorithm completa */
bool resourceRequest(int pid, int Request[]) {
    /* 1. Request <= Need[pid] ? */
    if (!vectors_le(Request, Need[pid], m)) {
        printf("ERRO: Request > Need para P%d\n", pid);
        return false;
    }

    /* 2. Request <= Available ? */
    if (!vectors_le(Request, Available, m)) {
        printf("ERRO: Request > Available\n", pid);
        return false;
    }

    /* 3. Alocação provisória */
    for (int j = 0; j < m; j++) {
        Available[j] -= Request[j];
        Allocation[pid][j] += Request[j];
        Need[pid][j] -= Request[j];
    }

    printf("Alocação provisória OK. Executando Safety...\n");

    /* 4. Safety ? */
    bool is_safe = safetyAlgorithm();

    if (is_safe) {
        printf("Requisição CONCEDIDA e confirmada.\n");
        return true;
    } else {
        /* 5. Rollback */
        for (int j = 0; j < m; j++) {
            Available[j] += Request[j];
            Allocation[pid][j] -= Request[j];
            Need[pid][j] += Request[j];
        }
        printf("Requisição NEGADA (estado inseguro). Rollback realizado.\n");
        return false;
    }
}

/* Funcao para pausar com Enter */
void pause(void) {
    int ch;
    printf("\nPressione Enter para continuar...");
    while ((ch = getchar()) != '\n' && ch != EOF);
}

int main(void) {
    printf("=== ALGORITMO DO BANQUEIRO - VERSÃO INTERATIVA ===\n");
    printf("Baseado em Silberschatz, Cap. 7\n\n");

    /* Opção de dados de exemplo */
    printf("Usar dados de exemplo do Silberschatz (5 proc, 3 rec)? (s/N): ");
    int c = getchar();
    if (c == 's' || c == 'S') {
        /* Preenche dados clássicos */
        n = 5;
        m = 3;
        Available[0] = 3; Available[1] = 3; Available[2] = 2;

        /* Max */
        Max[0][0] = 7; Max[0][1] = 5; Max[0][2] = 3;
        Max[1][0] = 3; Max[1][1] = 2; Max[1][2] = 2;
        Max[2][0] = 9; Max[2][1] = 0; Max[2][2] = 2;
        Max[3][0] = 2; Max[3][1] = 2; Max[3][2] = 2;
        Max[4][0] = 4; Max[4][1] = 3; Max[4][2] = 3;

        /* Allocation */
        Allocation[0][0] = 0; Allocation[0][1] = 1; Allocation[0][2] = 0;
        Allocation[1][0] = 2; Allocation[1][1] = 0; Allocation[1][2] = 0;
        Allocation[2][0] = 3; Allocation[2][1] = 0; Allocation[2][2] = 2;
        Allocation[3][0] = 2; Allocation[3][1] = 1; Allocation[3][2] = 1;
        Allocation[4][0] = 0; Allocation[4][1] = 0; Allocation[4][2] = 2;
    } else {
        /* Entrada manual com validações */
        /* Número de processos */
        do {
            printf("Número de processos (1-%d): ", MAX_PROCS);
            if (scanf("%d", &n) == 1 && n >= 1 && n <= MAX_PROCS) {
                break;
            }
            printf("Entrada inválida! Deve ser inteiro entre 1 e %d.\n", MAX_PROCS);
            while (getchar() != '\n' && getchar() != EOF);
        } while (1);

        /* Número de recursos */
        do {
            printf("Número de tipos de recurso (1-%d): ", MAX_RES);
            if (scanf("%d", &m) == 1 && m >= 1 && m <= MAX_RES) {
                break;
            }
            printf("Entrada inválida! Deve ser inteiro entre 1 e %d.\n", MAX_RES);
            while (getchar() != '\n' && getchar() != EOF);
        } while (1);

        /* Available */
        bool valid;
        do {
            printf("Vetor Available [%d valores >= 0]: ", m);
            valid = true;
            for (int j = 0; j < m; j++) {
                int tmp;
                if (scanf("%d", &tmp) != 1) {
                    printf("Entrada não numérica!\n");
                    valid = false;
                    break;
                }
                if (tmp < 0) {
                    printf("Valor %d inválido (< 0)!\n", tmp);
                    valid = false;
                    break;
                }
                Available[j] = tmp;
            }
            while (getchar() != '\n' && getchar() != EOF);
        } while (!valid);

        /* Max e Allocation por processo */
        for (int i = 0; i < n; i++) {
            /* Max para Pi */
            bool maxok;
            do {
                printf("Max para P%d [%d valores >= 0]: ", i, m);
                maxok = true;
                for (int j = 0; j < m; j++) {
                    int tmp;
                    if (scanf("%d", &tmp) != 1) {
                        printf("Entrada não numérica!\n");
                        maxok = false;
                        break;
                    }
                    if (tmp < 0) {
                        printf("Valor %d inválido (< 0)!\n", tmp);
                        maxok = false;
                        break;
                    }
                    Max[i][j] = tmp;
                }
                while (getchar() != '\n' && getchar() != EOF);
            } while (!maxok);

            /* Allocation para Pi */
            bool allocok;
            do {
                printf("Allocation para P%d [%d valores, 0 <= alloc <= max]: ", i, m);
                allocok = true;
                for (int j = 0; j < m; j++) {
                    int tmp;
                    if (scanf("%d", &tmp) != 1) {
                        printf("Entrada não numérica!\n");
                        allocok = false;
                        break;
                    }
                    if (tmp < 0) {
                        printf("Erro pos. %d: %d < 0!\n", j, tmp);
                        allocok = false;
                        break;
                    }
                    if (tmp > Max[i][j]) {
                        printf("Erro pos. %d: %d > Max=%d!\n", j, tmp, Max[i][j]);
                        allocok = false;
                        break;
                    }
                    Allocation[i][j] = tmp;
                }
                while (getchar() != '\n' && getchar() != EOF);
            } while (!allocok);
        }
    }

    /* Calcula Need e exibe estado inicial */
    computeNeed();
    displayState();
    pause();

    /* Menu interativo */
    int choice;
    while (true) {
        printf("\n=== MENU ===\n");
        printf("1 - Verificar estado seguro (Safety)\n");
        printf("2 - Simular requisição de recursos\n");
        printf("3 - Exibir estado atual novamente\n");
        printf("4 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &choice) != 1) {
            printf("Opção inválida!\n");
            while (getchar() != '\n' && getchar() != EOF);
            pause();
            continue;
        }
        while (getchar() != '\n' && getchar() != EOF);

        if (choice == 1) {
            safetyAlgorithm();
            pause();
        } else if (choice == 2) {
            int pid;
            printf("Qual processo (0 a %d): ", n - 1);
            if (scanf("%d", &pid) != 1 || pid < 0 || pid >= n) {
                printf("PID inválido!\n");
                while (getchar() != '\n' && getchar() != EOF);
                pause();
                continue;
            }
            while (getchar() != '\n' && getchar() != EOF);

            int Request[MAX_RES];
            bool reqok;
            do {
                printf("Vetor Request para P%d [%d valores >= 0]: ", pid, m);
                reqok = true;
                for (int j = 0; j < m; j++) {
                    int tmp;
                    if (scanf("%d", &tmp) != 1) {
                        printf("Entrada não numérica!\n");
                        reqok = false;
                        break;
                    }
                    if (tmp < 0) {
                        printf("Valor %d inválido (< 0)!\n", tmp);
                        reqok = false;
                        break;
                    }
                    Request[j] = tmp;
                }
                while (getchar() != '\n' && getchar() != EOF);
            } while (!reqok);

            resourceRequest(pid, Request);
            pause();
        } else if (choice == 3) {
            displayState();
            pause();
        } else if (choice == 4) {
            printf("Saindo...\n");
            break;
        } else {
            printf("Opção inválida!\n");
            pause();
        }
    }

    return 0;
}
