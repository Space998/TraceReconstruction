#ifndef DETECTOR_H 
#define DETECTOR_H
struct Rivelatore
{
	unsigned int name;			//acronym characterizing the tipe of detector used
	int m_plate; 				//Number of plate of the detector -> start counting from one
	float m_distance;			//Distance between the plates of the detector in meters
	float m_width;				//Dimension of the detector in the x axix supposing no thickness for the actual detecting plates in meters
	int m_number;				//Number of pixels in a plate of the detector
	float m_dimension;			//Dimension of the pixels of the detector in meters
	float m_lenght;				//Lenght of the plate of the detector in meters
	float m_errorMean;			//Mean for the poisson distribution to generate noise over the plates
	float m_angle;				//blind angle of the detector (degree)

	Rivelatore(float ang = 2, int num = 20000, float dim = 50e-6, int nplate = 3, float dist = 1, unsigned int n = 0x1234ABCD) : name{n}, m_plate{nplate}, 
		m_distance{dist}, m_width{(nplate -1) * dist}, m_number{num}, m_dimension{dim}, m_lenght{num*dim}, m_errorMean{0.5}, m_angle{ang} {}
};
static_assert(sizeof(Rivelatore) == 4*9);

//All distances are expressed in meters.

#endif