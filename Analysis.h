#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <string>
#include <vector>

//Function to evaluate algorithm performance, needs to be called inside Analisys
void checkCorrectness(std::string original, 
                        std::string analysis, 
                        std::vector<int64_t> times, 
                        const bool interactiveImg); 

//Function to analyse file
void Analisys(std::string namefile, 
                const float rhoPrecision, 
                const float thetaPrecision, 
                const bool terminalOutput, 
                const bool images, 
                const bool interactiveImg, 
                const bool check, 
                const bool constrain);
//theta and rho precision indicates the dimension of a pixel in the (rho,theta) space for the discretization of the Hough space
//theta precision is requested in degree -> the function will transform it in radiants
//rho precision is in meters

#endif