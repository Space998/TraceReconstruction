#include "WriteRead.h"
#include "DataType.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>

void readFile(std::string namefile)
{
    if(!std::filesystem::is_directory("Simulation/")) //Check if directory exist
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    if(std::filesystem::is_empty("Simulation/"))     //Check if directory is empty
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    if(namefile == "auto")     //If passed "auto" reads the last Simulation-"x".bin created
    {
        int count = 0;
        for (auto& p : std::filesystem::directory_iterator("Simulation/")) 
        {
            size_t found;
            size_t div;
            size_t point;
            found = p.path().string().find_last_of("/\\");
            //std::cout << typeid(p.path()).name() << std::endl;
            if (p.path().string().substr(found+1).find("Simulation") != std::string::npos)
                {
                div = p.path().string().substr(found+1).find_first_of("_");
                point = p.path().string().substr(found+1).substr(div+1).find_first_of(".");
                //std::cout << p.path().string().substr(found+1).substr(div+1).substr(0,point) << std::endl;
                if(std::stoi(p.path().string().substr(found+1).substr(div+1).substr(0,point)) > count)
                    count = std::stoi(p.path().string().substr(found+1).substr(div+1).substr(0,point));
                }
        }
        namefile = std::string("Simulation/Simulation_") + std::to_string(count) + std::string(".bin");
    }
    else if (!std::filesystem::exists(namefile))
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    std::ifstream in(namefile, std::ios::binary);
    while( !in.eof())       //Temporary
    {
        int w;
        in.read( (char*) &w, 4);
        if (structLenght.contains(w))
            std::cout << "0x" << std::hex << w << " : " << std::dec << structLenght[w]/4 << std::endl;
    }  
}