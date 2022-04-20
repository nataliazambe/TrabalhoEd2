#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <getopt.h>

#define MAX_TITLE_TYPE 13
#define MAX_PRIMARY_TITLE 422
#define MAX_ORIGINAL_TITLE 422
#define MAX_RUNTIME_MINUTES 6
#define MAX_GENRES 33
#define MAX_FILENAME 50

// PARTICIONAMENTO POR SELEÇÃO NATURAL
typedef struct stFilm
{
	char titleType[MAX_TITLE_TYPE],
		primaryTitle[MAX_PRIMARY_TITLE],
		originalTitle[MAX_ORIGINAL_TITLE];
	unsigned char isAdult;
	unsigned short int startYear,
		endYear;
	char runtimeMinutes[MAX_RUNTIME_MINUTES],
		genres[MAX_GENRES];
} tpFilm;

void liberarFilme(tpFilm **fm)
{
	free(*fm);
	*fm = NULL;
}

void imprimirFilme(tpFilm *fm)
{
	printf("titleType: %d\n"
		   "primaryTitle: %s\n"
		   "originalTitle: %s\n"
		   "isAdult: %hhu\n"
		   "startYear: %hu\n"
		   "endYear: %hu\n"
		   "runtimeMinutes: %s\n"
		   "genres: %s\n\n",
		   fm->titleType, fm->primaryTitle, fm->originalTitle, fm->isAdult, fm->startYear, fm->endYear, fm->runtimeMinutes, fm->genres);
}

int escreverFilme(tpFilm *fm, FILE *f)
{
	if (fm != NULL)
		if (fwrite(fm, sizeof(tpFilm), 1, f) > 0)
			return 1;
	return 0;
}

tpFilm *lerFilme(FILE *f)
{
	tpFilm *fm = (tpFilm *)malloc(sizeof(tpFilm)); // aloca tamanho da struct tpFilm
	if (fread(fm, sizeof(tpFilm), 1, f) > 0)
		return fm;
	free(fm);
	return NULL;
}

typedef struct stParticionador
{
	tpFilm **memoria;
	tpFilm **reservatorio;
	int r;
	int indReserv;
	int memSize;
} tpParticionador;

tpParticionador *criarParticionador(int memSize)
{
	tpParticionador *p = (tpParticionador *)malloc(sizeof(tpParticionador));

	p->memoria = (tpFilm **)malloc(sizeof(tpFilm *) * memSize);
	p->reservatorio = (tpFilm **)malloc(sizeof(tpFilm *) * memSize);
	p->indReserv = 0;
	p->memSize = memSize;

	return p;
}

void liberarParticionador(tpParticionador **part)
{
	tpParticionador *p = *part;

	for (int i = 0; i < p->memSize; i++)
	{
		if (p->memoria[i] != NULL)
			liberarFilme(&p->memoria[i]);
	}
	free(p->memoria);
	free(p->reservatorio);
	free(p);

	(*part) = NULL;
}

// Se um texto A eh menor que B em ordem alfabetica
enum saidaComparar
{
	MENORQUE,
	MAIORQUE,
	IGUAL
};

int comparar(char *string1, char *string2)
{
	int indice;
	char a;
	char b;

	for (indice = 0;
		 (indice < MAX_ORIGINAL_TITLE) &&
		 (string1[indice] != '\0') &&
		 (string2[indice] != '\0');
		 indice++)
	{
		a = toupper(string1[indice]);
		b = toupper(string2[indice]);

		if (a < b)			 // achei alguma letra de A que eh menor que B
			return MENORQUE; // menor que
		else if (a > b)		 // achei alguma letra em A que eh maior que B, portanto B eh menor
			return MAIORQUE; // maior que
	}

	if (string1[indice] == '\0')
		if (string2[indice] == '\0')
			return IGUAL;
		else
			return MENORQUE;
	else
		return MAIORQUE;
}

