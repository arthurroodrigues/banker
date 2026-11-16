# Algoritmo de Banker

Implementação 5 de Infraestrutura de Software (CESAR School).
Aluno: Arthur Rodrigues de Andrade, turma 3B, login `ara`.

## Descrição

Implementação em C do algoritmo de Banker (Silberschatz, *Operating System Concepts*, 10ª ed., p. P-45) para evitar deadlock. O programa lê os clientes e uma sequência de comandos de arquivos, decide se cada requisição mantém o sistema em estado seguro e grava o resultado em `result.txt`.

O número de clientes e o de recursos não são fixos:
- o número de **recursos** é a quantidade de argumentos na linha de comando
- o número de **clientes** é a quantidade de linhas do `customer.txt`

## Requisitos

- Linux, Unix ou macOS (testado no Ubuntu pelo WSL)
- `gcc` e `make`

## Compilar

```
make
```

Gera o executável `banker`. Para apagar o executável e o `result.txt`:

```
make clean
```

## Executar

Cada argumento é o número de instâncias de um tipo de recurso. Exemplo com três recursos (10, 5 e 7 instâncias):

```
./banker 10 5 7
```

O programa precisa dos arquivos `customer.txt` e `commands.txt` na pasta onde é executado. A saída é gravada em `result.txt`.

## Formato dos arquivos

**customer.txt**: uma linha por cliente, com o máximo que ele pode pedir de cada recurso, separado por vírgula.

```
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

**commands.txt**: um comando por linha.

| Comando | Significado |
|---|---|
| `RQ cliente r0 r1 ...` | cliente requisita recursos |
| `RL cliente r0 r1 ...` | cliente libera recursos |
| `*` | imprime o estado atual (MAXIMUM, ALLOCATION, NEED e AVAILABLE) |

```
RQ 0 0 2 1
RL 0 0 1 1
*
```

## Saída (`result.txt`)

Cada requisição gera uma destas mensagens:

- `Allocate to customer C the resources ...`: requisição aceita
- `The resources ... are not enough to customer C request ...`: não há recursos disponíveis agora
- `The customer C request ... was denied because exceed its maximum allocation`: pedido passa do que o cliente ainda pode pedir
- `The customer C request ... was denied because result in an unsafe state`: aceitar deixaria o sistema em estado inseguro (a alocação é desfeita)

Cada liberação gera `Release from customer C the resources ...`. Se o cliente tentar liberar mais do que possui, o pedido é negado com `The customer C release ... was denied because exceed its allocation` (mensagem definida por mim, o enunciado não especifica esse caso).

## Tratamento de erros

O programa mostra uma mensagem de erro no `stderr` e encerra, sem travar, quando:

- nenhum argumento é informado, ou algum não é um inteiro não negativo
- `customer.txt` ou `commands.txt` não existe ou não pode ser aberto
- `customer.txt` está vazio ou com formato inválido (campo não numérico ou vazio, quantidade de colunas diferente do número de recursos)
- uma linha do `commands.txt` tem comando desconhecido, cliente inexistente ou quantidade de valores diferente do número de recursos

Exemplos:

```
./banker
./banker 10 5 x
./banker 10 5
```

## Estrutura

| Arquivo | Responsabilidade |
|---|---|
| `banker.c` | todo o código: leitura, validação, algoritmo de Banker e saída |
| `Makefile` | compilação (`make`) e limpeza (`make clean`) |
| `README.md` | este arquivo |
| `evidencias.log` | registro do terminal durante o desenvolvimento (`script -a`) |

## Testes

O resultado para os arquivos de exemplo do professor foi comparado com o `result.txt` dele e o `diff` não mostrou nenhuma diferença:

```
./banker 10 5 7
diff result.txt esperado.txt
```

Casos limite e inválidos testados: arquivos ausentes, número de argumentos diferente do de colunas, argumento inválido, cliente inexistente, comando desconhecido, valores faltando ou sobrando, pedido com zeros, pedido igual ao máximo e pedido além do máximo.