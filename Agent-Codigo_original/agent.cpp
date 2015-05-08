#include "agent.h"
#include "environment.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <utility>

using namespace std;
// -----------------------------------------------------------
Agent::ActionType Agent::Think()
{

	ActionType accion;

    switch(rand()%5){
        case 0: accion=actFORWARD;
                break;
        case 1: accion=actTURN_L;
                break;
        case 2: accion=actTURN_R;
                break;
        case 3: accion=actSNIFF;
                break;
        case 4: accion=actEXTRACT;
                break;
    }

	return accion;
}
// -----------------------------------------------------------
void Agent::Perceive(const Environment &env)
{
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