int particionar(FILE *f, char *prefix, int memSize)
{
	tpParticionador *part = criarParticionador(memSize);
	int partGeradas = 0;

	// VERIFICA SE O ARQUIVO EXISTE
	if (f != NULL)
	{
		char stringAux[MAX_FILENAME];
		tpFilm *aux;
		FILE *fPart;

		// COPIA OS PRIMEIROS ELEMENTOS PARA A MEMORIA
		for (int i = 0; i < memSize; i++)
			part->memoria[i] = lerFilme(f);
		while (part->memoria[0] != NULL)
		{
			// CRIA A STRING PARA O NOME DO ARQUIVO
			sprintf(stringAux, "parts/%s%d.dat", prefix, partGeradas);
			fPart = fopen(stringAux, "w");
			while (part->indReserv < memSize)
			{
				// PROCURA PRIMEIRO ELEMENTO NA MEMORIA
				part->r = -1;
				for (int i = 0; i < memSize; i++)
					if (part->memoria[i] != NULL)
					{
						part->r = i;
						break;
					}
				// VERIFICA SE A MEMORIA ESTA VAZIA
				if (part->r != -1)
				{
					// PROCURA O ELEMENTO COM MENOR CHAVE NA MEMORIA
					for (int i = 0; i < memSize; i++)
						if (part->memoria[i] != NULL && comparar(part->memoria[i]->originalTitle, part->memoria[part->r]->originalTitle) == MENORQUE)
							part->r = i;

					// ESCREVE O ELEMENTO
					escreverFilme(part->memoria[part->r], fPart);
					// BUSCA O PROXIMO
					while (part->indReserv < memSize)
					{
						aux = lerFilme(f);
						if (aux != NULL)
						{
							// SE MENOR QUE O ESCOLHIDO, ADICIONA AO RESERVATORIO
							if (comparar(aux->originalTitle, part->memoria[part->r]->originalTitle) == MENORQUE)
							{
								part->reservatorio[part->indReserv] = aux;
								part->indReserv++;
							}
							// SE MAIOR QUE O ESCOLHIDO, SUBSTITUI
							else
							{
								liberarFilme(&part->memoria[part->r]);
								part->memoria[part->r] = aux;
								break;
							}
						}
						// SE ULTIMO ELEMENTO LIDO FOR NULO, ENTAO FIM DO ARQUIVO
						else
						{
							liberarFilme(&part->memoria[part->r]);
							break;
						}
					}
				}
				// SAI DO LOOP CASO NAO TENHA NENHUM ELEMENTO NA MEMORIA
				else
					break;
			}
			// CASO O RESERVATORIO TENHA ENCHIDO, REMOVE O ULTIMO ELEMENTO ESCRITO
			if (part->indReserv == memSize && part->memoria[part->r] != NULL)
				liberarFilme(&part->memoria[part->r]);

			// ESCREVE OS ELEMENTOS RESTANTES DA MEMORIA NA PARTICAO
			do
			{
				// PROCURA O PRIMEIRO ELEMENTO NA MEMORIA
				part->r = -1;
				for (int i = 0; i < memSize; i++)
					if (part->memoria[i] != NULL)
					{
						part->r = i;
						break;
					}
				// PROCURA O MENOR ELEMENTO NA MEMORIA
				if (part->r != -1)
				{
					for (int i = 0; i < memSize; i++)
						if (part->memoria[i] != NULL && comparar(part->memoria[i]->originalTitle, part->memoria[part->r]->originalTitle) == MENORQUE)
							part->r = i;
					// ESCREVE E LIBERA O MENOR
					escreverFilme(part->memoria[part->r], fPart);
					liberarFilme(&part->memoria[part->r]);
				}
			} while (part->r != -1);

			// FECHA O ARQUIVO E COPIA OS DADOS DO RESERVATORIO PARA A MEMORIA
			fclose(fPart);
			for (int i = 0; i < part->indReserv; i++)
				part->memoria[i] = part->reservatorio[i];
			part->indReserv = 0;
			partGeradas++;
		}
	}

	liberarParticionador(&part);

	return partGeradas;
}

// BUSCA BINÁRIA
tpFilm *pesquisa_binaria(FILE *arquivo, char *originalTitle)
{
	long tam_estrutura = sizeof(tpFilm);
	long tam_arquivo = 0;
	long total_registros = 0;

	fseek(arquivo, 0L, SEEK_END); // posiciona no final do arquivo
	tam_arquivo = ftell(arquivo); // retorna quantos bytes foram "lidos" até o momento
	fseek(arquivo, 0L, SEEK_SET); // reposiciona o ponteiro para a posição desejada, neste caso, inicio do arquivo

	total_registros = tam_arquivo / tam_estrutura;

	long inicio_parte = 0;
	long fim_parte = total_registros;
	long posicao_atual = 0;
	tpFilm *filme = NULL;

	int i = 0;

	int comparacoes = 0;

	do
	{
		posicao_atual = ((fim_parte - inicio_parte) / 2) + inicio_parte;

		fseek(arquivo, posicao_atual * tam_estrutura, SEEK_SET);
		tpFilm *fm = lerFilme(arquivo);

		int saidaComparacao = comparar(originalTitle, fm->originalTitle);

		if (saidaComparacao == MAIORQUE)
			inicio_parte = posicao_atual + 1;
		else if (saidaComparacao == MENORQUE)
			fim_parte = posicao_atual - 1;
		else
			filme = fm;

		if (filme == NULL)
			liberarFilme(&fm);
		else
			break;

	} while (fim_parte >= inicio_parte);

	return filme;
}

