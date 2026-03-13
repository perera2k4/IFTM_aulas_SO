#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main() {
	pid_t pid;
	
	/* cria um processo-filho */
	pid = fork();
	if (pid < 0) { /* um erro ocorreu */
		// fprintf(stderr, "Fork failed");
		return 0;
	}
	else if (pid == 0) { /* processo-filho */
    	printf("Iniciando o processo filho...\n");
		execlp("/bin/ls", "ls", NULL);
    	printf("Teste de execução...\n");
	}
	else { /* processo-pai */
    	/* o pai esperarÃ¡ que o filho seja concluÃ­do */
    	printf("Iniciando o processo pai...\n");
    	wait(NULL);
    	printf("Child complete");
	}
	
	printf("Codigo-fonte comum finalizando...\n");
	return 0;
}
