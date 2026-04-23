o programa possui regiao critica na insercao da lista compartilhada head
as duas linhas criticas sao new_node->next = head e head = new_node e precisam ser executadas juntas
sem controle de concorrencia, uma thread pode sobrescrever a cabeca inserida por outra e causar perda de nos
a solucao escolhida foi semaforo binario com valor inicial 1
no codigo, sem_wait entra na regiao critica e sem_post libera o acesso para a proxima thread
essa abordagem garante exclusao mutua na atualizacao de head e evita perdas e insercoes parciais
tambem foram adicionados prints de DEBUG antes e depois da regiao critica para acompanhar as threads
ao final, a validacao confere total esperado de 10 insercoes por thread e integridade sem perdas/duplicatas por seq_id
alem disso, ha validacao de parametros, checagem de malloc/calloc e liberacao completa de memoria com sem_destroy

executar com
make run
make teste10x
