/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// Network topology
//
//           10Mb/s, 10ms       10Mb/s, 10ms
//       n0-----------------n1-----------------n2
//
//
// - Tracing of queues and packet receptions to file
//   "tcp-large-transfer.tr"
// - pcap traces also generated in the following files
//   "tcp-large-transfer-$n-$i.pcap" where n and i represent node and interface
// numbers respectively
//  Usage (e.g.): ./waf --run tcp-large-transfer

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpLargeTransfer");

// The number of bytes to send in this simulation.
static const uint32_t totalTxBytes = 2000000;
static uint32_t currentTxBytes = 0;
// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
static const uint32_t writeSize = 1040;
uint8_t data[writeSize];

// These are for starting the writing process, and handling the sending
// socket's notification upcalls (events).  These two together more or less
// implement a sending "Application", although not a proper ns3::Application
// subclass.

void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);

/*
static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  NS_LOG_INFO ("Moving cwnd from " << oldval << " to " << newval);
}
*/

//Codigo adicional para coded caching

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
void genera_partes(int N, int F, int u, std::string *archivo, std::string **trozo)
{
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

//hasta aqui el codigo para coded caching

int main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //  LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  //  LogComponentEnable("TcpSocketImpl", LOG_LEVEL_ALL);
  //  LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
  //  LogComponentEnable("TcpLargeTransfer", LOG_LEVEL_ALL);

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // initialize the tx buffer.
  for(uint32_t i = 0; i < writeSize; ++i)
    {
      char m = toascii (97 + i % 26);
      data[i] = m;
    }


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

//genera N archivos con cadenas aleatorias de F bytes
std::string archivo[N];
genera_archivos(N, F, archivo);

//Vector para almacenar las solicitudes de cada usuario
int solicitud[K];

//Vector para almacenar el trozo que se tiene en la cache inicial por cada archivo
int demanda[N];

//generamos el vector de mapeo con los indices de entrega para el vector W
//printf("............\n");
int vector_comb[100];
comb_vector(K,K*Y, vector_comb);
//for (int i=0;i<D;i++)
//    printf("Comb[%d] = %d\n",i,vector_comb[i]);//muestra el vector de combinaciones

//V2
//todoloque sigue es para el modelo original de coded caching
//Generar los trozos a enviar cada uno de u bytes, vector de demanda
for (uint32_t n = 0; n < N; n++)    //por cada archivo
{
	uint32_t j = 0;
	std::string cad;
	int u = F / D;  //tamaño de cada trozo o subarchivo
	for (uint32_t i = 0; i < F; i=i+u)   //recortamos un trozo de tamaño F
	{
		if (F-i<u)
		{
			cad = archivo[n].substr(i,F-i);
			cad.append (u-F+i,'0');
		}
		else
			cad = archivo[n].substr(i,u);
		trozo[n][j] = cad;
		//printf ("Trozo W%d,[%d]: %s\n",n+1,vector_comb[j],trozo[n][j].c_str());//muestra los trozos obtenidos
		j++;
	}
}

//Etapa de Placement

//cada usuario k ha de solicitar una copia (caching) aleatoria inicial de los archivos
//Cada usuario k debe tener en su cache un trozo aleatorio de cada archivo
cache_inicial0 (trozo, K, N, D, cache3d_, vector_demanda,demanda,vector_comb);


//Muestra las copias de cada usuario.
Muestra_cache3d(cache3d_, vector_demanda, vector_comb,K, N, D);

// etapa de delivery

//Generación de combinaciones para la entrega
int Q;
int long_Q = K*Y+1;
Q = nCr(K,K*Y+1);
//cout << "Q: "<<Q<<"\n";//muestra la variable Q

//generación de las combinaciones para el delivery
int vector_delivery[100];
comb_vector(K,long_Q, vector_delivery);

//for (int i=0;i<Q;i++)
//    printf("Comb[%d] = %d\n",i,vector_delivery[i]);//muetra el vector de combinaciones

//convierte int a char
char elementoQ[Q][long_Q];
for (int q=0;q<Q;q++)
{
    sprintf(elementoQ[q],"%d", vector_delivery[q]);
    //printf("%s\n",elementoQ[q]);//muestra la conversion
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
        //printf("%c\n",elementoQ[n][q]);//muestra
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
        //muestra el vector X a enviar
        //printf("Vector X[%d][%d]: W[%c][%d]= %s\n",vector_delivery[n],q+1,elementoQ[n][q],pos_indice,trozo[archivo_id-1][parte].c_str());

        //convierte un sring a char[]
        strcpy(char_array, trozo[archivo_id-1][parte].c_str());
        //realiza el proceso XOR byte a byte
        for (int i = 0; i < F/D; i++)
        {
            vector_char[i] = vector_char[i] ^ char_array[i];
            //printf("%c",vector_char[i]);
        }
        printf("\n");
    }
    //generamos el vector X de delivery final
    vector_string[n]=vector_char;
    //printf("El vector X[%d]=%s\n",vector_delivery[n],vector_string[n]);
    //inicializa el vector de entrega
    for (int i = 0; i < D; i++)
    vector_char[i] = '0';
}

