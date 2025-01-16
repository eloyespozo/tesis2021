//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include <algorithm>
//#include <cstring>

//Librerias para Windows
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

//using namespace std;
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

float r;
printf ("M      N       K       r\n-------------------------------------------------\n");
for (int m=0;m<M;m++)
    for (int n=0;n<N;n++)
        for (int k=0;k<K;k++)
        {
            r=(k*(1-(m/n))*(n/(k*m))*(1-(1-(m/n))^k));
            printf("%d      %d      %d      %f\n",m,n,k,r);
        }

}
