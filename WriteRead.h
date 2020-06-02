#ifndef WR_H
#define WR_H

#include "DataType.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>

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
template <typename T> 
inline
void write(std::ofstream &file, T data)
{
    file.write(reinterpret_cast<char*>(&data), sizeof(data));
}

void readFile(std::string namefile);

#endif
