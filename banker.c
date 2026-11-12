#include <stdio.h>
#include <stdlib.h>

int num_recursos = 0;
int *disponivel = NULL;

int converter_inteiro(char *texto, int *valor){
    
    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (fim == texto)
        return 0;
    while (*fim == ' ' || *fim == '\n' || *fim == '\r')
        fim++;
    if (*fim != '\0' || numero < 0)
        return 0;
    *valor = (int)numero;
    return 1;
}

int ler_argumentos(int argc, char *argv[]){
    
    num_recursos = argc - 1;
    if (num_recursos < 1) {
        fprintf(stderr, "Erro: informe as instancias de cada recurso. Exemplo: ./banker 10 5 7\n");
        return 0;
    }

    disponivel = malloc(num_recursos * sizeof(int));
    for (int i = 0; i < num_recursos; i++) {
        if (!converter_inteiro(argv[i + 1], &disponivel[i])) {
            fprintf(stderr, "Erro: argumento invalido '%s'. Use inteiros nao negativos.\n", argv[i + 1]);
            free(disponivel);
            disponivel = NULL;
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]){
    
    if (!ler_argumentos(argc, argv))
        return 1;

    printf("recursos: %d\n", num_recursos);
    for (int i = 0; i < num_recursos; i++)
        printf("%d ", disponivel[i]);
    printf("\n");

    free(disponivel);
    return 0;
}