#include "agent.h"
#include "environment.h"
#include <iostream>
#include <locale>
#include <cstdlib>
#include <vector>
#include <utility>
#include <omp.h>

using namespace std;

#define TAM_ESCENARIO 19

#define PINTAR_MAPA false
#define PINTAR_GRADIENTES false

#define ESTIMAR_VELOCIDAD true


template <class T>
void ImprimirMapa(Mapa<T>& mapa, double maximo=1.0) {
  for (int i=0; i<TAM_ESCENARIO; i++) {
    for (int j=0; j<TAM_ESCENARIO; j++) {
      int nivel= ((mapa(j,i) * 8.0) / maximo);
      nivel= (nivel>8) ? 8 : nivel;
      cout << "\033[31;47m";
      if (nivel==0)
        cout << " ";
      else if (nivel==1)
        cout << "\u2581";
      else if (nivel==2)
        cout << "\u2582";
      else if (nivel==3)
        cout << "\u2583";
      else if (nivel==4)
        cout << "\u2584";
      else if (nivel==5)
        cout << "\u2585";
      else if (nivel==6)
        cout << "\u2586";
      else if (nivel==7)
        cout << "\u2587";
      else if (nivel==8)
        cout << "\u2588";
      cout << "\033[0m";

    }
    cout << endl;
  }
  cout << endl;
}


void Agent::ImprimirMapaAgente() {
  cout << "Posicion: " << posx << ", " << posy << endl;
  cout << "Velocidad estimada: " << (velocidadEstimada/mediciones) << ", mediciones: " << mediciones << endl;
  cout << "DifXConocido: " << (maxXConocido-minXConocido) << " DifYConocido: " << (maxYConocido-minYConocido) << endl;
  for (int y=0; y<TAM_ESCENARIO; y++) {
    for (int x=0; x<TAM_ESCENARIO; x++) {
      
      // Pintamos el personaje
      if (x==posx && y==posy) {
        cout << "\033[32;47m";
        if (dir==0)
          cout << "\u25b2";
        else if(dir==1)
          cout << "\u25b6";
        else if(dir==2)
          cout << "\u25bc";
        else
          cout << "\u25c0";      
        cout << "\033[0m";
      }
      
      // Pintamos lo desconociiidooo....
      else if (mapaDesconocidos(x,y)) {
        cout << "\u2591";
      }

      // Pintamos muros
      else if (mapaObstaculos(x,y)) {
        cout << "\033[33;40m";
        cout << "\u2588";
        cout << "\033[0m";
      }

      // Pintamos huecos, con el grado de antiguedad que tengan
      else {
        int nivel= (int) mapaAntiguedad(x,y)/60.0;
        nivel= (nivel>8) ? 8 : nivel;
        cout << "\033[31;47m";
        if (nivel==0)
          cout << " ";
        else if (nivel==1)
          cout << "\u2581";
        else if (nivel==2)
          cout << "\u2582";
        else if (nivel==3)
          cout << "\u2583";
        else if (nivel==4)
          cout << "\u2584";
        else if (nivel==5)
          cout << "\u2585";
        else if (nivel==6)
          cout << "\u2586";
        else if (nivel==7)
          cout << "\u2587";
        else if (nivel==8)
          cout << "\u2588";
        cout << "\033[0m";
      }
    }
    cout << endl;
  }
  cout << endl;
}


// A partir de un mapa de valores doubles y de un mapa de obstaculos booleanos calcula el degradado
void CalcularGradiente(
  Mapa<double>& mapa, 
  Mapa<bool>& mapaObstaculos, 
  double dispersion=0.1, 
  int iteraciones=100 ) 
{
  Mapa<double> destino;
  double i8= 1.0/8.0;

  // Lo procesamos
  for (int iteracion=0; iteracion<iteraciones; iteracion++) { 

    // Copiamos el ´ultimo resultado a la matriz principal mapa.
    #pragma omp parallel for
    for (int i=0; i<TAM_ESCENARIO; i++) {
      for (int j=0; j<TAM_ESCENARIO; j++) {
        if (iteracion!=0)
          mapa(i,j)= destino(i,j);
        destino(i,j)= 0.0;
      }
    }

    // Iteramos por cada casilla (i,j)
    #pragma omp parallel for
    for (int i=0; i<TAM_ESCENARIO; i++) {
      for (int j=0; j<TAM_ESCENARIO; j++) {
        
        // Iteramos por las casillas vecinas de (i,j): (i+a,j+b), con a y b pertenecientes a {-1,0,1}.
        for (int a=-1; a<2; a++) {
          for (int b=-1; b<2; b++) {
            
            // Compruebo que la casilla est´a dentro de la matriz
            if ((i+a < 0) || (i+a > 18) || (j+b < 0) || (j+b > 18))
              continue;
            // No se permite procesar las esquinas
            if (a*a + b*b > 1)  
              continue;

            // Procesamos el desenfoque de la casilla (i,j)
            if (a==0 && b==0) {
              destino(i,j)+= mapa(i,j)*(1.0-(dispersion));
            }
            // Procesamos el desenfoque de la casilla (i+a, j+b)
            else {
              if (!mapaObstaculos(i+a, j+b))
                destino(i+a, j+b) += mapa(i,j)*mapa(i,j)*dispersion*i8;
              else
                destino(i,j)      += mapa(i,j)*dispersion*i8;
            }
          }
        }

      }
    }

  }

}


