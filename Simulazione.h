#ifndef SIMULATION_H
#define SIMULATION_H

#include "DataType.h"
#include "Rivelatore.h"
#include <ctime>
#include <chrono>

//Utility functions
inline
int pixel(Rivelatore &rivelatore, float value)	//From the value (intersection between line and datector plate) determines the pixel that was hit
{
    return value/(rivelatore.m_dimension);
}

//Function for the evaluation of m and q end similar
inline
float mLine(const float y1, const float x1, const float y, const float x)
{
    return (y - y1)/(x - x1);
}

inline
float qLine(const float y1, const float x1, const float y, const float x)
{
    return y1 - x1 * (y - y1);
}

inline
float RandomFloat(float min, float max) 
{
    return  (max - min) * ((((float) rand()) / (float) RAND_MAX)) + min ;
}

inline
auto time()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t t_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
    return std::put_time(std::localtime(&t_c), "%F %T");
    /*
    auto data = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(data);  
    return std::ctime(&end_time);
    */
}

inline
unsigned int duration(std::chrono::high_resolution_clock::time_point time1)
{
   std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
   return std::chrono::duration_cast<std::chrono::nanoseconds>( time2 - time1 ).count();
}

//Function that return the value of m1 and m2 between with the m of the random line needs to be generated in case of a measure with limits -> the process can be visulized in the the geogebra file
int mBorders(float y, float x, float (&array1)[2], float (&array2)[2]);

//Simulation functions
int SimulatePoint(std::string filename, Rivelatore rivelatore, int num, const float y, const float x, const bool limit = true, const bool noise = false);		//Function to simulate the generation of num traces, all generated from a single point 
//int SimulateLine(std::string filename, Rivelatore rivelatore, int num);

#endif