#include <stdio.h>
#include <stdlib.h>

int converter_inteiro(char *texto, int *valor)
{
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

int main(int argc, char *argv[])
{
    int valor;

    if (argc != 2) {
        printf("uso: ./banker numero\n");
        return 1;
    }
    if (converter_inteiro(argv[1], &valor))
        printf("valido: %d\n", valor);
    else
        printf("invalido\n");
    return 0;
}