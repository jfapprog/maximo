/*
** UC: 21111 - Sistemas Operativos
** e-fólio B 2014/15 (mtmax.c)
**
** Aluno: 1301913 - José Póvoa
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define DIMBUF 10
#define MAXINT 99999999

/*protótipos das funções*/
void Inicializar(int *v, int dim);
int CalcularNItCons(void);
int Maximo(int *v, int dim);
void * TarefaProdutora(void *arg);

/*variáveis globais*/
int *v;	/*vetor a pesquisar*/
int nv;	/*dimensao do vetor v*/
int nt;	/*número de sub-tarefas produtoras*/
int *buf; /*buffer produtor/consumidor*/
int nbuf; /*dimensão do buffer*/
int nbloco; /*dimensão do bloco*/
int nib; /*número de itens no buffer*/
pthread_mutex_t mtx_buf; /*mutex para as variáveis buf e nib*/
pthread_mutex_t mtx_nv; /*mutex para a variável nv*/

int main(int argc, char *argv[])
{
	int i; /*iterador*/
	int r; /*retorno da função pthread_create*/
	int argValidos = 1; /*flag para validar argumentos passados na linha de comandos*/
	int max; /*máximo global*/
	int maxTarefa; /*máximo de sub-tarefa*/
	pthread_attr_t atribTarefa; /*atributo de sub-tarefa*/
	pthread_t *idTarefa; /*vetor de identificadores de sub-tarefas produtoras*/
	int *argTarefa; /*vetor de argumentos de sub-tarefas produtoras*/
	int nItCons; /*número de itens a consumir pela tarefa consumidora*/
	
	/*testar argumentos da linha de comandos*/
	if (argc!=4)
		argValidos = 0;
	else
	{
		nt = atoi(argv[1]);
		if (nt < 1)
			argValidos = 0;
		nv = atoi(argv[2]);
		if (nv < 1)
			argValidos = 0;
		nbloco = atoi(argv[3]);
		if (nbloco < 1)
			argValidos = 0;
	}

	/*teminar programa se os argumentos não são válidos*/
	if (!argValidos)
	{
		printf("\nUtilizacao: mtmax numtarefas dimvetor dimbloco\n\n");
		exit(1);
	}
	
	/*alocar memória para o vetor*/
	v = (int*) malloc (nv * sizeof(int));
	if (v == NULL)
	{
		printf("\nErro na alocacao de memoria\n\n");
		exit(1);
	}

	/*alocar memória para o buffer*/
	nbuf = DIMBUF;
	buf = (int*) malloc(nbuf * sizeof(int));
	if (buf == NULL)
	{
		printf("\nErro na alocacao de memoria\n\n");
		free(v);
		exit(1);
	}

	/*alocar memória para vetor de identificadores de tarefa*/
	idTarefa = (pthread_t *) malloc(nt * sizeof(pthread_t));
	if (idTarefa == NULL)
	{
		printf("\nErro na alocacao de memoria\n\n");
		free(v);
		free(buf);
		exit(1);
	}
	
	/*alocar memória para vetor de argumentos de tarefa*/
	argTarefa = (int *) malloc(nt * sizeof(int));
	if (argTarefa == NULL)
	{
		printf("\nErro na alocacao de memoria\n\n");
		free(v);
		free(buf);
		free(idTarefa);
		exit(1);
	}
	
	/*escrever mensagem inicial da tarefa principal (main)*/
	printf("\nMaximo de v[%d] com %d tarefas\n", nv, nt);

	/*inicializar gerador de números, número de itens no buffer, vetor, numero de itens a consumir*/
	srand(551);
	nib = 0;
	Inicializar(v,nv);
	nItCons = CalcularNItCons();

	/*inicializar objetos mutex*/
	pthread_mutex_init(&mtx_buf,NULL);
	pthread_mutex_init(&mtx_nv,NULL);

	/*inicializar variável de atributos com valores por defeito*/
	pthread_attr_init(&atribTarefa);
	/*modificar atributo estado de desacoplamento para "joinable"*/
	pthread_attr_setdetachstate(&atribTarefa, PTHREAD_CREATE_JOINABLE);

	/*criar as sub-tarefas produtoras*/
	for (i=0; i< nt; i++)
	{	
		/*atribuir à variável argTarefa o seu id*/
		argTarefa[i] = i;		
	
		/*criar e iniciar execução da sub-tarefa*/
		r = pthread_create(&idTarefa[i], &atribTarefa, TarefaProdutora, (void *) &argTarefa[i]);
		
		/*analisar se a sub-tarefa foi criada/iniciada com sucesso*/
		if (r)
		{
			printf("Erro ao criar a sub-tarefa produtora (%d)\n\n", argTarefa[i]);
			exit(1);
		}
	}
	
	/*tarefa consumidora - tarefa principal (main)*/	
	/*tentar ler do buffer, se não conseguir continua em espera ativa*/
	maxTarefa = 0;
	i = 0;
	while (i < nItCons)
	{
		/*proteger variáveis buf e nib*/
		pthread_mutex_lock(&mtx_buf);
		if (nib > 0)	
		{
			nib--;
			maxTarefa = buf[nib];
			i++;
		}
		/*desproteger variáveis buf e nib*/
		pthread_mutex_unlock(&mtx_buf);
		max = maxTarefa > max? maxTarefa : max;
	}

	/*esperar pelo final das sub-tarefas produtoras*/
	for (i=0; i<nt; i++)	
		pthread_join(idTarefa[i], (void **) NULL);

	/*escrever mensagem final da tarefa principal (main)*/
	printf("Maximo global = %d\n\n", max);

	/*destruir objetos mutex*/
	pthread_mutex_destroy(&mtx_buf);
	pthread_mutex_destroy(&mtx_nv);

	/*libertar memórias alocadas*/
	free(v);
	free(buf);
	free(idTarefa);
	free(argTarefa);
	
	return 0;
}