// -----------------------------------------------------------
Agent::ActionType Agent::Think()
{
  iteracion++;

  // Se actualiza el mapa de antiguedad

  #pragma omp parallel for
  for (int x=0; x<TAM_ESCENARIO; x++)
    for (int y=0; y<TAM_ESCENARIO; y++)
      if (!mapaObstaculos(x,y))
        mapaAntiguedad(x,y)+= 1.0;
      else
        mapaAntiguedad(x,y)= 0.0;


  // Actualizamos el modelo mental seg´un la acci´on que haya hecho el agente.
  
  int newx=posx;  // newx y newy son la nueva posici´on en el mapa a la que se quiere ir.
  int newy=posy;
  
  switch(ultimaAccion) {

    case actFORWARD:

      // Primero necesitamos calcular sobre qu´e casilla estamos obteniendo informaci´on
      if (dir==0)
        newy-= 1;
      else if(dir==1)
        newx+= 1;
      else if(dir==2)
        newy+= 1;
      else
        newx-= 1;

      // Actualizamos la celda a la que hemos intentado avanzar, como vac´ia si no hemos chocado o como llena si s´i.
      if (bump_) {
        mapaObstaculos(newx,newy)= true;
        mapaAntiguedad(newx,newy)= 0.0;
      }
      else {
        mapaObstaculos(newx,newy)= false;
        posx= newx;
        posy= newy;
      }
      // Si era desconocida, la volvemos conocida y si podemos estimamos los bordes del mapa
      if (mapaDesconocidos(newx,newy)==true) {
        minXConocido= (newx<minXConocido) ? newx : minXConocido;
        maxXConocido= (newx>maxXConocido) ? newx : maxXConocido;
        minYConocido= (newy<minYConocido) ? newy : minYConocido;
        maxYConocido= (newy>maxYConocido) ? newy : maxYConocido;

        if ((maxXConocido-minXConocido)==9) {
          for (int i=0; i<=minXConocido; i++) {
            for (int j=0; j<TAM_ESCENARIO; j++) {
              mapaDesconocidos(i,j)= false;
              mapaObstaculos(i,j)= true;
              mapaAntiguedad(i,j)= 0.0;
            }
          }

          for (int i=maxXConocido; i<TAM_ESCENARIO; i++) {
            for (int j=0; j<TAM_ESCENARIO; j++) {
              mapaDesconocidos(i,j)= false;
              mapaObstaculos(i,j)= true;
              mapaAntiguedad(i,j)= 0.0;
            }
          }
        }

        if ((maxYConocido-minYConocido)==9) {
          for (int i=0; i<TAM_ESCENARIO; i++) {
            for (int j=0; j<=minYConocido; j++) {
              mapaDesconocidos(i,j)= false;
              mapaObstaculos(i,j)= true;
              mapaAntiguedad(i,j)= 0.0;
            }
            for (int j=maxYConocido; j<TAM_ESCENARIO; j++) {
              mapaDesconocidos(i,j)= false;
              mapaObstaculos(i,j)= true;
              mapaAntiguedad(i,j)= 0.0;
            }
          }
        }

      }
      mapaDesconocidos(newx,newy)= false;
      break;


    case actTURN_R:
      dir= (dir+1)%4;
      break;

    case actTURN_L:
      dir= (dir+3)%4;
      break;

    case actIDLE:
      break;

    case actSNIFF:
      velocidadEstimada+= trufa_size_ / mapaAntiguedad(newx,newy);
      mediciones+= 1;
      break;

    case actEXTRACT:
      mapaAntiguedad(newx,newy)= 0.0;
      break;
  }

  // Se actualiza el mapa de rastro del agente

  #pragma omp parallel for
  for (int x=0; x<TAM_ESCENARIO; x++)
    for (int y=0; y<TAM_ESCENARIO; y++)
      mapaRastro(x,y)*= 0.9;  //0.5
  mapaRastro(posx,posy)+= 1.0;

  
  // Pintamos informaci´on sobre el modelo del agente
  if (PINTAR_MAPA)
    ImprimirMapaAgente();


  // Calculamos el gradiente de las casillas
  Mapa<double> potencialDesconocido; 
  Mapa<double> potencialAntiguedad; 
  Mapa<double> potencial;
  #pragma omp parallel for
  for (int i=0; i<TAM_ESCENARIO; i++) {
    for (int j=0; j<TAM_ESCENARIO; j++) {
      potencialDesconocido(i,j)= (double) mapaDesconocidos(i,j);
      if (i==0 || i==(TAM_ESCENARIO-1) || j==0 || j==(TAM_ESCENARIO-1)) potencialDesconocido(i,j)*= 100.0;
      potencialAntiguedad(i,j)= mapaAntiguedad(i,j)*0.001;
    }
  }
  
  CalcularGradiente(potencialDesconocido, mapaObstaculos, 0.1, 10);  //0.1, 100
  CalcularGradiente(potencialAntiguedad, mapaObstaculos, 0.001, 10);  //0.1, 10

  if (PINTAR_GRADIENTES) {
    ImprimirMapa(potencialDesconocido);
    ImprimirMapa(potencialAntiguedad);
  }

  #pragma omp parallel for
  for (int i=0; i<TAM_ESCENARIO; i++) {
    for (int j=0; j<TAM_ESCENARIO; j++) {
      potencial(i,j)= potencialDesconocido(i,j)*10000.0 + potencialAntiguedad(i,j) - mapaRastro(i,j);
    }
  }


  // Decidimos que acci´on realizar: Nos movemos adonde alla menos cantidad de "conocido"
  
  ActionType accion;

  // Decidimos en qu´e direcci´on (absoluta) se encuentra el punto con mayor desconocimiento.
  int direccionAbsolutaMaxima;   // La direccion absoluta es 0: arriba, 1: derecha, 2:abajo, 3:izquierda
  double valorMaximo;

  direccionAbsolutaMaxima= 0;
  valorMaximo= (mapaObstaculos(posx,posy-1)) ? -10000000.0 : potencial(posx,posy-1);

  if (!mapaObstaculos(posx+1,posy) && potencial(posx+1,posy) > valorMaximo) {
    valorMaximo= potencial(posx+1,posy);
    direccionAbsolutaMaxima= 1;
  }

  if (!mapaObstaculos(posx,posy+1) && potencial(posx,posy+1) > valorMaximo) {
    valorMaximo= potencial(posx,posy+1);
    direccionAbsolutaMaxima= 2;
  }

  if (!mapaObstaculos(posx-1,posy) && potencial(posx-1,posy) > valorMaximo) {
    valorMaximo= potencial(posx-1,posy);
    direccionAbsolutaMaxima= 3;
  }

  // Calculamos la direcci´on relativa a la orientaci´on actual del robot
  int direccionRelativaMaxima= (direccionAbsolutaMaxima - dir + 4) % 4;

  // Elegimos la accion
  double umbralDinamico1;
  double umbralDinamico2;
  #if (ESTIMAR_VELOCIDAD)
    umbralDinamico1= 190 *(0.03 / (velocidadEstimada/mediciones));
    umbralDinamico2= 30  *(0.03 / (velocidadEstimada/mediciones));
    //            140/20  160/20  170/30  180/30  190/30  200/30
    //agent:      1241.2  1278.9  1302.5  1318.1  1337.2  1352.8
    //mapa3:      1420.3  1460.6  1446.1  1475.2  1481.6  1489.5
    //mapa2_rap:  2885.0  2919.6  2956.0  2958.7  2931.1  2934.2
    //mapa1_rap:  3011.2  3039.4  3053.6  3048.0  3099.9  3077.6

  #else
    umbralDinamico1= 160;
    umbralDinamico2= 1000000;
  #endif

  // Si la casilla supera el umbral, o si estamos casi al final de la simulaci´on, extraemos.
  if ((mapaAntiguedad(posx,posy)>umbralDinamico1) || (iteracion>1800 && mapaAntiguedad(posx,posy)>(umbralDinamico1*0.25))) {
    accion= actEXTRACT;
  } 
  // Si no, pero si es suficientemente antigua, esnifamos para estimar la velocidad del mapa.
  else if (mapaAntiguedad(posx,posy)>umbralDinamico2 && mediciones<20 && mapaRastro(posx,posy)<1.5) {
    accion= actSNIFF;
  }
  // Si no, nos movemos hacia donde el gradiente de antiguedad sea mayor.
  else {
    switch(direccionRelativaMaxima){
        case 0: accion=actFORWARD;
                break;
        case 1: accion=actTURN_R;
                break;
        case 2: accion=actTURN_R;
                break;
        case 3: accion=actTURN_L;
                break;
    }
  }



  // Guardamos qu´e acci´on hemos hecho.
  ultimaAccion= accion;   

	return accion;
}
// -----------------------------------------------------------
void Agent::Perceive(const Environment &env)
{
  // Se actualizan los sensores

	trufa_size_ = env.TrufaAmount();
	bump_ = env.isJustBump();

}



// -----------------------------------------------------------



string ActionStr(Agent::ActionType accion)
{
	switch (accion)
	{
	case Agent::actFORWARD: return "FORWARD";
	case Agent::actTURN_L: return "TURN LEFT";
	case Agent::actTURN_R: return "TURN RIGHT";
	case Agent::actSNIFF: return "SNIFF";
	case Agent::actEXTRACT: return "EXTRACT";
	case Agent::actIDLE: return "IDLE";
	default: return "???";
	}
}