// INTEGRAÇÃO POR INTERCALAÇÃO ÓTIMA
typedef struct stFilenameList
{
	struct stFilenameList *previous;
	char *filename;
	struct stFilenameList *next;
} tpFilenameList;

typedef struct stFilenameQueue
{
	struct stFilenameList *first;
	struct stFilenameList *last;
} tpFilenameQueue;

tpFilenameQueue *criarFilenameQueue()
{
	tpFilenameQueue *queue = (tpFilenameQueue *)malloc(sizeof(tpFilenameQueue));
	queue->first = NULL;
	queue->last = NULL;
	return queue;
}

void inserirArquivoNaFila(tpFilenameQueue *queue, char *filename)
{
	tpFilenameList *no = (tpFilenameList *)malloc(sizeof(tpFilenameList));

	// COPIA A STRING
	char *filenameCopia = (char *)malloc(sizeof(char) * MAX_FILENAME);
	int i;
	for (i = 0; filename[i] != '\0'; i++)
		filenameCopia[i] = filename[i];
	filenameCopia[i] = '\0';

	// INICIALIZA OS ATRIBUTOS
	no->filename = filenameCopia;
	no->next = NULL;

	// INICIO DA FILA SE VAZIO
	if (queue->last == NULL)
		queue->first = no;
	// SENÃO COLOCA NO FIM DA FILA
	else
	{
		queue->last->next = no;
		no->previous = queue->last;
	}

	queue->last = no;
}

char *popArquivoNaFila(tpFilenameQueue *queue)
{
	// SE VAZIO, RETORNA NULO
	if (queue->first == NULL)
		return NULL;
	// SENÃO RETORNA O PRIMEIRO
	else
	{
		tpFilenameList *first = queue->first;
		queue->first = first->next;

		// LIBERA O ARQUIVO
		char *filename = first->filename;
		free(first);

		// SE PRIMEIRO, LIMPA A FILA
		if (first == queue->last)
			queue->last = NULL;

		return filename;
	}
}

int verificarFilaVazia(tpFilenameQueue *queue)
{
	return queue->first == NULL;
}

int integrar(char *nome_saida, char *prefix, int max_arquivos)
{
	tpFilenameQueue *fila = criarFilenameQueue();
	FILE *saida = NULL;

	// PROCURO QUEM SÃO MINHAS PARTIÇÕES
	char stringAux[MAX_FILENAME];
	int i;
	for (i = 0; 1; i++)
	{
		sprintf(stringAux, "parts/%s%d.dat", prefix, i);
		saida = fopen(stringAux, "r");
		if (saida == NULL)
			break;
		else
		{
			inserirArquivoNaFila(fila, stringAux);
			fclose(saida);
		}
	}

	assert(i > 1 && "existe apenas uma ou nenhuma particao ordenada");

	// NUMERO MAXIMO DE FILES
	int cntIntermediarios = 0;
	max_arquivos--;
	FILE *entradas[max_arquivos];
	tpFilm *filmes[max_arquivos];
	char *paraRemover[max_arquivos];
	int menor;

	while (1)
	{
		// ABRO AS PARTIÇÕES
		int abertos = 0;
		for (abertos = 0; (abertos < max_arquivos) && !verificarFilaVazia(fila); abertos++)
		{
			char *filename = popArquivoNaFila(fila);
			entradas[abertos] = fopen(filename, "r");
			filmes[abertos] = NULL;
			paraRemover[abertos] = filename;
		}

		if (abertos > 1)
		{
			// COLOCO NO FIM DA FILA PARA JUNTAR AO FINAL
			sprintf(stringAux, "parts/__tmp_%d.dat", cntIntermediarios);
			inserirArquivoNaFila(fila, stringAux);
			saida = fopen(stringAux, "w");

			// COLOCO O MENOR DOS ABERTOS NO INTERMEDIARIO
			while (1)
			{
				menor = -1;
				for (i = 0; i < abertos; i++)
				{
					if (filmes[i] == NULL)
						filmes[i] = lerFilme(entradas[i]);

					if (filmes[i] != NULL)
						if (menor < 0 ||
							comparar(filmes[i]->originalTitle,
									 filmes[menor]->originalTitle) == MENORQUE)
							menor = i;
				}
				if (menor > -1)
				{
					escreverFilme(filmes[menor], saida);
					liberarFilme(&filmes[menor]);
				}
				else
					break;
			}
			cntIntermediarios += 1;

			// FECHO AS PARTIÇÕES
			fclose(saida);
			for (i = 0; i < abertos; i++)
			{
				fclose(entradas[i]);
				remove(paraRemover[i]);
				free(paraRemover[i]);
			}
		}
		else
			break;
	}

	free(fila);

	// MOVO PARA A RAIZ E RENOMEIO
	if (rename(stringAux, nome_saida) != 0)
		printf("Nao foi possivel renomear, o arquivo esta localizado em: '%s'\n",
			   stringAux);
	return cntIntermediarios;
}

