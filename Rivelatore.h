#ifndef DETECTOR_H
#define DETECTOR_H
struct Rivelatore
{
	int m_plate = 3; 			//Numeber of plate of the detector
	float m_lenght = 1;			//Lenght of the plate of the detector
	float m_distance = 1;		//Distance between the plates of the detector
	float m_error = 24e-6;		//Erro associated to every position measurement
};

//All distances are expressed in meters.

#endif
