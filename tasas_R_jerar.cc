#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <limits>

//Librerias para Windows
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

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
//float M = 2.0;
int M = 2;
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
//printf ("Y=%f/%d=%f\n",M,N,Y);

//establecer el valor de K*Y
float KY=K*Y;

//cantidad de trozos a generar por archivo debiera ser D
int D;
D = nCr(K,K*Y);
//cout << "D: "<<D<<"\n";

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
/*
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
*/
char namefile[512];
FILE *archivo;
float r_jerarquico,r_jerarquico_1,r_jerarquico_2,r_jerarquicoA_1,r_jerarquicoA_2,r_compartido,r_compartido_gral,R1,R2;
float r_inicial,r_inicial_dec;
float k=10.0;
float n=8.0;
float l=10.0;
float m1, m2,a,b;
float k1, k2;
k1=8;
k2=1;
m1=0;
m2=n;
sprintf(namefile,"%s%d.%d.%d-%d-%d%s","tabla_comparativa_jerar-M.",(int)n,(int)k,(int)l,(int)k1,(int)k2,".csv");
archivo=fopen(namefile,"w");
//printf ("M      r(inicial)      r(decent)       delta       r(compartido)       delta\n---------------------------------------------------------------\n");
fprintf (archivo,"M1;M2;K1;K2;a;b;r1(a,b);r2(a,b)\n");
//fprintf (archivo,"0;%f;%f;0;%f;0\n",n,n,n);
//hierarchival coded caching
//for (int m=1;m<=n;m++)
for (int x=1;x<=n;x++)
    for (int y=1;y<=n;y++)
    //for (int n=1;n<=N;n++)
     //   for (int k=1;k<=K;k++)
        {
            //float a=x/10.0;
            //float b=y/10.0;
m1=(float)x;
m2=(float)y;
            //r_inicial = k*(1.0-(m/n))*(1/(1+(k*m/n)));//este es el modelo general centralizado
            //r_inicial_dec = k*(1.0-(m/n))*(n/(k*m))*(1.0-pow(1.0-(m/n),k));//este es el modelo general decentralizado

            //r_jerarquico = k*(1.0-(m/n))*(n/(k*m))*(1.0-pow(1.0-(m/n),k));//este es el modelo general de una capa
            //r_jerarquicoA_1 = k2*(1.0-(m1/n))*(n/(m1))*(1.0-pow(1.0-(m1/n),k1));//este es el esquema A para R1
            //r_jerarquicoA_2 = (1.0-(m2/n))*(n/(m2))*(1.0-pow(1.0-(m2/n),k2));//este es el esquema A para R2

if ((m1+(m2*k2)>=n) && ((0<=m1) && (m1<=n/4))) //Regimen I
{
    R1=fmin(k1*k2,n/m2);
    a=m1/n;
    b=m1/n;
}
else if (m1+(m2*k2)<n)   //Regimen II
{
    R1=fmin(k1*k2,(m1/(m1+(m2*k2)))*(((n-m1)*k2)/(m1+(m2*k2)))+((m2*k2)/(m1+(m2*k2)))*(((n*k2)-m1)/(m1+(m2*k2))));
    a=m1/(m1+(m2*k2));
    b=0;
}
else if ((m1+(m2*k2)>=n) && ((n/4<m1) && (m1<=n)))   //Regimen III
{
    R1=(4*pow(n-m1,2))/(3*n*m2);
    a=m1/n;
    b=1.0/4.0;
}
R2=4*fmin(k2,n/m2);

            r_jerarquico_1 = (a*k2)*((a*n-m1)/m1)*(1.0-pow(1.0-(m1/(a*n)),k1))+(1.0-a)*((((1.0-a)*n)-((1.0-b)*m2))/((1.0-b)*m2))*(1.0-pow(1.0-(((1.0-b)*m2)/((1.0-a)*n)),k1*k2));
            r_jerarquico_2 = a*((a*n-b*m2)/(b*m2))*(1.0-pow(1.0-((b*m2)/(a*n)),k2))+(1.0-a)*((((1.0-a)*n)-((1.0-b)*m2))/((1.0-b)*m2))*(1.0-pow(1.0-(((1.0-b)*m2)/((1.0-a)*n)),k2));

            //r_compartido = k*(1.0-(l*m/n))*(n/(k*m))*(1.0-pow(1.0-(l*m/n),k/l));//este es el modelo de caches compartidas
            //r_compartido_gral = ((n-m)/m)*(k/l)*(1.0-pow(1.0-(m/n),l));//este es el modelo de caches compartidas general
            //float factor = r_inicial_dec - r_inicial;
            //float factor2 = r_compartido_gral - r_inicial;
            //float razon1 = r_inicial_dec / r_inicial;
            //float razon2 = r_compartido_gral / r_inicial;
            //printf("%d      %f      %f : %f |      %f : %f\n",m,r_inicial,r_inicial_dec,factor,r_compartido_gral,factor2);
            //fprintf(archivo,"%d;%f;%f;%f;%f;%f\n",m,r_inicial,r_inicial_dec,factor,r_compartido_gral,factor2);
            //fprintf(archivo,"%f;%f;%f;%f;%f;%f;%f;%f\n",m1,m2,k1,k2,a,b,r_jerarquico_1,r_jerarquico_2);
            fprintf(archivo,"%f;%f;%f;%f;%f;%f;%f;%f\n",m1,m2,k1,k2,a,b,R1,R2);
        }



fclose(archivo);


}
