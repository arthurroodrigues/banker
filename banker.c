#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void requisicao(int cliente, int *pedido);
void imprimir_estado(void);
int liberacao(int cliente, int *pedido);

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
    while (*fim == ' ' || *fim == '\t' || *fim == '\n' || *fim == '\r')
        fim++;
    if (*fim != '\0' || numero < 0 || numero > INT_MAX)
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
int linha_vazia(const char *linha){
    while (*linha) {
        if (*linha != ' ' && *linha != '\t' && *linha != '\n' && *linha != '\r')
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

int carregar_clientes(const char *caminho){
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

int interpretar_comando(char *linha, int numero_linha, const char *caminho){
    char *comando = strtok(linha, " \t\r\n");

    if (strcmp(comando, "*") == 0) {
        if (strtok(NULL, " \t\r\n") != NULL) {
            fprintf(stderr, "Erro: %s, linha %d: o comando * nao recebe parametros\n", caminho, numero_linha);
            return 0;
        }
        imprimir_estado();
        return 1;
    }

    if (strcmp(comando, "RQ") != 0 && strcmp(comando, "RL") != 0) {
        fprintf(stderr, "Erro: %s, linha %d: comando '%s' invalido\n", caminho, numero_linha, comando);
        return 0;
    }

    int cliente;
    char *token = strtok(NULL, " \t\r\n");
    if (comando == NULL)
        return 1;
        
    if (token == NULL || !converter_inteiro(token, &cliente) || cliente >= num_clientes) {
        fprintf(stderr, "Erro: %s, linha %d: numero de cliente invalido\n", caminho, numero_linha);
        return 0;
    }

    int valores[num_recursos];
    for (int j = 0; j < num_recursos; j++) {
        token = strtok(NULL, " \t\r\n");
        if (token == NULL || !converter_inteiro(token, &valores[j])) {
            fprintf(stderr, "Erro: %s, linha %d: esperado %d valores inteiros\n", caminho, numero_linha, num_recursos);
            return 0;
        }
    }

    if (strtok(NULL, " \t\r\n") != NULL) {
        fprintf(stderr, "Erro: %s, linha %d: valores demais, esperado %d\n", caminho, numero_linha, num_recursos);
        return 0;
    }

    if (strcmp(comando, "RQ") == 0) {
        requisicao(cliente, valores);
        return 1;
    }

    if (!liberacao(cliente, valores)) {
        fprintf(stderr, "Erro: %s, linha %d: cliente %d tentou liberar mais do que tem alocado\n",  
            caminho, numero_linha, cliente);
        return 0;
    }
    return 1;
}

int processar_comandos(const char *caminho){
    char linha[1000];
    int numero_linha = 0;
    FILE *arquivo = fopen(caminho, "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", caminho);
        return 0;
    }

    if (freopen("result.txt", "w", stdout) == NULL) {
    fprintf(stderr, "Erro: nao foi possivel criar result.txt\n");
    fclose(arquivo);
    return 0;
    }

    while (fgets(linha, sizeof(linha), arquivo)) {
        numero_linha++;
        if (linha_vazia(linha))
            continue;
        if (!interpretar_comando(linha, numero_linha, caminho)) {
            fclose(arquivo);
            return 0;
        }
    }

    fclose(arquivo);
    return 1;
}

int estado_seguro(void){
    int trabalho[num_recursos];
    int terminado[num_clientes];

    for (int j = 0; j < num_recursos; j++)
        trabalho[j] = disponivel[j];
    for (int i = 0; i < num_clientes; i++)
        terminado[i] = 0;

    int encontrou = 1;
    while (encontrou) {
        encontrou = 0;
        for (int i = 0; i < num_clientes; i++) {
            if (terminado[i])
                continue;

            int pode_terminar = 1;
            for (int j = 0; j < num_recursos; j++) {
                if (necessidade[i][j] > trabalho[j]) {
                    pode_terminar = 0;
                    break;
                }
            }

            if (pode_terminar) {
                for (int j = 0; j < num_recursos; j++)
                    trabalho[j] += alocado[i][j];
                terminado[i] = 1;
                encontrou = 1;
            }
        }
    }

    for (int i = 0; i < num_clientes; i++)
        if (!terminado[i])
            return 0;
    return 1;
}

void imprimir_vetor(int *vetor)
{
    for (int j = 0; j < num_recursos; j++) {
        if (j > 0)
            printf(" ");
        printf("%d", vetor[j]);
    }
}

void requisicao(int cliente, int *pedido){
    for (int j = 0; j < num_recursos; j++) {
        if (pedido[j] > necessidade[cliente][j]) {
            printf("The customer %d request ", cliente);
            imprimir_vetor(pedido);
            printf(" was denied because exceed its maximum allocation\n");
            return;
        }
    }

    for (int j = 0; j < num_recursos; j++) {
        if (pedido[j] > disponivel[j]) {
            printf("The resources ");
            imprimir_vetor(disponivel);
            printf(" are not enough to customer %d request ", cliente);
            imprimir_vetor(pedido);
            printf(" \n");
            return;
        }
    }

    for (int j = 0; j < num_recursos; j++) {
        disponivel[j] -= pedido[j];
        alocado[cliente][j] += pedido[j];
        necessidade[cliente][j] -= pedido[j];
    }

    if (!estado_seguro()) {
        for (int j = 0; j < num_recursos; j++) {
            disponivel[j] += pedido[j];
            alocado[cliente][j] -= pedido[j];
            necessidade[cliente][j] += pedido[j];
        }
        printf("The customer %d request ", cliente);
        imprimir_vetor(pedido);
        printf(" was denied because result in an unsafe state\n");
        return;
    }

    printf("Allocate to customer %d the resources ", cliente);
    imprimir_vetor(pedido);
    printf("\n");
}

int liberacao(int cliente, int *pedido){
    for (int j = 0; j < num_recursos; j++)
        if (pedido[j] > alocado[cliente][j])
            return 0;

    for (int j = 0; j < num_recursos; j++) {
        disponivel[j] += pedido[j];
        alocado[cliente][j] -= pedido[j];
        necessidade[cliente][j] += pedido[j];
    }

    printf("Release from customer %d the resources ", cliente);
    imprimir_vetor(pedido);
    printf("\n");
    return 1;
}

void formatar_vetor(char *destino, int *vetor){
    destino[0] = '\0';
    for (int j = 0; j < num_recursos; j++) {
        char numero[16];
        snprintf(numero, sizeof(numero), j > 0 ? " %d" : "%d", vetor[j]);
        strcat(destino, numero);
    }
}

void imprimir_estado(void){
    char texto_maximo[4000];
    char texto_alocado[4000];
    char texto_necessidade[4000];

    printf("MAXIMUM   |   ALLOCATION  |   NEED\n");
    for (int i = 0; i < num_clientes; i++) {
        formatar_vetor(texto_maximo, maximo[i]);
        formatar_vetor(texto_alocado, alocado[i]);
        formatar_vetor(texto_necessidade, necessidade[i]);
        printf("%-10s|   %-12s|   %s \n", texto_maximo, texto_alocado, texto_necessidade);
    }
    printf("AVAILABLE ");
    imprimir_vetor(disponivel);
    printf(" \n");
}

int main(int argc, char *argv[]){
    if (!ler_argumentos(argc, argv))
        return 1;
    if (!carregar_clientes("customer.txt")) {
        free(disponivel);
        return 1;
    }
    if (!processar_comandos("commands.txt")) {
        free(disponivel);
        return 1;
    }

    free(disponivel);
    return 0;
}