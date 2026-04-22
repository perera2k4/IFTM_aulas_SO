o programa possui regiao critica na atualizacao da variavel compartilhada vendas_totais
sem controle de concorrencia, varias threads podem acessar esse trecho ao mesmo tempo e causar condicao de corrida
isso gera perda de atualizacao e valor final inconsistente nas vendas
a solucao escolhida foi semaforo binario com valor inicial 1
ele permite apenas uma thread por vez na secao critica, garantindo exclusao mutua
no codigo, sem_wait entra na regiao critica e sem_post libera o acesso para a proxima thread
essa abordagem e simples, eficiente e adequada quando existe um unico recurso compartilhado
tambem e didatica para Sistemas Operacionais por mostrar claramente sincronizacao entre threads
assim, o total de vendas permanece correto e previsivel em execucoes concorrentes

executar com
make run
make teste10x