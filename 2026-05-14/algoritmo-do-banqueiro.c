// compilação:
// gcc -ansi -pedantic -o banqueiro algoritmo-do-banqueiro.c
// execução:
// ./banqueiro

#include <stdio.h>

#define N 5
#define M 3

int Available[M];
int Max[N][M];
int Allocation[N][M];
int Need[N][M];

/* Calcula Need[i][j] = Max[i][j] - Allocation[i][j] */
void computeNeed(void)
{
    int i, j;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            Need[i][j] = Max[i][j] - Allocation[i][j];
}

/* Imprime o estado atual formatado */
void printState(void)
{
    int i, j;
    printf("\nESTADO ATUAL:\n");
    printf("Recursos Disponíveis (Available): A=%2d  B=%2d  C=%2d\n", Available[0], Available[1], Available[2]);
    printf("\nALOCAÇÃO (Allocation):\n");
    printf("     A   B   C\n");
    for (i = 0; i < N; i++) {
        printf("P%d: ", i);
        for (j = 0; j < M; j++) {
            printf("%3d", Allocation[i][j]);
        }
        printf("\n");
    }
    printf("\nMÁXIMO (Max):\n");
    printf("     A   B   C\n");
    for (i = 0; i < N; i++) {
        printf("P%d: ", i);
        for (j = 0; j < M; j++) {
            printf("%3d", Max[i][j]);
        }
        printf("\n");
    }
    printf("\nNECESSIDADE (Need):\n");
    printf("     A   B   C\n");
    for (i = 0; i < N; i++) {
        printf("P%d: ", i);
        for (j = 0; j < M; j++) {
            printf("%3d", Need[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Safety Algorithm: verifica se existe sequência segura */
int isSafeState(void)
{
    int Work[M];
    int Finish[N];
    int sequence[N];
    int seq_idx = 0;
    int found;
    int p, j, i;

    /* Work = Available */
    for (j = 0; j < M; j++) Work[j] = Available[j];
    /* Finish[i] = false para todos i */
    for (i = 0; i < N; i++) Finish[i] = 0;

    do {
        found = 0;
        for (p = 0; p < N; p++) {
            if (Finish[p] == 0) {
                /* Verifica se Need[p] <= Work */
                int can = 1;
                for (j = 0; j < M; j++) {
                    if (Need[p][j] > Work[j]) {
                        can = 0;
                        break;
                    }
                }
                if (can) {
                    /* Work = Work + Allocation[p] */
                    for (j = 0; j < M; j++) {
                        Work[j] += Allocation[p][j];
                    }
                    Finish[p] = 1;
                    sequence[seq_idx++] = p;
                    found = 1;
                    break; /* Encontra um por iteração */
                }
            }
        }
    } while (found);

    /* Se todos Finish[i] == true, estado seguro */
    int safe = 1;
    for (i = 0; i < N; i++) {
        if (Finish[i] == 0) {
            safe = 0;
            break;
        }
    }

    if (safe) {
        printf("Estado SEGURO.\n");
        printf("Sequência segura: ");
        for (i = 0; i < seq_idx; i++) {
            printf("P%d ", sequence[i]);
        }
        printf("\n");
    } else {
        printf("Estado INSEGURO.\n");
    }
    return safe;
}

/* Resource-Request Algorithm: simula requisição */
void requestResources(int pid, int request[])
{
    int old_Available[M];
    int old_Allocation[N][M];
    int old_Need[N][M];
    int i, j;

    printf("Processo P%d solicita: A=%d B=%d C=%d\n", pid, request[0], request[1], request[2]);

    /* 1. Request <= Need[pid]? */
    for (j = 0; j < M; j++) {
        if (request[j] > Need[pid][j]) {
            printf("ERRO: Requisição excede necessidade máxima (Need).\n");
            return;
        }
    }

    /* 2. Request <= Available? */
    for (j = 0; j < M; j++) {
        if (request[j] > Available[j]) {
            printf("INSUFICIENTE: Requisição > Disponíveis. Aguarde.\n");
            return;
        }
    }

    /* Salva estado anterior */
    for (j = 0; j < M; j++) old_Available[j] = Available[j];
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++) {
            old_Allocation[i][j] = Allocation[i][j];
            old_Need[i][j] = Need[i][j];
        }

    /* 3. Aloca temporariamente */
    for (j = 0; j < M; j++) {
        Available[j] -= request[j];
        Allocation[pid][j] += request[j];
        Need[pid][j] -= request[j];
    }

    printf("\nEstado após alocação PROVISÓRIA:\n");
    printState();

    /* 4. Safety Algorithm no novo estado */
    if (isSafeState()) {
        printf("*** REQUISIÇÃO CONCEDIDA! Novo estado SEGURO. ***\n\n");
    } else {
        printf("*** REQUISIÇÃO NEGADA! Estado seria INSEGURO. ***\n");
        printf("Estado ANTERIOR restaurado.\n\n");
        /* Rollback */
        for (j = 0; j < M; j++) Available[j] = old_Available[j];
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++) {
                Allocation[i][j] = old_Allocation[i][j];
                Need[i][j] = old_Need[i][j];
            }
    }
}

int main(void)
{
    int i, j;
    int av[M] = {3, 3, 2};
    int maxm[N][M] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    int allo[N][M] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    /* Inicializa com dados do exemplo Silberschatz */
    for (j = 0; j < M; j++) Available[j] = av[j];
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++) {
            Max[i][j] = maxm[i][j];
            Allocation[i][j] = allo[i][j];
        }
    computeNeed();

    printf("=== ALGORITMO DO BANQUEIRO - Silberschatz Cap. 7 ===\n\n");
    printState();

    printf("--- Safety Algorithm (Estado Inicial) ---\n");
    isSafeState();

    printf("\n--- Resource-Request Algorithm (Exemplo P1 req (1,0,2)) ---\n");
    int req[M] = {1, 0, 2};
    requestResources(1, req);

    printf("Demonstração finalizada.\n");
    return 0;
}
