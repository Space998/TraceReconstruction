#ifndef WR_H
#define WR_H

#include "DataType.h"
#include "Rivelatore.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>
#include <math.h>

//Subdivide a path sting into path - name - extention
inline
std::vector<std::string> SplitFilename (const std::string& str)     
{
    size_t found;
    size_t div;
    found=str.find_last_of("/\\");
    div=str.substr(found+1).find_first_of(".");
    return std::vector<std::string>{str.substr(0,found), str.substr(found+1).substr(0,div), str.substr(found+1).substr(div+1)};   //std::vector<std::string>{path, name, extension}
    /*
    std::cout << " folder: " << str.substr(0,found) << std::endl;
    std::cout << " filename: " << str.substr(found+1).substr(0,div) << std::endl;
    std::cout << " .exe: " << str.substr(found+1).substr(div+1) << std::endl; 
    */
}

//Count how many Simulation*.bin are in the directory
inline
int howMany(std::string path = "Simulation", std::string name = "Simulation")     
{
    int count=0;
    for (auto& p : std::filesystem::directory_iterator(path)) 
    {
        size_t found;
        //std::cout << p.path().string() << std::endl;
        found = p.path().string().find_last_of("/\\");
        //std::cout << typeid(p.path()).name() << std::endl;
        if (p.path().string().substr(found+1).find(name) != std::string::npos)
            count++;
    }
    return count;
}


//Library of functions to write and read over files.

//Function to determine the name of the files
//Determines the name for the file containing the original data
//Checks for the existance of the name file passed by the user or determens the name for the new automatically generated file
//Returns take number
int checkWriteFile(std::string &filename, std::string &file2);

//Function to write files
template <typename T> 
inline
void write(std::ofstream &file, T data)
{
    file.write(reinterpret_cast<char*>(&data), sizeof(data));
}

//Function that checks the existance of the file passed by the user to be read for data and in case of automatic naming return the correct file name
std::string existanceReadFile(std::string namefile);   

//Function to read file
void readFile(std::string namefile, Rivelatore &detector, const float rhoPrecision, const float thetaPrecision);
//theta and rho precision indicates the dimension of a pixel in the (rho,theta) space for the discretization of the Hough space
//theta precision is requested in degree -> the function will transform it in radiants
//rho precision is in meters

//Function for Hough trasformation
inline 
float yValue(Rivelatore &detector, const int y) //Returns the float value of the hit on y axis (set in the middle of the pixel)
{
    return (y*detector.m_dimension) + (detector.m_dimension/2);
}

inline
float rho(const float y, const float x, const float theta)        //Cos function returns the cosine of an angle of x radians.
{
    return cos(theta)*x + sin(theta*y);
}

inline
float degRad(const float deg)
{
    return (deg*M_PI)/180;
}

inline 
float radDeg(const float rad)
{
    return (rad*180)/M_PI;
}

#endif
