#ifndef WR_H
#define WR_H

#include "DataType.h"
#include "Rivelatore.h"
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

//Subdivide a path sting into path - name - extention
inline
std::vector<std::string> SplitFilename (const std::string& str)     
{
    size_t found;
    size_t div;
    found=str.find_last_of("/\\");
    div=str.substr(found+1).find_first_of(".");
    return std::vector<std::string>{str.substr(0,found), str.substr(found+1).substr(0,div), str.substr(found+1).substr(div+1)};   
    //std::vector<std::string>{path, name, extension}
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
unsigned int checkWriteFile(std::string &filename, std::string &file2);

//Function that checks the existance of the file passed by the user to be read for data and in case of automatic naming return the correct file name
std::string existanceFile(std::string namefile, std::string type);   //Function to determine the the existance of file namefile 
//std::string existanceReadFile(std::string namefile);   

//Function to write files
template <typename T> 
inline
void write(std::ofstream &file, T data)
{
    file.write(reinterpret_cast<char*>(&data), sizeof(data));
}

template <typename H, typename T> 
inline
void writeData(std::ofstream &datafile, H header ,const std::vector<T> &values)
{
    write(datafile, header);
    for(auto const& el: values) 
    {
        write(datafile, el);
    }
}

#endif
