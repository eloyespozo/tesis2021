#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

//Librerias para Windows
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/* //Habilitar para el funcionamiento de NS3 en Linux
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;
*/

// The number of bytes to send in this simulation.
//static const uint32_t totalTxBytes = 2000000;
//static uint32_t currentTxBytes = 0;
// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
//static const uint32_t writeSize = 1040;
//uint8_t data[writeSize];

// These are for starting the writing process, and handling the sending
// socket's notification upcalls (events).  These two together more or less
// implement a sending "Application", although not a proper ns3::Application
// subclass.

//funcion que genera una cadena aleatoria
void strGetRandomAlphaNum(char *sStr, unsigned int iLen)
{
  char Syms[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  unsigned int Ind = 0;
  srand(time(NULL) + rand());
  while(Ind < iLen)
  {
    sStr[Ind++] = Syms[rand()%62];
  }
  sStr[iLen] = '\0';
}

//Funcion que muestra las copias de cada usuario. Debera volverse funcion
void Muestra(std::string **cadena, int K,int D)
{
   	for (int k = 0; k < K; k++)
	{
		for (int d = 0; d < D; d++)
			printf ("Cache usuario (%d,%d): %s\n",k,d,cadena[k][d].c_str());
	}
}

//Funcion que muestra las copias de cada usuario. Debera volverse funcion
void Muestra_indices(std::string **cadena, int **demanda, int K,int N, int D)
{
   	for (int k = 0; k < K; k++)
	{
	for (int d = 0; d < D; d++)
		printf ("Cache usuario %d: Archivo(%d,%d): %s\n",k, demanda[k][0],d,cadena[k][d].c_str());
	}
}

//Funcion que muestra la cache de cada usuario.
void Muestra_cache(std::string **cadena, int **demanda, int K,int N, int D)
{
   	for (uint32_t k = 0; k < K; k++)
	{
	for (uint32_t n = 0; n < N; n++)
		printf ("Cache usuario %d: Archivo %d, trozo %d: %s\n",k, n, demanda[k][n],cadena[k][n].c_str());
	}
}

//Funcion que muestra la cache de cada usuario.
void Muestra_cache3d(std::string ***cadena, int **demanda, int *vector_comb, int K,int N, int D)
{
   	for (uint32_t k = 0; k < K; k++)
    {
        for (uint32_t f = 0; f < N; f++)
        {
            for (uint32_t n = 0; n < D; n++)
                printf ("Cache usuario %d: Archivo %d, trozo %d[%d]: %s\n",k, f, demanda[k][f], vector_comb[n],cadena[k][f][n].c_str());
        }
    }
}

//Funcion para generar las copias (caching) iniciales de los archivos por cada usuario (ya no va!!!)
void demanda_inicial (std::string **archivo, int K, int N, int D, std::string **pedazo, int **demanda)
{
	int f = rand() % N;//elige el archivo al azar
	int i = rand() % D;//elige el trozo al azar
	pedazo[K][i] = archivo[f][i];//genera un vector con el trozo obtenido
	demanda[K][0] = f;//registramos el archivo solicitado
	demanda[K][1] = i;//registramos el trozo inicial enviado
}

//Funcion para generar las copias (caching) iniciales de los archivos por cada usuario
//cada usuario debe tener un trozo aleatorio de cada arhcivo en su cache inicial
// Zi=(Wi,Wi,...,Wi)
void cache_inicial (std::string **archivo, int K, int N, int D, std::string **cache, int **mat_demanda, int *demanda)
{
	int i = rand() % D;//elige el trozo al azar
	for (int f = 0; f < N; f++)     //Por cada archivo
	{
		cache[K][f] = archivo[f][i];//genera un vector con el trozo obtenido
		//demanda[K][0] = f;//registramos el archivo solicitado
		mat_demanda[K][f] = i;//registramos el trozo inicial enviado en matriz
		demanda[K] = i;//registramos el trozo inicial enviado en vector
	}
}

//Funcion para generar las copias (caching) iniciales de los archivos por cada usuario
//cada usuario debe tener un trozo elegido de cada arhcivo en su cache inicial
// Zi=(Wi,Wi,...,Wi)
void cache_inicial0 (std::string **archivo, int K, int N, int D, std::string ***cacheZ, int **mat_demanda, int *demanda, int *vector_comb)
{
	for (int k=0; k<K; k++)  //por cada usuario existente
    {
        for (int f = 0; f < N; f++)     //por cada archivo del repositorio
        {
            for (int n=0; n<D; n++)     //por cada indice de un arhivo
            {
                std::string indiceT = std::to_string(vector_comb[n]);
                if (indiceT.find(std::to_string(k+1)) != std::string::npos)
                {
                    cacheZ[k][f][n] = archivo[f][n];//genera un vector con el trozo obtenido
                    //ojo revisar lo siguiente
                    mat_demanda[k][f] = n;//registramos el trozo inicial enviado en matriz
                    demanda[k] = n;//registramos el trozo inicial enviado en vector
                }
            }
        }
    }
}

/*
//funcion que opera con XOR byte a byte dos cadenas de texto
void genera_xor(char *value1, char *value2, char **xored, int LENGTH) {
  for(int i=0; i<LENGTH; ++i) {
    xored[i] = (char)(value1[i] ^ value2[i]);
  }
}
*/

//Funcion para generar las copias de los archivos que se enviarán a todos los usuarios (multicast)
void cache_entrega (std::string **archivo, int K, int N, int Q, std::string **envioX, int **mat_demanda, int *demanda, int *vector_delivery)
{
	int i=0;
	for (int k=0; k<K; k++)  //por cada usuario existente
    {
        for (int f=0; f<N; f++)     //por cada archivo del repositorio
        {
            for (int n=0; n<Q; n++)     //por cada indice de un arhivo
            {
                std::string indiceK = std::to_string(vector_delivery[n]);
                if (indiceK.find(std::to_string(k+1)) == std::string::npos)
                {
                    envioX[k][i] = archivo[f][n];//genera un vector con el trozo obtenido
                    i++;

                    //ojo revisar lo siguiente
                    //mat_demanda[k][f] = n;//registramos el trozo inicial enviado en matriz
                    //demanda[k] = n;//registramos el trozo inicial enviado en vector
                }

            }
        }
    }
}

void genera_archivos(int N, int F, std::string *archivo)
{
//genera N archivos con cadenas aleatorias de F bytes
//std::string archivo[N];
for (int n = 0; n < N; n++)
{
	char cadena[F];
	strGetRandomAlphaNum (cadena, F);
	archivo[n] = cadena;
	printf ("%s \n",archivo[n].c_str()); //este printf debiera quitarse luego
}
}

//Funcion que separa los arhcivos en cadenas sueltas
void genera_partes(int N, int F, int u, std::string *archivo, int *vector_comb,std::string **trozo)
{
//Generar los trozos a enviar cada uno de u bytes, vector de demanda
for (int n = 0; n < N; n++)    //por cada archivo
{
	int j = 0;
	std::string cad;
	for (int i = 0; i < F; i=i+u)   //recortamos un trozo de tamaño F
	{
		if (F-i<u)
		{
			cad = archivo[n].substr(i,F-i);   //esto hace un padding si es necesario
			cad.append (u-F+i,'0');
		}
		else
			cad = archivo[n].substr(i,u);
		trozo[n][j] = cad;
		printf ("Trozo W%d,[%d]: %s\n",n+1,vector_comb[j],trozo[n][j].c_str());
		j++;
	}
}
}




/*a continuación combinatorias*/
void comb(int m, int n, char *c)
//Esta función generala combinatoria de dos numeros y las meustra en una buffer
//su uso:
//unsigned char buf[100];
//comb(5, 3, buf);
{
	int i;
	for (i = 0; i < n; i++) c[i] = n - i;

	while (1) {
		for (i = n; i--;)
			printf("%d%c", c[i], i ? ' ': '\n');

		/* this check is not strictly necessary, but if m is not close to n,
		   it makes the whole thing quite a bit faster */
		i = 0;
		if (c[i]++ < m) continue;

		for (; c[i] >= m - i;) if (++i >= n) return;
		for (c[i]++; i; i--) c[i-1] = c[i] + 1;
	}
}

void comb_pos(int m, int n, char *c,int **matriz_Q)
//Esta función arma un vector con la combinatoria de dos numeros y las meustra en una buffer
//su uso:
//unsigned char buf[100];
//comb_pos(5, 3, buf);
{
//    unsigned int vector_pos[] = {1,4,6,8};
	int i;
	int k = 0;
	//int matriz_Q[][];
	int temp;
	for (i = 0; i < n; i++) c[i] = n - i;

	while (1) {
		for (i = n; i--;)
        {
			printf("%d%c", c[i], i ? ' ': '\n');
			temp=(int)c[i];
			//printf("%d%c", vector_pos[temp-1], i ? ' ': '\n');
			matriz_Q[k][i]=temp;
        }
        k++;
		/* this check is not strictly necessary, but if m is not close to n,
		   it makes the whole thing quite a bit faster */
		i = 0;
		if (c[i]++ < m) continue;

		for (; c[i] >= m - i;) if (++i >= n) return;
		for (c[i]++; i; i--) c[i-1] = c[i] + 1;
	}
}

//Esta función arma un vector con las combinatorias de dos numeros que luego servira para el mapeo
void comb_vector(int m, int n, int *vector_comb)
//Esta función arma un vector con la combinatoria de dos numeros y las meustra en una buffer
//su uso:
//unsigned char buf[100];
//comb_pos(5, 3, buf);
{
	int i;
	int k = 0;
	char c[100];
	std::string temp="";
	for (i = 0; i < n; i++) c[i] = n - i;

	while (1) {

		for (i = n; i--;)
        {
			temp = temp + std::to_string(c[i]);
        }
        vector_comb[k]=stoi(temp);
        //vector_comb[k]=temp;
        temp="";
        k++;


		/* this check is not strictly necessary, but if m is not close to n,
		   it makes the whole thing quite a bit faster */
		i = 0;
		if (c[i]++ < m) continue;

		for (; c[i] >= m - i;) if (++i >= n) return;
		for (c[i]++; i; i--) c[i-1] = c[i] + 1;
	}
}

// Retorna el factorial de n
int fact(int n)
{
    int res = 1;
    for (int i = 2; i <= n; i++)
        res = res * i;
    return res;
}

//Retorna el resutlado de nCr
int nCr(int n, int r)
{
    return fact(n) / (fact(r) * fact(n - r));
}


//rvisar lo siguietne, sin uso
void realizaXOR(char vector_string[], int len)
{
    // Define XOR key
    // Any character value will work
    char xorKey = '0';

    // calculate length of input string
//    int len = strlen(vector_string);

    // perform XOR operation of key
    // with every caracter in string
    for (int i = 0; i < len; i++)
    {
        vector_string[i] = vector_string[i] ^ xorKey;
        printf("%c",vector_string[i]);
    }
}

/*hasta aqui*/

using namespace std;
int main (int argc, char *argv[])
{

//comienzo del envio de cache codficado

//definición de variables
//definir la cantida de archivos en la libreria (N)
int N = 4;
//definir el tamano de los archivos en la libreria (F) bytes (bits)
int F = 60;
//definir la cantidad de usuarios (K)
int K = 4;
//establecer el espacio de almacenamiento en la cache en cada nodo edge (M)
float M = 2.0;
//establecer el tamano de cache fraccional u=MF/N
int u = M * F / N;
//establecer el tamano de la cache en cada nodo edge (V)
//uint32_t V = u * N * F;

//cantidad de trozos a generar por archivo // ojo. verificar .....
//int D;   // D = F / u;
//D = (F % u == 0 ) ? F/u : (F/u)+1;

//estableceer el valor de la fracción Y de cada archivo (tamaño)
float Y;
Y = M / N;
printf ("Y=%f/%d=%f\n",M,N,Y);

//establecer el valor de K*Y
float KY=K*Y;

//cantidad de trozos a generar por archivo debiera ser D
int D;
D = nCr(K,K*Y);
cout << "D: "<<D<<"\n";

/*
//estas variables se usan en shared caching
//establece la cantidad de Cache Helpers (V invertida)
int V = 4;
float VY= V*Y;
//establecer la cantidad de subfiles en que se divide cada archivo
//WnT = combinatoria de V y V*Y
//equivale a D
int W;
W = nCr(V,V*Y);
printf ("%d\n%f\n%d\n",V,VY,W);
D=W;
//se establece el tamaaño de cada subarchivo
u = F / W ;

//estas variables se usan en la parte de delivery
//Para cada ronda se deben crear H conjuntos Q incluidos en [V] (todas las combinaciones)
//H = combinatoria de V y V*Y+1
int H = nCr(V,V*Y+1);
//Cada conjunto Q debe ser de tamaño V*Y+1
int Tam_q = V*Y+1;
*/
//fin variables

//Repetimos el modelo MAN
//Se define Matriz para almacenar los trozos de cada archivo
std::string **trozo;     //std::string trozo[N][D];
trozo = new std::string *[N];
for(int i = 0; i < N; i++)
    trozo[i] = new std::string[D];
//Se define Matriz con los índices de demanda
int **vector_demanda;     //int vector_demanda[K][2];
vector_demanda = new int *[K];
for(int i = 0; i < K; i++)
    vector_demanda[i] = new int[2];
//Se define Matriz para la cache de datos
std::string **cache_;     //std::string cache_[K][D];
cache_ = new std::string *[K];
for(int i = 0; i < K; i++)
    cache_[i] = new std::string[D];

//Se define Matriz 3Dimensional para la cache de datos
std::string ***cache3d_;     //std::string cache_[K][F][D];
cache3d_ = new std::string **[K];
for(int x = 0; x < K; x++) {
    cache3d_[x] = new std::string *[F];
    for(int y = 0; y < F; y++) {
        cache3d_[x][y] = new std::string [D];
        for (int z = 0; z < D; z++) {
            cache3d_[x][y][z] = "";
        }
    }
}
//ejemplode inicializacion de matriz 3d
/*
int*** array;    // 3D array definition;
// begin memory allocation
array = new int**[dimX];
for(int x = 0; x < dimX; ++x) {
    array[x] = new int*[dimY];
    for(int y = 0; y < dimY; ++y) {
        array[x][y] = new int[dimZ];
        for(int z = 0; z < dimZ; ++z) { // initialize the values to whatever you want the default to be
            array[x][y][z] = 0;
        }
    }
}
*/


//genera N archivos con cadenas aleatorias de F bytes
std::string archivo[N];
genera_archivos(N, F, archivo);

//Vector para almacenar las solicitudes de cada usuario
int solicitud[K];

//Vector para almacenar el trozo que se tiene en la cache inicial por cada archivo
int demanda[N];

//generamos el vector de mapeo con los indices de entrega para el vector W
printf("............\n");
int vector_comb[100];
comb_vector(K,K*Y, vector_comb);
for (int i=0;i<D;i++)
{
    printf("Comb[%d] = %d\n",i,vector_comb[i]);
}

//Generar los trozos a enviar cada uno de u bytes, vector de demanda
genera_partes(N,F,u,archivo,vector_comb,trozo);

//Etapa de Placement - Shared Cache

//cada usuario k ha de solicitar una copia (caching) aleatoria inicial de los archivos
//Cada usuario k debe tener en su cache un trozo aleatorio de cada archivo
cache_inicial0 (trozo, K, N, D, cache3d_, vector_demanda,demanda,vector_comb);


//Muestra las copias de cada usuario.
Muestra_cache3d(cache3d_, vector_demanda, vector_comb,K, N, D);

// etapa de delivery - algoritmo shred cache

//Generación de combinaciones para la entrega
int Q;
int long_Q = K*Y+1;
Q = nCr(K,K*Y+1);
cout << "Q: "<<Q<<"\n";

//generación de las combinaciones para el delivery
int vector_delivery[100];
comb_vector(K,long_Q, vector_delivery);
for (int i=0;i<Q;i++)
{
    printf("Comb[%d] = %d\n",i,vector_delivery[i]);
}

//convierte string a char
/*char* vector_Q_char;
string str_obj("GeeksForGeeks");
vector_Q_char = &str_obj[0];
//cout << char_arr;
*/

//convierte int a char
char elementoQ[Q][long_Q];
for (int q=0;q<Q;q++)
{
    sprintf(elementoQ[q],"%d", vector_delivery[q]);
    printf("%s\n",elementoQ[q]);
}

//inicializa el vector de envío delivery
char char_array[F/D];
char vector_char[F/D];
string vector_string[Q];
for (int i = 0; i < D; i++)
    vector_char[i] = '0';

//cache_entrega(trozo, K, N, Q, cache_, vector_demanda,demanda,vector_delivery);
for (int n=0; n<Q; n++)     //por cada indice de un arhivo
{
    for (int q=0;q<long_Q;q++)
    {
        std::string indiceQ = std::to_string(vector_delivery[n]);
        printf("%c\n",elementoQ[n][q]);
        indiceQ.erase(std::remove(indiceQ.begin(),indiceQ.end(),elementoQ[n][q]),indiceQ.end());
        int pos_indice=stoi(indiceQ);
        int parte;
        int archivo_id=elementoQ[n][q]-'0';
        for (int i=0; i<D;i++)
        {
            if(pos_indice==vector_comb[i])
            {
                parte=i;
                break;
            }
        }
        //envioX[k][i] = archivo[f][n];//genera un vector con el trozo obtenido
        printf("Vector X[%d][%d]: W[%c][%d]= %s\n",vector_delivery[n],q+1,elementoQ[n][q],pos_indice,trozo[archivo_id-1][parte].c_str());

        //convierte un sring a char[]
        strcpy(char_array, trozo[archivo_id-1][parte].c_str());
        //realiza el proceso XOR byte a byte
        for (int i = 0; i < F/D; i++)
        {
            vector_char[i] = vector_char[i] ^ char_array[i];
            printf("%c",vector_char[i]);
        }
        printf("\n");
    }
    //generamos el vector X de delivery final
    vector_string[n]=vector_char;
    printf("El vector X[%d]=%s\n",vector_delivery[n],vector_string[n]);
    //inicializa el vector de entrega
    for (int i = 0; i < D; i++)
    vector_char[i] = '0';
}
//fin de caching shared



//verificación del vector de delivery
//se verifica en la cache de cada usuario


//cada usuario ha de solicitar un archivo
//supondremos qeu usuario k1 pide file 1, k2 pide file 2, ......
//al final deberá ser aleatorio
//ojo revisar, todo mal.......
/*
for (int k = 0; k < K; k++) //por cada usuario
{

	solicitud[k] = rand() % D;//elige el archivo a solicitar al azar
	printf ("Usuario %d solicita archivo: %d\n",k,solicitud[k]);
}


//Etapa de Delivery
//envio de trozos de archivo faltantes por cache de usuario

for (int s = 0; s < K; s++)	//por cada usuario
{
	for (int j = 0; j < N; j++) //por cada archivo/solicitud
	{
		//if (vector_demanda[s][0] != j )
		if (demanda[s] != j )
		{
			printf ("Usuario %d archivo a enviar (%d,%d): %s\n",s,solicitud[s],j,trozo[solicitud[s]][j].c_str());
			//revisar lo siguiente .....
			cache_[solicitud[s]][j] = trozo[solicitud[s]][j];//rellena el vector con el trozo faltante
		}
	}
}


printf ("..............................\n");
//Muestra las copias de cada usuario.
//Muestra(cache_, K, D);
Muestra_cache(cache_, vector_demanda, K, N, D);
*/
//segundo intento
//hasta aqui lo del modelo origianlde coded caching




//lo que sigue corresponde al modelo de shared caching
/*
int T[V];
//char buf[100];
//comb(V, 2, buf);

//desde aqui la etapa Placement de modelo shared caching
//se muestran los trozos de cada archivo, copiado de más abajo
for (int n = 0; n < N; n++)
{
	int j = 0;
	std::string cad;
	for (int i = 0; i < F; i=i+u)
	{
		if (F-i<u)
		{
			cad = archivo[n].substr(i,F-i);
			cad.append (u-F+i,'0');
		}
		else
			cad = archivo[n].substr(i,u);
		trozo[n][j] = cad;
		printf ("trozo %d,%d: %s\n",n,j,trozo[n][j].c_str());
		j++;
	}
}


//esto para shared caches - delivery
//El vector L contiene las cardinalidades de U ordenados descendientemente
int L[] = {3,2,2,1};
//La matriz U contiene lel numero de usario asociados a las caches lambda
int U[sizeof(L)/sizeof(int)][K];
//lo siguiente genera el vetor de cardinalidades U
int m = 1;
for (int i=0;i<sizeof(L)/sizeof(int);i++)
	for (int k=0;k<L[i];k++)
	{
		U[i][k] = m;
		printf ("U[%d][%d] = %d\n",i+1,k+1,m);
		m++;
	}

//*040221
//El vector L contiene las cardinalidades de U ordenados descendentemente
//int L[] = {3,2,2,1};
//La matriz U contiene lel numero de usario asociados a las caches lambda
//int U[4][3] = {{1,2,3},{4,5},{6,7},{8}};
//R son las rondas con los respectivos usuarios a enviarse por cada ronda
int R[3][4];
//Siempre en base a L[1] se debe definir a que usuarios enviar en cada ronda
for (int j=0;j<L[0];j++)
{
    int lambda;
    for(lambda=0;lambda<V;lambda++)
    {
        if(L[lambda]>=j+1)
        {
            R[j][lambda]=U[lambda][j];
            printf("R[%d][%d] = %d\n",j+1,lambda+1,U[lambda][j]);
        }
    }
}
*/

//Ahora el esquema de transmision
//se deben crear conjuntos de tamaño
/* lo siguietne funciona pero no es necesario
char buf1[100];
int **matriz_Q;
comb_pos(V, Tam_q, buf1,matriz_Q);
*/

/*for (int k=0;k<H;k++)
{for (int x=0;x<Tam_q;x++)
     {
         printf("%d ",matriz_Q[k][x]);
     }
}*/

//hasta aqui el esquema de transmision


//hasta aqui lo del modelo shared caching



}