//hasta aqui el coded caching

  // Here, we will explicitly create three nodes.  The first container contains
  // nodes 0 and 1 from the diagram above, and the second one contains nodes
  // 1 and 2.  This reflects the channel connectivity, and will be used to
  // install the network interfaces and connect them with a channel.
  NodeContainer n0n1;
  n0n1.Create (2);

  // We create the channels first without any IP addressing information
  // First make and configure the helper, so that it will put the appropriate
  // attributes on the network interfaces and channels we are about to install.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (10000000)));
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));

  // And then install devices and channels connecting our topology.
  NetDeviceContainer dev0 = p2p.Install (n0n1);

//agregamos los nodos adicionales
  uint32_t nroNodos = 8;
  NodeContainer n1nX[nroNodos];
  NetDeviceContainer devX[nroNodos];
//creamos varios canales punto a punto
  for (uint32_t n = 0; n < nroNodos; n++)
    {
    n1nX[n].Add (n0n1.Get (1));
    n1nX[n].Create (1);
    devX[n] = p2p.Install (n1nX[n]);
    }

  // Now add ip/tcp stack to all nodes.
  InternetStackHelper internet;
  internet.InstallAll ();

  // Later, we add IP addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
//  Ipv4InterfaceContainer ipInterfs = ipv4.Assign (dev0);
	ipv4.Assign (dev0);
//creamos varios canales punto a punto
Ipv4InterfaceContainer ipInterfs[nroNodos];
  for (uint32_t n = 0; n < nroNodos; n++)
    {
  char ip[16];
  sprintf(ip,"10.2.%d.0",n + 1);
  ipv4.SetBase (Ipv4Address(ip), "255.255.255.0");
  //ipv4.Assign (devX[n]);
  ipInterfs[n] = ipv4.Assign (devX[n]);
    }

  // and setup ip routing tables to get total ip-level connectivity.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  ///////////////////////////////////////////////////////////////////////////
  // Simulation 1
  //
  // Send 2000000 bytes over a connection to server port 50000 at time 0
  // Should observe SYN exchange, a lot of data segments and ACKS, and FIN
  // exchange.  FIN exchange isn't quite compliant with TCP spec (see release
  // notes for more info)
  //
  ///////////////////////////////////////////////////////////////////////////

  uint16_t servPort = 50000;
/*
  // Create a packet sink to receive these packets on n0...
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), servPort));
  ApplicationContainer apps = sink.Install (n0n1.Get (0));
//ApplicationContainer apps = sink.Install (n1n2.Get (1));//original
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (3.0));
*/
//creamos el socket
Ptr<Socket> localSocket;
//creamos el origen de los datos en el nodo n0...
  localSocket =
    Socket::CreateSocket (n0n1.Get (0), TcpSocketFactory::GetTypeId ());
  localSocket->Bind ();



  // Create a source to send packets from n0.  Instead of a full Application
  // and the helper APIs you might see in other example files, this example
  // will use sockets directly and register some socket callbacks as a sending
  // "Application".

  // Create and bind the socket...
//creamos varios sockets. Activar/desactivar las variables
ApplicationContainer apps[nroNodos];
//Ptr<Socket> localSocketX[nroNodos];
  for (uint32_t n = 0; n < nroNodos; n++)
    {
/*
// creamos los origenes de los datos en los nodos creados
//  localSocketX[n] =
//    Socket::CreateSocket (n1nX[n].Get (1), TcpSocketFactory::GetTypeId ());
//  localSocketX[n]->Bind ();

  // One can toggle the comment for the following line on or off to see the
  // effects of finite send buffer modelling.  One can also change the size of
  // said buffer.

  //localSocketX[n]->SetAttribute("SndBufSize", UintegerValue(4096));
*/
//creamos los destinos de los datos en los nodos creados
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), servPort));

   apps[n] = sink.Install (n1nX[n].Get (1));

  apps[n].Start (Seconds (0.0));
  apps[n].Stop (Seconds (3.0));

    }



//  // Trace changes to the congestion window
//  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));

  // ...and schedule the sending "Application"; This is similar to what an
  // ns3::Application subclass would do internally.

//transmitimos a cada nodo
  for (uint32_t n = 0; n < nroNodos; n++)
    {
/*
// crea la transmision en el nodo 0
  Simulator::ScheduleNow (&StartFlow, localSocketX[n],
                          ipInterfs.GetAddress (0), servPort);
*/
//crea la transmision en los nodos creados
  Simulator::ScheduleNow (&StartFlow, localSocket,
                          ipInterfs[n].GetAddress (1), servPort);
    }

  //Ask for ASCII and pcap traces of network traffic
  AsciiTraceHelper ascii;
//  p2p.EnableAsciiAll (ascii.CreateFileStream ("tcp-large-transfer.tr"));
//  p2p.EnablePcapAll ("tcp-large-transfer");

  // Finally, set up the simulator to run.  The 1000 second hard limit is a
  // failsafe in case some change above causes the simulation to never end
  Simulator::Stop (Seconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//begin implementation of sending "Application"
void StartFlow (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
  NS_LOG_LOGIC ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
  localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect

  // tell the tcp implementation to call WriteUntilBufferFull again
  // if we blocked and new tx buffer space becomes available
  localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
  WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
  while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
    {
      uint32_t left = totalTxBytes - currentTxBytes;
      uint32_t dataOffset = currentTxBytes % writeSize;
      uint32_t toWrite = writeSize - dataOffset;
      toWrite = std::min (toWrite, left);
      toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
      int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
      if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }
      currentTxBytes += amountSent;
    }
  localSocket->Close ();
}


