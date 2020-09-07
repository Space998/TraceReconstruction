#ifndef SIMULATION_H
#define SIMULATION_H

#include "DataType.h"
#include "Rivelatore.h"
#include <iomanip>
#include <chrono>
#include <cmath>

//Random generators
//Definition of the poisson random generator
//The number of noise hit of a plate are calculated from a poisson distribution with mean value of rivelatore.m_errorMean
int poisson(const float mean);

//Uniform random int generator
int randomInt(const int &min, const int &max);

//Uniform random float generator
float randomFloat(const float &min, const float &max);

//Function for the evaluation of line parameters
inline
float mLine(const float y1, const float x1, const float y, const float x)
{
    return (y - y1)/(x - x1);
}

inline
float qLine(const float y, const float x, const float m)
{
    return y - (m * x);
}

//Functions to determine time and time durations
inline
auto time()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t t_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
    return std::put_time(std::localtime(&t_c), "%F %T");
}

inline
int64_t duration(const std::chrono::high_resolution_clock::time_point time1)
{
   std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
   return std::chrono::duration_cast<std::chrono::nanoseconds>( time2 - time1 ).count();
}

//Function to dermine pixel that got hit
inline
int pixel(const Rivelatore &rivelatore, float value)	//From the value (intersection between line and datector plate) determines the pixel that was hit
{
    return round(value/(rivelatore.m_dimension));
}


//Function that return the value of m1 and m2 between with the m of the random line needs to be generated in case of a measure with limits -> the process can be visulized in the the geogebra file
void mBorders(const Rivelatore &rivelatore, const float y, const float x, float &m1, float &m2);

//Simulation functions
//Function to simulate the generation of num traces, 
//all generated from a single point 
void SimulatePoint(std::string filename,
                    const Rivelatore &rivelatore, 
                    const int num, 
                    const float y, 
                    const float x, 
                    const bool limit, 
                    const bool noise);

#endif