#ifndef HF_H
#define HF_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <iterator>

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
float yValueCor(Rivelatore &detector, const int y) //Returns the float value of the hit on y axis (set in the middle of the pixel)
{
    return (y*detector.m_dimension) + (detector.m_dimension/2);
}
 
/*
inline 
std::vector<float> yValueCorAll(Rivelatore &detector, std::vector<int> &yValue) //Returns the float value of the hit on y axis (set in the middle of the pixel)
{
    std::vector<float> output;
    for (int i = 0; i < int(yValue.size()); i++)
        output.push_back(yValueCor(detector, yValue.at(i)));
    return output;
}
*/

inline 
float xValueCor(Rivelatore &detector, const int x) //Returns the float value of the hit on x axis (position of the plate)
{
    return -(x*detector.m_distance);
}

/*
inline 
std::vector<float> xValueCorAll(Rivelatore &detector, std::vector<int> &xValue) //Returns the float value of the hit on y axis (set in the middle of the pixel)
{
    std::vector<float> output;
    for (int i = 0; i < int(xValue.size()); i++)
        output.push_back(xValueCor(detector, xValue.at(i)));
    return output;
}
*/

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
void rhoAll(std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, std::vector<std::map<int,int>> &rhoValue, const float thetaPrecision, const float rhoPrecision)
{
    for (int i = 0; i < int(rhoValue.size()); i++)
    {
        for (int j = 0; j < int(xValueFloat.size()); j++)
        {
            rhoValue.at(i)[rhoDiscrete(rho(yValueFloat.at(j),xValueFloat.at(j),(i+1)*thetaPrecision), rhoPrecision)] ++;
            //std::cout << i << " - " << rhoDiscrete(rho(yValueFloat.at(j),xValueFloat.at(j),(i+1)*thetaPrecision), rhoPrecision) << ", " << rhoValue.at(i)[rhoDiscrete(rho(yValueFloat.at(j),xValueFloat.at(j),(i+1)*thetaPrecision), rhoPrecision)] << std::endl;
        }
    }
}

/*
inline
void rhoAll(std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, std::vector<std::vector<int>> &rhovalueALL, const float thetaPrecision, const float rhoPrecision)
{
    for (int i = 0; i < int(rhovalueALL.size()); i++)
    {
        for (int j = 0; j < int(xValueFloat.size()); j++)
        {
            rhovalueALL.at(i).push_back(rhoDiscrete(rho(yValueFloat.at(j),xValueFloat.at(j),(i+1)*thetaPrecision), rhoPrecision));
        }
    }
}

inline
void rhoOccurrency(std::vector<std::vector<int>> &rhovalueALL, std::vector<std::vector<int>> &rhovalue, std::vector<std::vector<int>> &rhovalueOccurrency)
{
    for (int i = 0; i < int(rhovalueALL.size()); i++)
    {
        std::sort(rhovalueALL.at(i).begin(), rhovalueALL.at(i).end());
    }
}
*/

/*
inline
void maxMap(std::map<int,int> &x)
{
    std::map<int,int>::iterator best = std::max_element(x.begin(),x.end(),[] (const std::pair<char,int>& a, const std::pair<char,int>& b)->bool{ return a.second < b.second; } );
    std::cout << best->first << " , " << best->second << "\n";
}
*/

inline
float mReconstructed(const int theta, const float thetaPrecision)
{
    return -cos(degRad((float(theta+1))*thetaPrecision))/sin(degRad((float(theta+1))*thetaPrecision));
}

inline 
float qReconstructed(const int theta, const int rho, const float thetaPrecision, const float rhoPrecision)
{
    return (rho*rhoPrecision)/sin(degRad((float(theta+1))*thetaPrecision));
}

inline 
float mean(std::vector<int64_t> &vec)
{
    int64_t sum = 0;
    for(int i = 0; i < int(vec.size()); i++)
        sum += vec.at(i);
    return float(sum)/float(vec.size());
}

#endif