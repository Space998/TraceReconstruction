#ifndef WR_H
#define WR_H

#include "DataType.h"
#include <fstream>
#include <iostream>

//Library of functions to write and read over files.
template <typename T> 
inline
void write(std::ofstream &file, T data)
{
    file.write(reinterpret_cast<char*>(&data), sizeof(data));
}


inline
void readFile(std::ifstream &file)
{
    while( !file.eof())
    {
        int w;
        file.read( (char*) &w, 4);
        std::cout << w << std::endl;
    }  
}
#endif
