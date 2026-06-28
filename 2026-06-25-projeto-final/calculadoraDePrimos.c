/*
    Compilar: gcc calculadoraDePrimos.c -o calculadoraDePrimos.exe -O2
    Executar: ./calculadoraDePrimos.exe 

    /min <N>      define o inicio do intervalo (padrao: 1)
    /max <N>      define o fim do intervalo (padrao: 10000)
    /threads <M>  define o numero de threads (padrao: nucleos do sistema)
    /affinity     fixa cada thread em um nucleo distinto
    /priority <P> define a prioridade (-2 a 2)
    /time         exibe o tempo de execucao
    /info         exibe informacoes do sistema
    /params       exibe os parametros e encerra
    /quiet        modo silencioso, exibe apenas o total
    /?            exibe esta ajuda
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int id;
    int min_val;
    int max_val;
} ThreadData;

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

DWORD WINAPI PrimeWorker(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    DWORD prime_count = 0;

    for (int i = data->min_val; i <= data->max_val; i++) {
        if (is_prime(i)) {
            prime_count++;
        }
    }

    return prime_count;
}

void PrintSystemInfo() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    
    DWORD_PTR processAffinity, systemAffinity;
    GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity);

    printf("\tInformacoes do Sistema\n");
    printf("Arquitetura do Processador: %u\n", sysinfo.wProcessorArchitecture);
    printf("Numero de Processadores Lógicos: %u\n", sysinfo.dwNumberOfProcessors);
    printf("Mascara de Afinidade do Processo: 0x%llX\n", (unsigned long long)processAffinity);
    printf("Mascara de Afinidade do Sistema: 0x%llX\n", (unsigned long long)systemAffinity);
}

int main(int argc, char* argv[]) {
    int min_val = 1;
    int max_val = 10000;
    int num_threads = 0;
    int priority = 0;
    bool use_affinity = false;
    bool show_time = false;
    bool show_info = false;
    bool show_params = false;
    bool quiet_mode = false;
    bool has_priority = false;

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    num_threads = sysinfo.dwNumberOfProcessors;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "/info") == 0) {
            show_info = true;
        } else if (strcmp(argv[i], "/params") == 0) {
            show_params = true;
        } else if (strcmp(argv[i], "/affinity") == 0) {
            use_affinity = true;
        } else if (strcmp(argv[i], "/time") == 0) {
            show_time = true;
        } else if (strcmp(argv[i], "/quiet") == 0) {
            quiet_mode = true;
        } else if (strcmp(argv[i], "/min") == 0 && i + 1 < argc) {
            min_val = atoi(argv[++i]);
        } else if (strcmp(argv[i], "/max") == 0 && i + 1 < argc) {
            max_val = atoi(argv[++i]);
        } else if (strcmp(argv[i], "/threads") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "/priority") == 0 && i + 1 < argc) {
            priority = atoi(argv[++i]);
            has_priority = true;
        } else {
            printf("Erro: Argumento desconhecido ou mal formatado: %s\n", argv[i]);
            return 1;
        }
    }

    if (min_val > max_val || min_val < 0) {
        printf("Erro: Intervalo invalido.\n");
        return 1;
    }
    if (num_threads <= 0 || num_threads > 64) {
        printf("Erro: Numero de threads invalido. O limite e 64.\n");
        return 1;
    }

    if (show_info) {
        PrintSystemInfo();
        return 0;
    }

    if (show_params) {
        printf("\tParametros de Execucao\n");
        printf("Intervalo: [%d, %d]\n", min_val, max_val);
        printf("Threads: %d\n", num_threads);
        printf("Afinidade ativada: %s\n", use_affinity ? "Sim" : "Nao");
        printf("Prioridade definida: %s (Nivel: %d)\n", has_priority ? "Sim" : "Nao", priority);
        return 0;
    }

    if (!quiet_mode) {
        printf("Iniciando calculo de primos no intervalo [%d, %d] usando %d threads...\n\n", min_val, max_val, num_threads);
    }

    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    HANDLE* hThreads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
    ThreadData* threadData = (ThreadData*)malloc(num_threads * sizeof(ThreadData));
    
    if (hThreads == NULL || threadData == NULL) {
        printf("Erro ao alocar memoria.\n");
        return 1;
    }

    int range = max_val - min_val + 1;
    int chunk_size = range / num_threads;
    int remainder = range % num_threads;
    int current_min = min_val;

    for (int i = 0; i < num_threads; i++) {
        threadData[i].id = i + 1;
        threadData[i].min_val = current_min;
        
        int current_chunk = chunk_size + (i < remainder ? 1 : 0);
        threadData[i].max_val = current_min + current_chunk - 1;
        current_min = threadData[i].max_val + 1;

        hThreads[i] = CreateThread(
            NULL,
            0,
            PrimeWorker,
            &threadData[i],
            CREATE_SUSPENDED,
            NULL
        );

        if (hThreads[i] == NULL) {
            printf("Erro: Falha ao criar a thread %d.\n", i + 1);
            return 1;
        }

        if (use_affinity) {
            DWORD_PTR mask = (DWORD_PTR)1 << (i % sysinfo.dwNumberOfProcessors);
            if (SetThreadAffinityMask(hThreads[i], mask) == 0) {
                printf("Aviso: Falha ao definir afinidade para a thread %d.\n", i + 1);
            }
        }

        if (has_priority) {
            int win_priority = THREAD_PRIORITY_NORMAL;
            switch(priority) {
                case -2: win_priority = THREAD_PRIORITY_LOWEST; break;
                case -1: win_priority = THREAD_PRIORITY_BELOW_NORMAL; break;
                case 1:  win_priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
                case 2:  win_priority = THREAD_PRIORITY_HIGHEST; break;
                default: win_priority = THREAD_PRIORITY_NORMAL; break;
            }
            if (!SetThreadPriority(hThreads[i], win_priority)) {
                printf("Aviso: Falha ao definir prioridade para a thread %d.\n", i + 1);
            }
        }

        ResumeThread(hThreads[i]);
    }

    DWORD waitResult = WaitForMultipleObjects(num_threads, hThreads, TRUE, INFINITE);
    if (waitResult == WAIT_FAILED) {
        printf("Erro: Falha no WaitForMultipleObjects.\n");
        return 1;
    }

    QueryPerformanceCounter(&end_time);

    DWORD total_primes = 0;
    if (!quiet_mode) {
        printf("\tResultados Parciais\n");
    }

    for (int i = 0; i < num_threads; i++) {
        DWORD exitCode;
        if (GetExitCodeThread(hThreads[i], &exitCode)) {
            total_primes += exitCode;
            if (!quiet_mode) {
                printf("Thread %d (Intervalo [%d, %d]): Encontrou %lu primos.\n", 
                    threadData[i].id, threadData[i].min_val, threadData[i].max_val, exitCode);
            }
        } else {
            printf("Erro ao obter o codigo de saida da thread %d.\n", i + 1);
        }
        CloseHandle(hThreads[i]);
    }

    printf("\nTotal de numeros primos encontrados: %lu\n", total_primes);

    if (show_time) {
        double elapsed_time = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
        printf("\nRelatorio de Tempo\n");
        printf("Tempo total de execucao: %.2f ms\n", elapsed_time);
    }

    free(hThreads);
    free(threadData);

    return 0;
}