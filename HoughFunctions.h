#ifndef HF_H
#define HF_H

#include "Rivelatore.h"
#include <vector>
#include <cmath>
#include <map>

//Function for Hough trasformation
inline
int abs(const int x)    //calculates absolute value of a float
{
    if(x > 0)
        return x;
    else
        return -x;    
}

inline
float absFloat(const float x)    //calculates absolute value of a float
{
    if(x > 0)
        return x;
    else
        return -x;    
}

inline
float degRad(const float deg) //Transforms degree in radiant
{
    return (deg*M_PI)/180;
}

inline 
float radDeg(const float rad)   //Transforms radiand in degree
{
    return (rad*180)/M_PI;
}

inline 
float yValueCor(const Rivelatore &detector, const int y) //Returns the float value of the hit on y axis (set in the middle of the pixel)
{
    return (y*detector.m_dimension) + (detector.m_dimension/2);
}
 
inline 
float xValueCor(const Rivelatore &detector, const int x) //Returns the float value of the hit on x axis (position of the plate)
{
    return -(x*detector.m_distance);
}

inline
float rho(const float y, const float x, const float theta)        //Cos function returns the cosine of an angle of x radians.
{
    return cos(degRad(theta))*x + sin(degRad(theta))*y;
}

inline
int rhoDiscrete(const float rho, const float rhoPrecision)
{
    return rho/rhoPrecision;
}

inline
float mReconstructed(const float theta)
{
    return -cos(degRad(theta))/sin(degRad(theta));
}

inline 
float qReconstructed(const float theta, const float rho)
{
    return (rho)/sin(degRad(theta));
}

/*
inline 
float mean(std::vector<int64_t> &vec)
{
    int64_t sum = 0;
    for(int i = 0; i < int(vec.size()); i++)
        sum += vec.at(i);
    return float(sum)/float(vec.size());
}
*/

#endif