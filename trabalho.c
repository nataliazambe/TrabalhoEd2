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

	char a;
	char b;

	for (int indice = 0, a = toupper(string1[indice]), b = toupper(string2[indice]);
		 (indice < MAX_ORIGINAL_TITLE) && (a != '\0') && (b != '\0');
		 indice++, a = toupper(string1[indice]), b = toupper(string2[indice]))
	{
		if (a < b)			 // achei alguma letra de A que eh menor que B
			return MENORQUE; // menor que
		else if (a > b)		 // achei alguma letra em A que eh maior que B, portanto B eh menor
			return MAIORQUE; // maior que
	}

	if (a == '\0' && b == '\0')
		return IGUAL;
	else if (a == '\0')
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
		char stringAux[50];
		tpFilm *aux;
		FILE *fPart;

		// COPIA OS PRIMEIROS ELEMENTOS PARA A MEMORIA
		for (int i = 0; i < memSize; i++)
			part->memoria[i] = lerFilme(f);
		while (part->memoria[0] != NULL)
		{
			// CRIA A STRING PARA O NOME DO ARQUIVO
			sprintf(stringAux, "parts/%s%d.dat", prefix, partGeradas);
			fPart = fopen(stringAux, "wb");
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

tpFilm *pesquisa_binaria(FILE *arquivo, char *originalTitle)
{
	int tam_estrutura = sizeof(tpFilm);
	int tam_arquivo = 0;
	int total_registros = 0;

	fseek(arquivo, 0L, SEEK_END); // posiciona no final do arquivo
	tam_arquivo = ftell(arquivo); // retorna quantos bytes foram "lidos" até o momento
	fseek(arquivo, 0L, SEEK_SET); // reposiciona o ponteiro para a posição desejada, neste caso, inicio do arquivo

	total_registros = tam_arquivo / tam_estrutura;

	int inicio_parte = 0;
	int fim_parte = total_registros;
	int posicao_atual = 0;
	tpFilm *filme = NULL;

	int i = 0;

	int comparacoes = 0;

	do
	{
		posicao_atual = ((fim_parte - inicio_parte) / 2) + inicio_parte;

		fseek(arquivo, posicao_atual * tam_estrutura, SEEK_SET);
		tpFilm *fm = lerFilme(arquivo);

		int saidaComparacao = comparar(originalTitle, &(fm->originalTitle));

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

// Manual
void ajuda(char *name)
{
	fprintf(stderr, "\
	[uso] %s <opcoes>\n\
	-h, --help          mostra essa tela e sai.\n\
	-m, --modo=MODO     operacao a ser executada.\n\
	se MODO == 'particionar'\n\
		-p, --prefixo=PREFIXO                nome que precede cada parte\n\
		-r, --max-registros=MAX_REGISTROS    quantidade maxima de registros na memoria\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo na raiz para particionar\n\
	se MODO == 'buscar'\n\
		-n, --nome=TITULO                    original title do registro a buscar\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado na raiz para buscar\n\
	se MODO == 'integrar'\n\
		-q, --max-arquivos=MAX_ARQUIVOS      quantidade maxima de arquivos abertos simultaneamente\n\
		-a, --arquivo=NOME_ARQUIVO           nome do arquivo ordenado das pares a ser salvo na raiz\n\
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

	assert (modo != NULL);
	assert (arquivo != NULL);
	FILE *f = fopen(arquivo, "r");
	assert (f != NULL);
	if (comparar(modo, "particionar") == IGUAL)
	{
	    assert (prefixo != NULL);
	    assert(max_registros > 0);
		int qtde = particionar(f, prefixo, max_registros);
		printf("%d particoes geradas", qtde);
	}
	else if (comparar(modo, "buscar") == IGUAL)
	{
		assert (nome != NULL);
		tpFilm *fm = pesquisa_binaria(f, nome);
		if (fm != NULL)
		{
			imprimirFilme(fm);
			liberarFilme(&fm);
		}
		else
			printf("Filme nao encontrado");
	}
	else if (comparar(modo, "integrar") == IGUAL)
	{
		assert (max_arquivos > 0);
		// coloquem a parte da integração aqui
	}
	else
	{
		printf("Modo invalido");
	}
	fclose(f);

	return 0;
}
