#PROG=algoritmo-do-banqueiro
PROG=algoritmo-do-banqueiro-interativo

run: compile
	./$(PROG).exe

compile: clean
	gcc -pedantic $(PROG).c -o $(PROG).exe

clean:
	rm -f $(PROG).exe