// Manual
void ajuda(char *name)
{
	fprintf(stderr, "\
	[uso] %s <opcoes>\n\
	-h, --help          mostra essa tela e sai.\n\
	-m, --modo=MODO     operacao a ser executada.\n\
	se MODO == 'particionar'\n\
		-p, --prefixo=PREFIXO                nome que precede cada parte\n\
		-r, --max-registros=MAX_REGISTROS    metade da quantidade maxima de registros na memoria\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo na raiz para particionar\n\
	se MODO == 'buscar'\n\
		-n, --nome=TITULO                    original title do registro a buscar\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado na raiz para buscar\n\
	se MODO == 'integrar'\n\
		-q, --max-arquivos=MAX_ARQUIVOS      quantidade maxima de arquivos abertos simultaneamente\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado das pares a ser salvo na raiz\n\
		-p, --prefixo=PREFIXO				 nome que precede cada parte\n\
",
			name);
	exit(-1);
}

int main(int argc, char **argv)
{
	int opt;

	/* Variáveis que receberão os argumentos
	 * das opções. */
	char *modo = NULL, *prefixo = NULL, *nome = NULL,
		 *arquivo = NULL;

	int max_registros = 0, max_arquivos = 0;

	const struct option stopcoes[] = {
		{"help", no_argument, 0, 'h'},
		{"modo", required_argument, 0, 'm'},
		{"prefixo", required_argument, 0, 'p'},
		{"nome", required_argument, 0, 'n'},
		{"max-registros", required_argument, 0, 'r'},
		{"arquivo", required_argument, 0, 'a'},
		{"max-arquivos", required_argument, 0, 'q'},
		{0, 0, 0, 0},
	};

	if (argc < 2)
		ajuda(argv[0]);

	while ((opt = getopt_long(argc, argv, "hn:m:p:n:r:a:q", stopcoes, NULL)) > 0)
	{
		switch (opt)
		{
		case 'h': /* -h ou --help */
			ajuda(argv[0]);
			break;
		case 'm': /* -m ou --modo */
			modo = optarg;
			break;
		case 'p': /* -p ou --prefixo */
			prefixo = optarg;
			break;
		case 'n': /* -n ou --nome */
			nome = optarg;
			break;
		case 'r': /* -r ou --max-registros */
			max_registros = atoi(optarg);
			break;
		case 'a': /* -a ou --arquivo */
			arquivo = optarg;
			break;
		case 'q': /* -q ou --max-arquivos */
			max_arquivos = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Opcao invalida ou faltando argumento: `%c'\n", optopt);
			return -1;
		}
	}

	assert(modo != NULL);
	assert(arquivo != NULL);
	if (comparar(modo, "particionar") == IGUAL)
	{
		FILE *f = fopen(arquivo, "r");
		assert(f != NULL && "arquivo inexistente");

		assert(prefixo != NULL);
		assert(max_registros > 0);
		int qtde = particionar(f, prefixo, max_registros);
		printf("%d particoes geradas\n", qtde);

		fclose(f);
	}
	else if (comparar(modo, "buscar") == IGUAL)
	{
		FILE *f = fopen(arquivo, "r");
		assert(f != NULL && "arquivo inexistente");

		assert(nome != NULL);
		tpFilm *fm = pesquisa_binaria(f, nome);
		if (fm != NULL)
		{
			imprimirFilme(fm);
			liberarFilme(&fm);
		}
		else
			printf("Filme nao encontrado\n");

		fclose(f);
	}
	else if (comparar(modo, "integrar") == IGUAL)
	{
		assert(max_arquivos > 0);
		assert(prefixo != NULL);
		int qtde = integrar(arquivo, prefixo, max_arquivos);
		printf("%d particoes intermediarias\n", qtde);
	}
	else
	{
		printf("Modo invalido\n");
	}

	return 0;
}
