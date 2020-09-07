#include "Rivelatore.h"
#include "Simulazione.h"
#include "Analysis.h"

int main(){

    Rivelatore detector;
 
    SimulatePoint("auto", detector, 10, 0.7,1, true, true);
    Analisys("auto", 0.005, 0.1, true, true, true, true, false);

    //SimulatePoint("auto", detector, 10, 0.7,1, true, true);
    //readFile("auto", 0.005, 0.1, true, true, false, true);

	return 0;
}
