#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int num_recursos = 0;
int num_clientes = 0;
int *disponivel = NULL;
int **maximo = NULL;
int **alocado = NULL;
int **necessidade = NULL;

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
int linha_vazia(const char *linha)
{
    while (*linha) {
        if (*linha != ' ' && *linha != '\n' && *linha != '\r')
            return 0;
        linha++;
    }
    return 1;
}

int **alocar_matriz(int linhas, int colunas){

    int **matriz = malloc(linhas * sizeof(int *));
    for (int i = 0; i < linhas; i++)
        matriz[i] = calloc(colunas, sizeof(int));
    return matriz;

}

int carregar_clientes(const char *caminho)
{
    char linha[1000];
    FILE *arquivo = fopen(caminho, "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", caminho);
        return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo))
        if (!linha_vazia(linha))
            num_clientes++;

    if (num_clientes == 0) {
        fprintf(stderr, "Erro: %s nao possui clientes\n", caminho);
        fclose(arquivo);
        return 0;
    }

    rewind(arquivo);
    maximo = alocar_matriz(num_clientes, num_recursos);
    alocado = alocar_matriz(num_clientes, num_recursos);
    necessidade = alocar_matriz(num_clientes, num_recursos);

    int cliente = 0;
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha_vazia(linha))
            continue;

        int coluna = 0;
        char *inicio = linha;
        while (1) {
            char *virgula = strchr(inicio, ',');
            if (virgula)
                *virgula = '\0';

            if (coluna >= num_recursos || !converter_inteiro(inicio, &maximo[cliente][coluna])) {
                fprintf(stderr, "Erro: formato invalido em %s, cliente %d (esperado %d colunas de inteiros)\n",
                        caminho, cliente, num_recursos);
                fclose(arquivo);
                return 0;
            }
            necessidade[cliente][coluna] = maximo[cliente][coluna];
            coluna++;

            if (!virgula)
                break;
            inicio = virgula + 1;
        }

        if (coluna != num_recursos) {
            fprintf(stderr, "Erro: %s, cliente %d tem %d colunas, esperado %d\n",
                    caminho, cliente, coluna, num_recursos);
            fclose(arquivo);
            return 0;
        }
        cliente++;
    }

    fclose(arquivo);
    return 1;
}

int main(int argc, char *argv[])
{
    if (!ler_argumentos(argc, argv))
        return 1;
    if (!carregar_clientes("customer.txt")) {
        free(disponivel);
        return 1;
    }

    printf("recursos: %d, clientes: %d\n", num_recursos, num_clientes);
    for (int i = 0; i < num_clientes; i++) {
        for (int j = 0; j < num_recursos; j++)
            printf("%d ", maximo[i][j]);
        printf("\n");
    }

    free(disponivel);
    return 0;
}