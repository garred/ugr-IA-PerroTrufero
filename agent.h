#ifndef AGENT__
#define AGENT__

#include <string>
#include <omp.h>
#include <iostream>
using namespace std;

template <class T, int N=19>
struct Mapa {
	
	T** valor;

	Mapa() {
		valor= new T*[N];
		for (int i=0; i<N; i++) {
			valor[i]= new T[N];
			for (int j=0; j<N; j++)
				valor[i][j]= 0;
		}
	}

	~Mapa() {
		for (int i=0; i<10; i++)
			delete [] valor[i];
		delete [] valor;
	}

	T& operator()(int x, int y) {
		return valor[x][y];
	}
	
};


// -----------------------------------------------------------
//				class Agent
// -----------------------------------------------------------
class Environment;
class Agent
{
public:

	enum ActionType
	{
	    actFORWARD,
	    actTURN_L,
	    actTURN_R,
	    actSNIFF,
	    actEXTRACT,
	    actIDLE
	};

	Agent() {
		// Sensores. No tocar.	//
	  trufa_size_=-1;					//
	  bump_=false;						//

		this->ultimaAccion= actIDLE;

	  dir= 0;
	  posx= 9;
	  posy= 9;
	  minXConocido=posx;
	  maxXConocido=posx;
	  minYConocido=posy;
	  maxYConocido=posy;

	  mediciones= 5;
	  velocidadEstimada= 0.03*mediciones;

	  iteracion= 0;

	  for (int i=0; i<19; i++) {
	  	for (int j=0; j<19; j++) {
	  		this->mapaObstaculos(i,j)= false;
	  		this->mapaDesconocidos(i,j)= true;
	  		this->mapaAntiguedad(i,j)= 0.0;
	  		this->mapaRastro(i,j)= 0.0;
	  	}
	  }
	  this->mapaDesconocidos(posx,posy)= false;
	  this->mapaObstaculos(posx,posy)= false;
	}

	void Perceive(const Environment &env);
	ActionType Think();
	void ImprimirMapaAgente();

private:
	int trufa_size_;
	bool bump_;

	ActionType ultimaAccion;

	int dir;	//0:adelante, 1:derecha, 2:detras, 3:izquierda
	int posx;			//Posici´on inicial del agente en su propio modelo del mapa
	int posy;

	int minXConocido;		//Coordenadas x e y minimas conocidas.
	int maxXConocido;
	int minYConocido;
	int maxYConocido;

	
	double velocidadEstimada;	//Velocidad estimada de crecimiento del mapa.
	double mediciones;				//N´umero de mediciones (olfateos) realizadas.

	int iteracion;

	Mapa<bool> mapaObstaculos;
	Mapa<bool> mapaDesconocidos;
	Mapa<double> mapaAntiguedad;
	Mapa<double> mapaRastro;

};

string ActionStr(Agent::ActionType);

#endif
