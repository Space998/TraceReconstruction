#ifndef DETECTOR_H
#define DETECTOR_H
struct Rivelatore
{
	int m_plate; 				//Numeber of plate of the detector -> start counting from one
	float m_distance;			//Distance between the plates of the detector in meters
	float m_width;				//Dimension of the detector in the x axix supposing no thickness for the actual detecting plates
	int m_number;				//Number of pixels in a plate of the detector
	float m_dimension;			//Dimension of the pixels of the detector in meters
	float m_lenght;				//Lenght of the plate of the detector in meters

	Rivelatore(int num = 20000, float dim = 50e-6) : m_plate{3}, m_distance{1}, m_width{(m_plate -1) * m_distance}, m_number{num}, m_dimension{dim}, m_lenght{num*dim} {}
};

//All distances are expressed in meters.

#endif
