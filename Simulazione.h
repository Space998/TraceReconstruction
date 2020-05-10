#ifndef SIMULATION_H
#define SIMULATION_H

#include "Rivelatore.h"

class Simulazione
{
	public:
		Simulazione(int num);

        int Num()                       const { return m_num; }
        Rivelatore const & Detector()   const { return m_rivelatore; }

        //Methods
		int SimulatePoint(const float y, const float x, const bool limit, const bool noise); 
		int SimulateLine();
		
	private:
		int m_num;                      //Number of events to be generated, in the programm counted from zero to m_num-1
		Rivelatore m_rivelatore;        //Detector
};

#endif