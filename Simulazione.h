#ifndef SIMULATION_H
#define SIMULATION_H

#include "DataType.h"
#include "Rivelatore.h"
#include <ctime>
#include <chrono>

//Utility functions
inline
int pixel(float value)	//From the value (intersection between line and datector plate) determines the pixel that was hit
{
    return value*1e6;
};

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
    auto data = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(data);  
    return std::ctime(&end_time);
}

//Function that return the value of m1 and m2 between with the m of the random line needs to be generated in case of a measure with limits
int mBorders(float y, float x, float (&array1)[2], float (&array2)[2]);

//Simulation functions
int SimulatePoint(Rivelatore rivelatore, int num, const float y, const float x, const bool limit, const bool noise);		//Function to simulate the generation of num traces, all generated from a single point 
int SimulateLine(Rivelatore rivelatore, int num);

#endif