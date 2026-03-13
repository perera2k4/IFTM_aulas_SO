#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    if (fork() == 0) {

    }
    else {
        sleep(60); 
    }

    return 0;
}