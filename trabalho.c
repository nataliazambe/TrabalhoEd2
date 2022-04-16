#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

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
	tpFilm *fm = (tpFilm*) malloc(sizeof(tpFilm)); // aloca tamanho da struct tpFilm
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

int menorQue(char *string1, char *string2)
{

	char a;
	char b;

	for (int indice = 0; ((indice < 422) || ((string1[indice] != '\0') || (string2[indice] != '\0'))); indice++)
	{
		a = toupper(string1[indice]);
		b = toupper(string2[indice]);

		if (a < b)		// achei alguma letra de A que eh menor que B
			return 1;								// menor que
		else if (a > b) // achei alguma letra em A que eh maior que B, portanto B eh menor
			return 0;								// maior que
	}

	return a == '\0';
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
						if (part->memoria[i] != NULL && menorQue(part->memoria[i]->originalTitle, part->memoria[part->r]->originalTitle))
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
							if (menorQue(aux->originalTitle, part->memoria[part->r]->originalTitle))
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
						if (part->memoria[i] != NULL && menorQue(part->memoria[i]->originalTitle, part->memoria[part->r]->originalTitle))
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

void pesquisa_binaria(FILE* arquivo, char *originalTitle){
    int tam_estrutura      = sizeof(tpFilm);
    int tam_arquivo        = 0;
    int total_registros    = 0;
    tpFilm filme;

    fseek(arquivo, 0L, SEEK_END); //posiciona no final do arquivo
    tam_arquivo = ftell(arquivo); //retorna quantos bytes foram "lidos" até o momento
    fseek(arquivo, 0L, SEEK_SET); //reposiciona o ponteiro para a posição desejada, neste caso, inicio do arquivo

    total_registros = tam_arquivo / tam_estrutura;

    int achou = 0;

    int inicio_parte = 0;
    int fim_parte = total_registros;
    int posicao_atual = 0;

    int i = 0;

    int comparacoes = 0;

    do {
        posicao_atual = ((fim_parte - inicio_parte) / 2) + inicio_parte;

        fseek(arquivo, posicao_atual * tam_estrutura, SEEK_SET);
        fread(&filme, tam_estrutura, 1, arquivo);

        for (i = 0; i < 6; i++)
            if (originalTitle[i] > filme.originalTitle[i]){
                inicio_parte = posicao_atual + 1;
                break;
            }
            else if (originalTitle[i] < filme.originalTitle[i]){
                fim_parte = posicao_atual - 1;
                break;
            }

        if (i == 6)
            achou = 1;

        comparacoes++;
    } while(!achou && (fim_parte >= inicio_parte));
    fclose(arquivo);

    if (achou)
        imprimirFilme(&filme);
    else
        printf("Original title %s not found \n", originalTitle);

    printf("Comparações %d\n", comparacoes);
}

int main(int ac, char **av)
{
    FILE *arquivo_entrada = fopen(av[1], "r");
    assert(arquivo_entrada != NULL);
	
	//particionar(arquivo_entrada, "test", 250);

    for (int i = 0; i < 100; i++){
		tpFilm* fm = lerFilme(arquivo_entrada);
		imprimirFilme(fm);
		//escreverFilme(fm, test);
		liberarFilme(&fm);
    }
	    
    fclose(arquivo_entrada);
	return EXIT_SUCCESS;
}