void Inicializar(int *v, int dim)
{
	int i;
	/*gerar um número aleatório e guardar em v[i]*/
	for (i=0; i<dim; i++)
		v[i] = rand() % (MAXINT +1);
}

int CalcularNItCons(void)
{
	int nItCons;	
	nItCons = nv /nbloco;
	/*se a divisão inteira não tem resto zero, adicionar 1*/
	if (nv % nbloco != 0)
		nItCons++;
	return nItCons;
}

int Maximo(int *v, int dim)
{	
	int i, max = v[0];
	for (i=1; i<dim; i++)
		max = max < v[i]? v[i] : max;
	return max; 
}

void * TarefaProdutora(void *arg)
{
	int *argTarefa = (int *) arg; /*converte e armazena o argumento passado para a sub-tarefa*/
	int indice; /*indice inicial do bloco do vetor a obter*/
	int dim; /*dimensão do bloco do vetor a obter*/
	int maxTarefa; /*máximo local*/
	int escrBuf; /*flag que verifica se já escreveu no buffer*/
	int obtBloco; /*flag que verifica se obteve bloco*/
	
	while (1)
	{
		obtBloco = 0;
		/*obter um bloco do vetor*/
		/*proteger variável nv*/
		pthread_mutex_lock(&mtx_nv);
		if (nv > 0)
		{		
			if (nv - nbloco >= 0)
			{
				dim = nbloco;
				nv = nv - dim;
			}
			else
			{
				dim = nv;
				nv = 0;
			}
			indice = nv;
			obtBloco = 1;
		}
		/*desproteger variável nv*/
		pthread_mutex_unlock(&mtx_nv);
		
		if (!obtBloco)
			break;	

		/*calcular o máximo*/
		maxTarefa = Maximo(&v[indice], dim);

		/*tentar escrever no buffer, se não conseguir continua em espera ativa*/
		escrBuf = 0;
		while (!escrBuf)
		{
			/*proteger variáveis buf e nib*/
			pthread_mutex_lock(&mtx_buf);
			if (nib < nbuf)	
			{
				buf[nib] = maxTarefa;
				nib++;
				escrBuf = 1;
			}
			/*desproteger variáveis buf e nib*/
			pthread_mutex_unlock(&mtx_buf);
		}
	
		/*escrever mensagem da sub-tarefa produtora*/
		printf("(%d) max = %d\n", *argTarefa, maxTarefa);
	}
	
	return (void*) NULL;
}
