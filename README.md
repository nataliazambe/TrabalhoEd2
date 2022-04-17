# Trabalho ED2

## Descrição
Este trabalho consiste em duas implementações a primeira envolvendo ordenação de arquivos;
e, a segunda implementação envolve a construção de uma estrutura de índices baseada em árvore digital binária.

### Implementação 1
A primeira implementação consiste em ordenar o arquivo de dados films.dat pelo atributo originalTitle. Para
isso, o aluno deverá utilizar a técnica de seleção natural, com valor de M de 10.000 registros e com o reservatório
do mesmo tamanho. O aluno deve adotar a intercalação ótima com F = 3 na etapa de intercalação.

### Implementação 2
A segunda implementação deverá buscar e listar um ou mais registros no arquivo de filmes ordenado pelo atributo
originalTitle. Para isso, o aluno escrever um programa que peça a o título original da obra e em seguida execute a
busca binária, acessando de forma direta o arquivo e executando os saltos de acordo com o algoritmo de busca binária.

## Como usar

1. Compilar usando `make`
2. Escolher uma das opções abaixo utilizando o binário `bin*` gerado
```
-h, --help          mostra a tela de ajuda
-m, --modo=MODO     operacao a ser executada.
se MODO == 'particionar'
	-p, --prefixo=PREFIXO                nome que precede cada parte
	-r, --max-registros=MAX_REGISTROS    quantidade maxima de registros na memoria
	-a, --arquivo=NOME_ARQUIVO           nome do arquivo na raiz para particionar
se MODO == 'buscar'
	-n, --nome=TITULO                    original title do registro a buscar
	-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado na raiz para buscar
se MODO == 'integrar'
	-q, --max-arquivos=MAX_ARQUIVOS      quantidade maxima de arquivos abertos simultaneamente
	-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado das pares a ser salvo na raiz
```

## Autores
* Natalia Zambe
* Guilherme Magno
* Matheus Amorim